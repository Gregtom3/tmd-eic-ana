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
    const Table* getTable() const;
private:
    TFile* file;
    TTree* tree;
    std::string filename;
    std::string treename;
    std::unique_ptr<Table> table;
};

#endif // TMD_H