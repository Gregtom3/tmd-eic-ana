#include "Logger.h"
#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>

int main() {

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

    // Run a simple inject/extract on bin 0
    const int n_injections = 5;
    const int bin_index = 0;
    tmd.queueInjection({ .bin_index = bin_index, .n = n_injections, .A_opt = 0.3 });
    tmd.runQueuedInjections();
    LOG_INFO("inject_extract completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
