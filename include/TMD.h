#ifndef TMD_H
#define TMD_H

#include "TFile.h"
#include "TTree.h"
#include "Grid.h"
#include "TCut.h"
#include <map>

// Function to load a ROOT file and TTree
void loadRootFile(const char* filename, const char* treename);

// Generate TCuts for each bin in a Grid (for X and Q)
std::map<std::string, TCut> generateBinTCuts(const Grid& grid);

#endif // TMD_H