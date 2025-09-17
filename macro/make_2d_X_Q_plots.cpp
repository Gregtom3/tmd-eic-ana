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
    LOG_INFO("[make_2d_X_Q_plots] Successfully loaded ROOT file and TTree.");
    tmd.setMaxEntries(args.maxEntries);
    if (args.maxEntries > 0)
        LOG_INFO("[make_2d_X_Q_plots] Set max entries to: " + std::to_string(args.maxEntries));
    tmd.setTargetPolarization(args.targetPolarization);
    LOG_INFO("[make_2d_X_Q_plots] Set target polarization to " + std::to_string(args.targetPolarization));
    tmd.loadTable(args.energyConfig);
    LOG_INFO("[make_2d_X_Q_plots] Successfully loaded table for energy config: " + args.energyConfig);
    if(args.grid.empty()) {
        LOG_FATAL("Grid variables not specified. Use --grid <var1,var2,...>");
        return 1;
    }
    tmd.buildGrid( args.grid );
    const Grid* grid = tmd.getGrid();
    grid->printGridSummary(5); // Print summary of first 5 bins
    LOG_INFO("[make_2d_X_Q_plots] Successfully built grid based on table data.");

    tmd.fillHistograms("PhPerp", args.outDir, args.overwrite);
    tmd.plot2DMap("PhPerp", args.outDir + "/PhPerp_2D_X_Q_" + args.energyConfig + ".png");

    tmd.fillHistograms("Z", args.outDir, args.overwrite);
    tmd.plot2DMap("Z", args.outDir + "/Z_2D_X_Q_" + args.energyConfig + ".png");

    tmd.fillHistograms("PhiH", args.outDir, args.overwrite);
    tmd.plot2DMap("PhiH", args.outDir + "/PhiH_2D_X_Q_" + args.energyConfig + ".png");

    return 0;
}
