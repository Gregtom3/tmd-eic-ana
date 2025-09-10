#include "TMD.h"
#include "Logger.h"
#include <iostream>
#include "TCut.h"
#include "Grid.h"
#include <map>
#include "Hist.h"
#include <filesystem>

TMD::TMD(const std::string& filename, const std::string& treename)
    : file(nullptr), tree(nullptr), filename(filename), treename(treename), table(nullptr), grid(nullptr) {
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
    hist = std::make_unique<Hist>(tree);
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
    this->energyConfig = energyConfig; // store for cache naming
    table = std::make_unique<Table>(energyConfig);
}

const Table* TMD::getTable() const {
    return table.get();
}

void TMD::buildGrid(const std::vector<std::string>& binNames) {
    if (!table) {
        throw std::runtime_error("Table not loaded in TMD::buildGrid");
    }
    grid = std::make_unique<Grid>(table->buildGrid(binNames));
    binTCuts = generateBinTCuts(*grid);
    LOG_INFO("Successfully generated " + std::to_string(binTCuts.size()) + " bin TCuts.");
}

const std::map<std::string, TCut>& TMD::getBinTCuts() const {
    return binTCuts;
}

const Grid* TMD::getGrid() const {
    return grid.get();
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

void TMD::fillHistograms(const std::string& var, const std::string& outDir, bool overwrite) {
    if (!hist) return;

    // Ensure out directory exists
    std::filesystem::path dir(outDir);
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    // Compose cache filename: hists_<rootstem>__<treename>__<energyConfig>__<var>__nbins<N>.root
    std::string rootStem = std::filesystem::path(filename).stem().string();
    size_t nBins = binTCuts.size();
    std::string cacheName = "hists_" + rootStem + "__" + treename + "__" + energyConfig + "__" + var + "__nbin" + std::to_string(nBins) + ".root";
    std::filesystem::path cachePath = dir / cacheName;

    bool histLoaded = false;
    bool meanLoaded = false;
    if (!overwrite && std::filesystem::exists(cachePath)) {
        histLoaded = hist->loadHistCache(cachePath.string(), var);
        meanLoaded = hist->loadMeanCache(cachePath.string(), var);
        if (histLoaded && meanLoaded) {
            LOG_INFO("Using cached histograms and means: " + cachePath.string());
            return;
        }
    }

    // Build histograms and save
    hist->fillHistograms(var, binTCuts);
    hist->saveHistCache(cachePath.string(), var);
    hist->saveMeanCache(cachePath.string(), var);
}

void TMD::plotBin(const std::string& var, size_t binIndex) {
    if (!hist) return;   
    hist->plotBin(var, binIndex);  
}

