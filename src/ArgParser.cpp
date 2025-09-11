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
        } else {
            LOG_ERROR("Unknown argument: " + arg);
            exit(1);
        }
    }
    return args;
}
