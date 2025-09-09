#include "TMD.h"
#include <iostream>

void loadRootFile(const char* filename, const char* treename) {
    // Open the ROOT file
    TFile* file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    // Load the TTree
    TTree* tree = (TTree*)file->Get(treename);
    if (!tree) {
        std::cerr << "Error: Could not find tree " << treename << " in file " << filename << std::endl;
        file->Close();
        return;
    }

    std::cout << "Successfully loaded TTree: " << treename << " from file: " << filename << std::endl;

    // Clean up
    file->Close();
}