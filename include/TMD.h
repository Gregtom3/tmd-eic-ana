#ifndef TMD_H
#define TMD_H

#include "Grid.h"
#include "Hist.h"
#include "Inject.h"
#include "InjectionProject.h"
#include "Plotter.h"
#include "TCut.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"
#include "Table.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

class TMD {
public:
    TMD(const std::string& filename, const std::string& treename);
    ~TMD();
    bool isLoaded() const;
    void setMaxEntries(Long64_t maxEntries);
    TTree* getTree() const;
    std::map<std::string, TCut> generateBinTCuts(const Grid& grid) const;
    void loadTable();
    void loadTable(const std::string& energyConfig);
    void buildGrid(const std::vector<std::string>& binNames);
    const Table* getTable() const;
    const Grid* getGrid() const;
    const std::map<std::string, TCut>& getBinTCuts() const;
    void fillHistograms(const std::string& var, const std::string& outDir = "out", bool overwrite = false);
    void plot1DBin(const std::string& var, size_t binIndex, const std::string& outpath = "");
    void plot2DMap(const std::string& var, const std::string& outpath);
    void queueInjection(const InjectionProject::Job& job);
    void runQueuedInjections();

private:
    TFile* file;
    TTree* tree;
    std::string filename;
    std::string treename;
    std::string energyConfig; // stored for cache naming
    std::unique_ptr<Table> table;
    std::unique_ptr<Grid> grid;
    std::vector<std::string> binNames; // mainBinNames (ex: <"X", "Q">)
    std::map<std::string, TCut> binTCuts;
    std::unique_ptr<Hist> hist;
    std::unique_ptr<Plotter> plotter;
    InjectionProject* proj = nullptr;

    // MC and scaling information
    double xsTotal{0.0};     // total cross-section read from file
    long long totalEvents{0}; // total MC events read from file
    double mc_lumi{0.0};     // computed mc luminosity (nb^-1)
    double exp_lumi{0.0};    // computed expected luminosity (nb^-1)
    double scale{1.0};       // scale = exp_lumi / mc_lumi
};

#endif // TMD_H