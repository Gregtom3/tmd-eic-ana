#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'
require 'time'
require 'csv'
require 'set'

# Default options (only provide a default for tree)
options = {
  tree: "tree",
  x_only: false # New from Filippo
}

parser = OptionParser.new do |opts|
  opts.banner = "Usage: submit_injection_jobs.rb [options]"
  opts.on("--energy STRING", "Energy config (required) e.g. 10x100") { |v| options[:energy] = v }
  opts.on("--tableDir STRING", "Path to tables (required) e.g. tables/xQZPhPerp_v0") { |v| options[:tableDir] = v }
  opts.on("--n_injections INTEGER", Integer, "Number of injections (required)") { |v| options[:n_injections] = v }
  opts.on("--bins INTEGER", Integer, "Number of bins to run (required). If N <= 0, run ALL bins") { |v| options[:bins] = v }
  opts.on("--bins_per_job INTEGER", Integer, "Number of bins per job (required)") { |v| options[:bins_per_job] = v }
  opts.on("--grid STRING", "Grid string (required) e.g. \"X,Q\". Values must be one of: X, Q, Z, PhPerp") { |v| options[:grid] = v }
  opts.on("--maxEntries INTEGER", Integer, "Maximum entries to process from ROOT file (default: all)") { |v| options[:maxEntries] = v }
  opts.on("--extract_with_true STRING", "Whether to extract with true kinematics (default: false)") { |v| options[:extract_with_true] = v }
  opts.on("--tree STRING", "Tree name (default: #{options[:tree]})") { |v| options[:tree] = v }
  opts.on("-h", "--help", "Show this message") { puts opts; exit }
end

begin
  parser.parse!
rescue OptionParser::InvalidOption, OptionParser::MissingArgument => e
  puts "Error: #{e.message}\n\n"
  puts parser
  exit 1
end

# Validate required options
required = [:energy, :n_injections, :bins, :bins_per_job, :grid, :tableDir]
missing = required.select { |k| options[k].nil? }
if missing.any?
  puts "Error: Missing required options: #{missing.map(&:to_s).join(', ')}\n\n"
  puts parser
  exit 1
end

# Validate and normalize grid
allowed_grids = %w[X Q Z PhPerp]
grid_list = options[:grid].split(',').map(&:strip)
if grid_list.empty? || grid_list.any?(&:empty?)
  puts "Error: --grid must be a comma-separated list with at least one value (e.g. 'X,Q')"
  exit 1
end
invalid = grid_list.reject { |g| allowed_grids.include?(g) }
if invalid.any?
  puts "Error: Invalid grid values: #{invalid.join(', ')}. Allowed values are: #{allowed_grids.join(', ')}"
  exit 1
end

# If bins <= 0, we will run all bins (handled later after reading table). Use nil to indicate "all".
if options[:bins] <= 0
  bins_label = 'all'
  options[:bins] = nil
else
  bins_label = options[:bins].to_s
end

# Validate bins_per_job
if options[:bins_per_job] <= 0
  puts "Error: --bins_per_job must be > 0"
  exit 1
end

# Validate extract_with_true if set
if options[:extract_with_true]
  unless ['true', 'false'].include?(options[:extract_with_true].downcase)
    puts "Error: --extract_with_true must be 'true' or 'false' if provided"
    exit 1
  end
  options[:extract_with_true] = options[:extract_with_true].downcase
end


# Map energy configurations to table files
base_dir = options[:tableDir]
table_files = {
  "5x41"   => "#{base_dir}/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt",
  "10x100" => "#{base_dir}/AUT_average_PV20_EPIC_piplus_sqrts=63.246.txt",
  "18x275" => "#{base_dir}/AUT_average_PV20_EPIC_piplus_sqrts=140.712.txt"
}

# Get the table file based on the energy option
table_file = table_files[options[:energy]]
if table_file.nil?
  puts "Error: Unknown energy configuration '#{options[:energy]}'"
  exit 1
end

# Read the table file and parse unique bins based on the requested grid
unique_bins = Set.new
begin
  CSV.foreach(table_file, col_sep: ",", headers: true) do |row|
    # Build a key composed of the min/max ranges for each requested grid dimension
    key = []
    grid_list.each do |g|
      min_col = "#{g}_min"
      max_col = "#{g}_max"
      if !row.headers.include?(min_col) || !row.headers.include?(max_col)
        puts "Error: Table file '#{table_file}' missing required columns: #{min_col} or #{max_col}"
        exit 1
      end
      key << row[min_col].to_f
      key << row[max_col].to_f
    end
    unique_bins.add(key)
  end
rescue Errno::ENOENT
  puts "Error: Table file '#{table_file}' not found."
  exit 1
end

# Print the total number of unique bins for the requested grid
puts "Total [#{grid_list.join(',')}] bins: #{unique_bins.size}"

# Infer ROOT file based on energy config
options[:root_file] = "../../out/Piplus.3.27.2025___epic.25.03.1_#{options[:energy]}/analysis.root"

# Ensure the ROOT file exists
unless File.exist?(options[:root_file])
  puts "Error: ROOT file '#{options[:root_file]}' does not exist."
  puts "Please check that the file was produced for energy '#{options[:energy]}' and the path is correct."
  exit 1
end

# Timestamped subdirectory under slurm/
timestamp = Time.now.strftime("%Y%m%d_%H%M%S")
# make a safe grid label for directory names
grid_label = options[:grid].gsub(/[^0-9A-Za-z_-]/, '_')
slurm_subdir = File.join("slurm", "#{options[:energy]}_inj#{options[:n_injections]}_bins#{bins_label}_grid#{grid_label}_#{timestamp}")
FileUtils.mkdir_p(slurm_subdir)

# Keep track of created scripts
slurm_files = []

# Calculate the total number of bins if not explicitly set
if options[:bins].nil?
  options[:bins] = unique_bins.size
end

# Loop over bin indices in chunks of bins_per_job and create jobs
(0...options[:bins]).each_slice(options[:bins_per_job]) do |bin_indices|
  job_name   = "inject_bins#{bin_indices.first}_to_#{bin_indices.last}_#{options[:energy]}"
  yaml_out   = "job_#{job_name}"
  slurm_file = File.join(slurm_subdir, "slurm_#{job_name}.sh")

  File.open(slurm_file, "w") do |f|
    f.puts "#!/bin/bash"
    f.puts "#SBATCH --job-name=#{job_name}"
    f.puts "#SBATCH --output=#{slurm_subdir}/slurm_%j.out"
    f.puts "#SBATCH --error=#{slurm_subdir}/slurm_%j.err"
    f.puts "#SBATCH --account=eic"
    f.puts "#SBATCH --partition=production"
    f.puts "#SBATCH --time=24:00:00"
    f.puts "srun ./bin/inject \\"
    f.puts "  --file #{options[:root_file]} \\"
    f.puts "  --tree #{options[:tree]} \\"
    f.puts "  --energy #{options[:energy]} \\"
    f.puts "  --table #{table_file} \\"
    f.puts "  --grid \"#{options[:grid]}\" \\"
    f.puts "  --n_injections #{options[:n_injections]} \\"
    f.puts "  --bin_index_start #{bin_indices.first} \\"
    f.puts "  --bin_index_end #{bin_indices.last} \\"
    if options[:extract_with_true]
      f.puts "  --extract_with_true '#{options[:extract_with_true]}' \\"
    end
    # If maxEntries is given and > 0, include it
    if options[:maxEntries] && options[:maxEntries] > 0
      f.puts "  --maxEntries #{options[:maxEntries]} \\"
    end
    f.puts "  --outFilename #{yaml_out} \\"
    f.puts "  --outDir #{slurm_subdir}"
    f.puts ""
  end

  slurm_files << slurm_file
  puts "Created job script: #{slurm_file}"
end

puts "\nAll jobs written to: #{slurm_subdir}"

# Prompt to submit
print "\nSubmit all jobs now? (Y/N): "
answer = $stdin.gets.strip.downcase
if answer == "y"
  slurm_files.each do |file|
    system("sbatch #{file}")
  end
  puts "Submitted #{slurm_files.size} jobs."
else
  puts "Jobs not submitted."
end
