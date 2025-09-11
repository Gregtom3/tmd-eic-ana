#ifndef TMD_H
#define TMD_H

#include "Grid.h"
#include "Hist.h"
#include "Inject.h"
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

class TMD {
public:
    TMD(const std::string& filename, const std::string& treename);
    ~TMD();
    bool isLoaded() const;
    void setMaxEntries(Long64_t maxEntries);
    TTree* getTree() const;
    std::map<std::string, TCut> generateBinTCuts(const Grid& grid) const;
    void loadTable(const std::string& energyConfig);
    void buildGrid(const std::vector<std::string>& binNames);
    const Table* getTable() const;
    const Grid* getGrid() const;
    const std::map<std::string, TCut>& getBinTCuts() const;
    void fillHistograms(const std::string& var, const std::string& outDir = "out", bool overwrite = false);
    void plot1DBin(const std::string& var, size_t binIndex, const std::string& outpath = "");
    void plot2DMap(const std::string& var, const std::string& outpath);
    void inject_extract(int bin_index, double A = 0.1);

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
};

#endif // TMD_H