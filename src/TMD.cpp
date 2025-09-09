#include "TMD.h"
#include <iostream>
#include "TCut.h"
#include "Grid.h"
#include <map>

void loadRootFile(const char* filename, const char* treename) {
    // Open the ROOT file
    TFile* file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        LOG_ERROR(std::string("Could not open file ") + filename);
        return;
    }

    // Load the TTree
    TTree* tree = (TTree*)file->Get(treename);
    if (!tree) {
        LOG_ERROR(std::string("Could not find tree ") + treename + " in file " + filename);
        file->Close();
        return;
    }

    LOG_INFO(std::string("Successfully loaded TTree: ") + treename + " from file: " + filename);

    // Clean up
    file->Close();
}

std::map<std::string, TCut> generateBinTCuts(const Grid& grid) {
    std::map<std::string, TCut> binTCuts;
    const auto& bins = grid.getBins();
    for (const auto& binPair : bins) {
        const std::string& key = binPair.first;
        const Bin& bin = binPair.second;
        double X_min = bin.getMin("X");
        double X_max = bin.getMax("X");
        double Q_min = bin.getMin("Q");
        double Q_max = bin.getMax("Q");
        std::string cutStr =
            "X >= " + std::to_string(X_min) + " && X < " + std::to_string(X_max) +
            " && Q >= " + std::to_string(Q_min) + " && Q < " + std::to_string(Q_max);
        binTCuts[key] = TCut(cutStr.c_str());
    }
    return binTCuts;
}

