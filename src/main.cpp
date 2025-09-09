#include "TMD.h"
#include "Logger.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <ROOT file> <TTree name>" << std::endl;
        return 1;
    }

    // Set log level
    Logger::setLevel(Logger::Level::Info);
    
    const char* filename = argv[1];
    const char* treename = argv[2];

    // Load the ROOT file and TTree
    loadRootFile(filename, treename);

    // Example usage of Logger
    LOG_PRINT("[main.cpp] Successfully loaded ROOT file and TTree.");
    
    return 0;
}