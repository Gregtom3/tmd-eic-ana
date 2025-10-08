#include "Logger.h"
#include "ArgParser.h"
#include "TMD.h"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    std::string outDir = "out/";
    Args args = parseArgs(argc, argv);
    // Parse output directory and create if needed
    if(args.outDir != "out") {
        LOG_INFO("Using output directory from args: " + args.outDir);
        outDir = args.outDir;
    } else {
        LOG_INFO("Using default output directory: " + outDir);
    }
    // Ensure the artifacts directory exists
    std::filesystem::create_directories(outDir);

    const std::string outpath = "out/output.root";
    TMD tmd(outpath, "tree");
    if (!tmd.isLoaded()) {
        LOG_ERROR("TMD failed to load generated tree file");
        return 1;
    }

    LOG_INFO("TMD loaded generated file successfully");


    tmd.loadTable(args.table,args.energyConfig);
    tmd.buildGrid({"X", "Q"});
    LOG_INFO("Grid built successfully");

    tmd.fillHistograms("PhPerp", outDir, true);
    tmd.plot2DMap("PhPerp", outDir + "/PhPerp_2D_X_Q_10x100.png");

    tmd.fillHistograms("Z", outDir, true);
    tmd.plot2DMap("Z", outDir + "/Z_2D_X_Q_10x100.png");

    tmd.fillHistograms("PhiH", outDir, true);
    tmd.plot2DMap("PhiH", outDir + "/PhiH_2D_X_Q_10x100.png");

    LOG_INFO("2D plots generated successfully");
    return 0;
}