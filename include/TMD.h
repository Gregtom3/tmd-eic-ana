#ifndef TMD_H
#define TMD_H

#include "TFile.h"
#include "TTree.h"
#include "Grid.h"
#include "TCut.h"
#include "Table.h"
#include <map>
#include <string>
#include <memory>

class TMD {
public:
    TMD(const std::string& filename, const std::string& treename);
    ~TMD();
    bool isLoaded() const;
    TTree* getTree() const;
    std::map<std::string, TCut> generateBinTCuts(const Grid& grid) const;
    void loadTable(const std::string& energyConfig);
    void buildGrid(const std::vector<std::string>& binNames);
    const Table* getTable() const;
    const Grid* getGrid() const;
    const std::map<std::string, TCut>& getBinTCuts() const;
    void plotFirstBin1D(const std::string& var = "", int nbins = -1, double xmin = -1, double xmax = -1);
    void plotFirstBin1D(const std::string& var);
private:
    TFile* file;
    TTree* tree;
    std::string filename;
    std::string treename;
    std::unique_ptr<Table> table;
    std::unique_ptr<Grid> grid;
    std::map<std::string, TCut> binTCuts;
};

#endif // TMD_H