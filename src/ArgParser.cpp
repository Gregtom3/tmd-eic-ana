#include "ArgParser.h"
#include "Logger.h"
#include <iostream>
#include <string>

Args parseArgs(int argc, char** argv) {
    if (argc < 4) {
        LOG_ERROR(std::string("Usage: ") + argv[0] +
                  " <ROOT file> <TTree name> <energy config> [--overwrite|-f] [--out <dir>] [--maxEntries <N>]");
        exit(1);
    }

    Args args;
    args.filename = argv[1];
    args.treename = argv[2];
    args.energyConfig = argv[3];

    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--overwrite" || arg == "-f") {
            args.overwrite = true;
        } else if (arg == "--out" && i + 1 < argc) {
            args.outDir = argv[++i];
        } else if (arg == "--maxEntries" && i + 1 < argc) {
            args.maxEntries = std::stoll(argv[++i]);
        } else if (arg == "--outFilename" && i + 1 < argc) {
            args.outFilename = argv[++i];
        } else if (arg == "--targetPolarization" && i + 1 < argc) {
            args.targetPolarization = std::stod(argv[++i]);
        } else if (arg == "--n_injections" && i + 1 < argc) {
            args.n_injections = std::stoi(argv[++i]);
        } else if (arg == "--bin_index" && i + 1 < argc) {
            args.bin_index = std::stoi(argv[++i]);
        } else if (arg == "--extract_with_true" && i + 1 < argc) {
            std::string val = argv[++i];
            args.extract_with_true = (val == "1" || val == "true");
        } else if (arg == "--A_opt" && i + 1 < argc) {
            args.A_opt = std::stod(argv[++i]);
        } else {
            LOG_ERROR("Unknown argument: " + arg);
            exit(1);
        }
    }
    return args;
}
