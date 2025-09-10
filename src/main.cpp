#include "TMD.h"
#include "Logger.h"
#include "Table.h"
#include <iostream>
#include <string>
#include <vector>

struct Args {
    std::string filename;
    std::string treename;
    std::string energyConfig;
    bool overwrite = false;
    std::string outDir = "out";
    Long64_t maxEntries = -1;
};

Args parseArgs(int argc, char** argv) {
    if (argc < 4) {
        LOG_ERROR(std::string("Usage: ") + argv[0] + " <ROOT file> <TTree name> <energy config> [--overwrite|-f] [--out <dir>] [--maxEntries <N>]");
        exit(1);
    }

    Args args;
    args.filename = argv[1];
    args.treename = argv[2];
    args.energyConfig = argv[3];

    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--overwrite" || arg == "-f") {
            args.overwrite = true;
        } else if (arg == "--out" && i + 1 < argc) {
            args.outDir = argv[++i];
        } else if (arg == "--maxEntries" && i + 1 < argc) {
            args.maxEntries = std::stoll(argv[++i]);
        } else {
            LOG_ERROR("Unknown argument: " + arg);
            exit(1);
        }
    }
    return args;
}

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
    if(args.maxEntries>0)
        LOG_INFO("[main.cpp] Set max entries to: " + std::to_string(args.maxEntries));

    tmd.loadTable(args.energyConfig);
    LOG_INFO("[main.cpp] Successfully loaded table for energy config: " + args.energyConfig);

    tmd.buildGrid({"X","Q"});
    const Grid* grid = tmd.getGrid();
    grid->printGridSummary(5); // Print summary of first 5 bins
    LOG_INFO("[main.cpp] Successfully built grid based on table data.");

    tmd.fillHistograms("PhPerp", args.outDir, args.overwrite);
    //tmd.plotBin("PhPerp", 0);
    tmd.plot2DMap("PhPerp", "playground/test.png");

    tmd.inject_extract(0);
    return 0;
}