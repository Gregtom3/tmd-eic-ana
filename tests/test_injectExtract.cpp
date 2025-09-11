#include "Logger.h"
#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>

int main() {

    const std::string outpath = "out/output_generated.root";

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

    // Run a simple inject/extract on bin 0
    tmd.inject_extract(0, 0.2);
    LOG_INFO("inject_extract completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
