#!/usr/bin/env ruby

require 'fileutils'
require 'optparse'
require 'time'

# Default options
options = {
  energy: "10x100",
  n_injections: 1,
  bins: 10,
  tree: "tree"
}

OptionParser.new do |opts|
  opts.banner = "Usage: submit_injection_jobs.rb [options]"

  opts.on("--energy STRING", "Energy config (default: #{options[:energy]})") { |v| options[:energy] = v }
  opts.on("--n_injections N", Integer, "Number of injections (default: #{options[:n_injections]})") { |v| options[:n_injections] = v }
  opts.on("--bins N", Integer, "Number of bins to run (default: #{options[:bins]})") { |v| options[:bins] = v }
end.parse!

# Infer ROOT file based on energy config
options[:root_file] = "../../out/Piplus.3.27.2025___epic.25.03.1_#{options[:energy]}/analysis.root"

# Timestamped subdirectory under slurm/
timestamp = Time.now.strftime("%Y%m%d_%H%M%S")
slurm_subdir = File.join("slurm", "#{options[:energy]}_inj#{options[:n_injections]}_bins#{options[:bins]}_#{timestamp}")
FileUtils.mkdir_p(slurm_subdir)

# Keep track of created scripts
slurm_files = []

# Loop over bin indices and create jobs
(0...options[:bins]).each do |bin_idx|
  job_name   = "inject_bin#{bin_idx}_#{options[:energy]}"
  yaml_out   = "job_#{job_name}.yaml"
  slurm_file = File.join(slurm_subdir, "slurm_#{job_name}.sh")

  File.open(slurm_file, "w") do |f|
    f.puts "#!/bin/bash"
    f.puts "#SBATCH --job-name=#{job_name}"
    f.puts "#SBATCH --output=#{slurm_subdir}/slurm_%j.out"
    f.puts "#SBATCH --error=#{slurm_subdir}/slurm_%j.err"
    f.puts "#SBATCH --account=eic"
    f.puts "#SBATCH --partition=production"
    f.puts "#SBATCH --time=02:00:00"
    f.puts ""
    f.puts "srun ./bin/inject \\"
    f.puts "  #{options[:root_file]} \\"
    f.puts "  #{options[:tree]} \\"
    f.puts "  #{options[:energy]} \\"
    f.puts "  --n_injections #{options[:n_injections]} \\"
    f.puts "  --bin_index #{bin_idx} \\"
    f.puts "  --out #{slurm_subdir} \\"
    f.puts "  --outFilename #{yaml_out}"
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
