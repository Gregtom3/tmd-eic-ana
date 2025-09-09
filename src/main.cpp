#include "TMD.h"
#include "Logger.h"
#include "Table.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 4) {
        LOG_ERROR(std::string("Usage: ") + argv[0] + " <ROOT file> <TTree name> <energy config>");
        return 1;
    }
    Logger::setLevel(Logger::Level::Info);
    std::string filename = argv[1];
    std::string treename = argv[2];
    std::string energyConfig = argv[3];

    TMD tmd(filename, treename);
    if (!tmd.isLoaded()) {
        LOG_FATAL("Failed to load ROOT file or TTree.");
        return 1;
    }
    LOG_PRINT("[main.cpp] Successfully loaded ROOT file and TTree.");

    tmd.loadTable(energyConfig);
    LOG_PRINT("[main.cpp] Successfully loaded table for energy config: " + energyConfig);
    
    tmd.buildGrid({"X", "Q"});
    const Grid* grid = tmd.getGrid();
    grid->printGridSummary(5); // Print summary of first 5 bins
    LOG_PRINT("[main.cpp] Successfully built grid based on table data.");



    return 0;
}