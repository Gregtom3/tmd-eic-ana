// Macro to create simple kinematic plots
#include "Logger.h"
#include "TMD.h"
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

    tmd.setMaxEntries(args.maxEntries);
    if (args.maxEntries > 0)
        LOG_INFO("[make_kinematic_plots] Set max entries to: " + std::to_string(args.maxEntries));

    tmd.loadTable(args.energyConfig);
    tmd.buildGrid({"X","Q","Z","PhPerp"});

    tmd.fillHistograms("X", args.outDir, args.overwrite);
    tmd.fillHistograms("Q", args.outDir, args.overwrite);
    tmd.fillHistograms("Z", args.outDir, args.overwrite);
    tmd.fillHistograms("PhPerp", args.outDir, args.overwrite);

    return 0;
}
