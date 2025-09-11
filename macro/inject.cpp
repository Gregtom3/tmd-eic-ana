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
    LOG_INFO("[main.cpp] Successfully loaded ROOT file and TTree.");
    tmd.setMaxEntries(args.maxEntries);
    if (args.maxEntries > 0)
        LOG_INFO("[main.cpp] Set max entries to: " + std::to_string(args.maxEntries));
    tmd.setTargetPolarization(args.targetPolarization);
    LOG_INFO("[main.cpp] Set target polarization to " + std::to_string(args.targetPolarization));
    tmd.setOutDir(args.outDir);
    tmd.setOutFilename(args.outFilename);
    tmd.loadTable(args.energyConfig);

    tmd.buildGrid({"X"});

    tmd.queueInjection({
        .bin_index = args.bin_index,
        .n = args.n_injections,
        .extract_with_true = args.extract_with_true,
        .A_opt = args.A_opt
    });
    tmd.runQueuedInjections();
    return 0;
}
