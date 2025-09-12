#include "Logger.h"
#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>
#include <filesystem> // For directory creation

int main() {
    const std::string artifactDir = "artifacts/plots";
    // Ensure the artifacts directory exists
    std::filesystem::create_directories(artifactDir);

    const std::string outpath = "out/output.root";

    // Now attempt to use TMD on this file
    TMD tmd(outpath, "tree");
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
    tmd.fillHistograms("X", artifactDir, true);
    LOG_INFO("fillHistograms completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
