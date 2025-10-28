#include "InjectionProject.h"
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include "Inject.h"


InjectionProject::InjectionProject(const std::string& filename, TTree* tree, const Table* table, double scale, const Grid* grid, double targetPolarization, const std::string& outDir, const std::string& outFilename, channel_type channel)
    : filename(filename), tree(tree), table(table), scale(scale), grid(grid), targetPolarization(targetPolarization), outDir(outDir), outFilename(outFilename), channel(channel) {
        // Create outprefix
        std::string rootStem = std::filesystem::path(filename).stem().string();
        if (!outFilename.empty()) {
            outPrefix = std::filesystem::path(outDir) / outFilename;
        } else {
            outPrefix = std::filesystem::path(outDir) / (std::string("injection_") + rootStem + "_" + tree->GetName());
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
        std::unique_ptr<Inject> injector;
        if (channel == channel_type::Hadron) {
            injector = std::make_unique<InjectHadron>(tree, table, scale, targetPolarization);
        } else if (channel == channel_type::Dihadron) {
            injector = std::make_unique<InjectDihadron>(tree, table, scale, targetPolarization);
        }

        std::vector<double> extractedVals;
        std::vector<double> extractedErrs;
        double total_sumW = 0.0;
        double total_sumXW = 0.0;
        double total_sumQW = 0.0;
        double total_sumQ2W = 0.0;
        double total_sumZW = 0.0;
        double total_sumPhPerpW = 0.0;
        for (int i = 0; i < job.n; ++i) {
            auto res = injector->injectExtractForBin(bin, job.extract_with_true, job.A_opt);
            extractedVals.push_back(res.first);
            extractedErrs.push_back(res.second);
            const auto& stats = injector->getLastStats();
            total_sumW += stats.sumW;
            total_sumXW += stats.sumXW;
            total_sumQW += stats.sumQW;
            total_sumQ2W += stats.sumQ2W;
            total_sumZW += stats.sumZW;
            total_sumPhPerpW += stats.sumPhPerpW;
        }
        // compute simple summary: mean and stddev of extractedVals
        double mean = 0.0;
        for (double v : extractedVals) mean += v;
        mean /= std::max(1, static_cast<int>(extractedVals.size()));
        double var = 0.0;
        for (double v : extractedVals) var += (v - mean) * (v - mean);
        double stddev = extractedVals.size() > 1 ? std::sqrt(var / (extractedVals.size() - 1)) : 0.0;

        // compute averages
        double avgX = total_sumW > 0.0 ? total_sumXW / total_sumW : 0.0;
        double avgQ = total_sumW > 0.0 ? total_sumQW / total_sumW : 0.0;
        double avgQ2 = total_sumW > 0.0 ? total_sumQ2W / total_sumW : 0.0;
        double avgZ = total_sumW > 0.0 ? total_sumZW / total_sumW : 0.0;
        double avgPhPerp = total_sumW > 0.0 ? total_sumPhPerpW / total_sumW : 0.0;
        
        // Emit YAML for this job
        out << YAML::BeginMap;
        out << YAML::Key << "bin_index" << YAML::Value << job.bin_index;
        out << YAML::Key << "events" << YAML::Value << bin.getEvents();
        out << YAML::Key << "expected_events" << YAML::Value << bin.getExpectedEvents();
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
        out << YAML::Key << "avg_X" << YAML::Value << avgX;
        out << YAML::Key << "avg_Q" << YAML::Value << avgQ;
        out << YAML::Key << "avg_Q2" << YAML::Value << avgQ2;
        out << YAML::Key << "avg_Z" << YAML::Value << avgZ;
        out << YAML::Key << "avg_PhPerp" << YAML::Value << avgPhPerp;
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
