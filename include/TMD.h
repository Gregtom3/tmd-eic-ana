#ifndef TMD_H
#define TMD_H

#include "TFile.h"
#include "TTree.h"

// Function to load a ROOT file and TTree
void loadRootFile(const char* filename, const char* treename);

#endif // TMD_H