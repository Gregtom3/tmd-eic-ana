#include "InjectionProject.h"
#include "Inject.h"
#include "Logger.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>

InjectionProject::InjectionProject(TTree* tree, const Table* table, double scale, const Grid* grid)
    : tree(tree), table(table), scale(scale), grid(grid) {}

void InjectionProject::addJob(int bin_index, int n_injections, std::optional<double> A_opt) {
    jobs.push_back(Job{bin_index, n_injections, A_opt});
}

bool InjectionProject::run(const std::string& outPrefix) {
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
        std::string csvName = outPrefix + "_bin" + std::to_string(job.bin_index) + "_job" + std::to_string(jobIdx) + ".csv";
        std::ofstream csv(csvName);
        if (!csv.is_open()) {
            LOG_ERROR("InjectionProject: could not open " + csvName);
            continue;
        }
        csv << "injected,extracted,err\n";
        std::vector<double> extractedVals;
        std::vector<double> extractedErrs;
        for (int i = 0; i < job.n; ++i) {
            auto res = injector.injectExtractForBin(bin, job.A_opt);
            csv << (job.A_opt.has_value() ? std::to_string(job.A_opt.value()) : "table") << "," << res.first << "," << res.second << "\n";
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
        out << YAML::Key << "mean_extracted" << YAML::Value << mean;
        out << YAML::Key << "stddev_extracted" << YAML::Value << stddev;
        out << YAML::Key << "csv" << YAML::Value << csvName;
        out << YAML::EndMap;

        csv.close();
        ++jobIdx;
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;

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
