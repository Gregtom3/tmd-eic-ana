#include "InjectionProject.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>

InjectionProject::InjectionProject(const std::string& filename, TTree* tree, const Table* table, double scale, const Grid* grid, double targetPolarization, const std::string& outDir, const std::string& outFilename)
    : filename(filename), tree(tree), table(table), scale(scale), grid(grid), targetPolarization(targetPolarization), outDir(outDir), outFilename(outFilename) {
        // Create outprefix
        std::string rootStem = std::filesystem::path(filename).stem().string();
        if (!outFilename.empty()) {
            outPrefix = std::filesystem::path(outDir) / outFilename;
        } else {
            outPrefix = std::filesystem::path(outDir) / (std::string("injection_") + rootStem + "_" + tree->GetName() + ".yaml");
        }
    }

void InjectionProject::addJob(const InjectionProject::Job& job = {}) {
    jobs.push_back(job);
}

bool InjectionProject::run() {
    if (!grid) {
        LOG_ERROR("InjectionProject: no grid provided");
        return false;
    }
    const auto& bins = grid->getBins();
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "jobs" << YAML::Value << YAML::BeginSeq;
    int jobIdx = 0;
    for (const auto& job : jobs) {
        // locate bin
        if (static_cast<size_t>(job.bin_index) >= bins.size()) {
            LOG_ERROR("InjectionProject: bin index out of range: " + std::to_string(job.bin_index));
            continue;
        }
        auto it = bins.begin();
        std::advance(it, job.bin_index);
        const Bin& bin = it->second;
        Inject injector(tree, table, scale, targetPolarization);
        std::vector<double> extractedVals;
        std::vector<double> extractedErrs;
        for (int i = 0; i < job.n; ++i) {
            auto res = injector.injectExtractForBin(bin, job.extract_with_true, job.A_opt);
            extractedVals.push_back(res.first);
            extractedErrs.push_back(res.second);
        }
        // compute simple summary: mean and stddev of extractedVals
        double mean = 0.0;
        for (double v : extractedVals) mean += v;
        mean /= std::max(1, static_cast<int>(extractedVals.size()));
        double var = 0.0;
        for (double v : extractedVals) var += (v - mean) * (v - mean);
        double stddev = extractedVals.size() > 1 ? std::sqrt(var / (extractedVals.size() - 1)) : 0.0;
        
        // Emit YAML for this job
        out << YAML::BeginMap;
        out << YAML::Key << "bin_index" << YAML::Value << job.bin_index;
        out << YAML::Key << "events" << YAML::Value << bin.getEvents();
        out << YAML::Key << "X_min" << YAML::Value << bin.getMin("X");
        out << YAML::Key << "X_max" << YAML::Value << bin.getMax("X");
        out << YAML::Key << "Q_min" << YAML::Value << bin.getMin("Q");
        out << YAML::Key << "Q_max" << YAML::Value << bin.getMax("Q");
        out << YAML::Key << "Z_min" << YAML::Value << bin.getMin("Z");
        out << YAML::Key << "Z_max" << YAML::Value << bin.getMax("Z");
        out << YAML::Key << "PhPerp_min" << YAML::Value << bin.getMin("PhPerp");
        out << YAML::Key << "PhPerp_max" << YAML::Value << bin.getMax("PhPerp");
        out << YAML::Key << "used_reconstructed_kinematics" << YAML::Value << (!job.extract_with_true);
        out << YAML::Key << "n_injections" << YAML::Value << job.n;
        out << YAML::Key << "injected" << YAML::Value << (job.A_opt.has_value() ? job.A_opt.value() : 0.0);
        out << YAML::Key << "all_extracted" << YAML::Value << YAML::Flow << extractedVals;
        out << YAML::Key << "all_errors" << YAML::Value << YAML::Flow << extractedErrs;
        out << YAML::Key << "mean_extracted" << YAML::Value << mean;
        out << YAML::Key << "stddev_extracted" << YAML::Value << stddev;
        out << YAML::EndMap;

        ++jobIdx;
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;

    // Write to file
    std::string yamlName = outPrefix + ".yaml";
    std::ofstream yamlOut(yamlName);
    if (!yamlOut.is_open()) {
        LOG_ERROR("InjectionProject: could not write YAML " + yamlName);
        return false;
    }
    yamlOut << out.c_str();
    yamlOut.close();
    LOG_INFO("InjectionProject: wrote summary to " + yamlName);
    return true;
}
