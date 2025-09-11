#include "InjectionProject.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>

InjectionProject::InjectionProject(const std::string& filename, TTree* tree, const Table* table, double scale, const Grid* grid)
    : filename(filename), tree(tree), table(table), scale(scale), grid(grid) {
        // Create outprefix
        std::string rootStem = std::filesystem::path(filename).stem().string();
        outPrefix = std::string("injection_") + rootStem + "_" + tree->GetName();
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
        Inject injector(tree, table, scale);
        std::vector<double> extractedVals;
        std::vector<double> extractedErrs;
        for (int i = 0; i < job.n; ++i) {
            auto res = injector.injectExtractForBin(bin, job.A_opt);
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

        out << YAML::BeginMap;
        out << YAML::Key << "bin_index" << YAML::Value << job.bin_index;
        out << YAML::Key << "n_injections" << YAML::Value << job.n;
        out << YAML::Key << "injected" << YAML::Value << (job.A_opt.has_value() ? job.A_opt.value() : 0.0);
        // All values
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
