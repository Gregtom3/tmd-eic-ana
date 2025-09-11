#include "Logger.h"
#include "TMD.h"
#include "Table.h"
#include "ArgParser.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    Logger::setLevel(Logger::Level::Info);
    Args args = parseArgs(argc, argv);

    TMD tmd(args.filename, args.treename);
    if (!tmd.isLoaded()) {
        LOG_FATAL("Failed to load ROOT file or TTree.");
        return 1;
    }

    LOG_INFO("[main.cpp] Successfully loaded ROOT file and TTree.");
    tmd.setMaxEntries(args.maxEntries);
    if (args.maxEntries > 0)
        LOG_INFO("[main.cpp] Set max entries to: " + std::to_string(args.maxEntries));

    tmd.loadTable(args.energyConfig);
    LOG_INFO("[main.cpp] Successfully loaded table for energy config: " + args.energyConfig);

    tmd.buildGrid({"X", "Q"});
    const Grid* grid = tmd.getGrid();
    grid->printGridSummary(5); // Print summary of first 5 bins
    LOG_INFO("[main.cpp] Successfully built grid based on table data.");

    tmd.fillHistograms("PhPerp", args.outDir, args.overwrite);
    // tmd.plotBin("PhPerp", 0);
    tmd.plot2DMap("PhPerp", "playground/test.png");

    tmd.inject_extract(0);
    return 0;
}
