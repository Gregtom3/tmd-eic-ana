#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'
require 'time'
require 'csv'
require 'set'

# Default options
options = {
  energy: "10x100",
  n_injections: 1,
  bins: nil, # Default to nil to allow optional setting
  bins_per_job: 4, # New parameter with default value
  tree: "tree"
}

OptionParser.new do |opts|
  opts.banner = "Usage: submit_injection_jobs.rb [options]"

  opts.on("--energy STRING", "Energy config (default: #{options[:energy]})") { |v| options[:energy] = v }
  opts.on("--n_injections N", Integer, "Number of injections (default: #{options[:n_injections]})") { |v| options[:n_injections] = v }
  opts.on("--bins N", Integer, "Number of bins to run (default: all unique pairs)") { |v| options[:bins] = v }
  opts.on("--bins_per_job N", Integer, "Number of bins per job (default: #{options[:bins_per_job]})") { |v| options[:bins_per_job] = v }
end.parse!

# Map energy configurations to table files
table_files = {
  "5x41" => "tables/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt",
  "10x100" => "tables/AUT_average_PV20_EPIC_piplus_sqrts=63.246.txt",
  "18x275" => "tables/AUT_average_PV20_EPIC_piplus_sqrts=140.712.txt"
}

# Get the table file based on the energy option
table_file = table_files[options[:energy]]
if table_file.nil?
  puts "Error: Unknown energy configuration '#{options[:energy]}'"
  exit 1
end

# Read the table file and parse unique (X_min, X_max, Q_min, Q_max) pairs
unique_pairs = Set.new
begin
  CSV.foreach(table_file, col_sep: ",", headers: true) do |row|
    x_min = row["X_min"].to_f
    x_max = row["X_max"].to_f
    q_min = row["Q_min"].to_f
    q_max = row["Q_max"].to_f
    unique_pairs.add([x_min, x_max, q_min, q_max])
  end
rescue Errno::ENOENT
  puts "Error: Table file '#{table_file}' not found."
  exit 1
end

# Print the total number of unique pairs
puts "Total [X,Q] bins: #{unique_pairs.size}"

# Infer ROOT file based on energy config
options[:root_file] = "../../out/Piplus.3.27.2025___epic.25.03.1_#{options[:energy]}/analysis.root"

# Timestamped subdirectory under slurm/
timestamp = Time.now.strftime("%Y%m%d_%H%M%S")
slurm_subdir = File.join("slurm", "#{options[:energy]}_inj#{options[:n_injections]}_bins#{options[:bins]}_#{timestamp}")
FileUtils.mkdir_p(slurm_subdir)

# Keep track of created scripts
slurm_files = []

# Calculate the total number of bins if not explicitly set
if options[:bins].nil?
  options[:bins] = unique_pairs.size
end

# Loop over bin indices in chunks of bins_per_job and create jobs
(0...options[:bins]).each_slice(options[:bins_per_job]) do |bin_indices|
  job_name   = "inject_bins#{bin_indices.first}_to_#{bin_indices.last}_#{options[:energy]}"
  yaml_out   = "job_#{job_name}.yaml"
  slurm_file = File.join(slurm_subdir, "slurm_#{job_name}.sh")

  File.open(slurm_file, "w") do |f|
    f.puts "#!/bin/bash"
    f.puts "#SBATCH --job-name=#{job_name}"
    f.puts "#SBATCH --output=#{slurm_subdir}/slurm_%j.out"
    f.puts "#SBATCH --error=#{slurm_subdir}/slurm_%j.err"
    f.puts "#SBATCH --account=eic"
    f.puts "#SBATCH --partition=production"
    f.puts "#SBATCH --time=24:00:00"
    f.puts "srun ./bin/inject \\
      --file #{options[:root_file]} \\"
      --tree #{options[:tree]} \\
      --energy #{options[:energy]} \\
      --n_injections #{options[:n_injections]} \\
      --bin_index_start #{bin_indices.first} \\
      --bin_index_end #{bin_indices.last} \\
      --outFilename #{yaml_out} \\
      --outDir #{slurm_subdir}"
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
