#ifndef TMD_H
#define TMD_H

#include "TFile.h"
#include "TTree.h"
#include "Grid.h"
#include "TCut.h"
#include <map>
#include <string>

class TMD {
public:
    TMD(const std::string& filename, const std::string& treename);
    ~TMD();
    bool isLoaded() const;
    TTree* getTree() const;
    std::map<std::string, TCut> generateBinTCuts(const Grid& grid) const;
private:
    TFile* file;
    TTree* tree;
    std::string filename;
    std::string treename;
};

#endif // TMD_H