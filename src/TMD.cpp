#include "TMD.h"
#include "Logger.h"
#include <iostream>
#include "TCut.h"
#include "Grid.h"
#include <map>

TMD::TMD(const std::string& filename, const std::string& treename)
    : file(nullptr), tree(nullptr), filename(filename), treename(treename), table(nullptr) {
    file = TFile::Open(filename.c_str());
    if (!file || file->IsZombie()) {
        LOG_ERROR(std::string("Could not open file ") + filename);
        file = nullptr;
        return;
    }
    tree = dynamic_cast<TTree*>(file->Get(treename.c_str()));
    if (!tree) {
        LOG_ERROR(std::string("Could not find tree ") + treename + " in file: " + filename);
        file->Close();
        file = nullptr;
        tree = nullptr;
        return;
    }
    LOG_INFO(std::string("Successfully loaded TTree: ") + treename + " from file: " + filename);
}

TMD::~TMD() {
    if (file) file->Close();
    // unique_ptr handles table cleanup
}

bool TMD::isLoaded() const {
    return file && tree;
}

TTree* TMD::getTree() const {
    return tree;
}

void TMD::loadTable(const std::string& energyConfig) {
    table = std::make_unique<Table>(energyConfig);
}

const Table* TMD::getTable() const {
    return table.get();
}

std::map<std::string, TCut> TMD::generateBinTCuts(const Grid& grid) const {
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

