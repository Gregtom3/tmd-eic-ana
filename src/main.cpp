#include "TMD.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <ROOT file> <TTree name>" << std::endl;
        return 1;
    }

    //const char* filename = argv[1];
    //const char* treename = argv[2];

    // Load the ROOT file and TTree
    //loadRootFile(filename, treename);

    return 0;
}