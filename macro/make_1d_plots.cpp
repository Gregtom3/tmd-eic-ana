// Macro to create simple 1D kinematic plots
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
        LOG_INFO("[make_1d_plots] Set max entries to: " + std::to_string(args.maxEntries));

    if(args.table.empty()){
        LOG_FATAL("Table not specified. Use --table </path/to/table.csv>");
    }
    tmd.loadTable(args.table,args.energyConfig);

    tmd.buildGrid({"X","Q","Z","PhPerp"});

    tmd.fillHistograms("X", args.outDir, args.overwrite);
    tmd.fillHistograms("Q", args.outDir, args.overwrite);
    tmd.fillHistograms("Z", args.outDir, args.overwrite);
    tmd.fillHistograms("PhPerp", args.outDir, args.overwrite);

    tmd.plot1DBin("X", 0, args.outDir + "/kinematic_X.png");
    tmd.plot1DBin("Q", 0, args.outDir + "/kinematic_Q.png");
    tmd.plot1DBin("Z", 0, args.outDir + "/kinematic_Z.png");
    tmd.plot1DBin("PhPerp", 0, args.outDir + "/kinematic_PhPerp.png");
    return 0;
}
