#include "Logger.h"
#include "TMD.h"
#include "ArgParser.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>
#include <filesystem> // For directory creation

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

    // Now attempt to use TMD on this file
    TMD tmd("out/output.root", "tree"); 
    if (!tmd.isLoaded()) {
        LOG_ERROR("TMD failed to load generated tree file");
        return 1;
    }

    LOG_INFO("TMD loaded generated file successfully");

    // Load table (default) and build a simple grid
    tmd.loadTable();
    tmd.buildGrid({"X"});
    std::cout << "Grid summary:" << std::endl;
    tmd.getGrid()->printGridSummary();

    // Fill histograms for variable X into the artifacts directory
    tmd.fillHistograms("X", outDir, true);
    tmd.fillHistograms("Q", outDir, true);
    tmd.fillHistograms("Z", outDir, true);
    tmd.fillHistograms("PhPerp", outDir, true);
    tmd.fillHistograms("PhiH", outDir, true);
    LOG_INFO("fillHistograms completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
