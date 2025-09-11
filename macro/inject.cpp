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
    tmd.setTargetPolarization(0.7);
    LOG_INFO("[main.cpp] Set target polarization to 0.7");
    tmd.loadTable();
    
    tmd.buildGrid({"X"});
    
    const int n_injections = 1000;
    const int bin_index = 0;
    const bool extract_with_true = true;
    tmd.queueInjection({ .bin_index = bin_index, .n = n_injections, .extract_with_true = extract_with_true, .A_opt = 0.05,  });
    tmd.runQueuedInjections();
    return 0;
}
