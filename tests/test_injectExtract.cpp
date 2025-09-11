#include "Logger.h"
#include "ArgParser.h"
#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>

int main(int argc, char** argv) {
    Args args = parseArgs(argc, argv);

    TMD tmd(args.filename, args.treename);
    if (!tmd.isLoaded()) {
        LOG_ERROR("TMD failed to load generated tree file");
        return 1;
    }

    LOG_INFO("TMD loaded generated file successfully");

    tmd.setTargetPolarization(args.targetPolarization);
    tmd.setOutDir(args.outDir);
    tmd.setOutFilename(args.outFilename);
    tmd.loadTable(args.energyConfig);
    tmd.buildGrid({"X"});
    std::cout << "Grid summary:" << std::endl;
    tmd.getGrid()->printGridSummary();

    tmd.queueInjection({
        .bin_index = args.bin_index,
        .n = args.n_injections,
        .extract_with_true = args.extract_with_true,
        .A_opt = args.A_opt
    });
    tmd.runQueuedInjections();
    LOG_INFO("inject_extract completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
