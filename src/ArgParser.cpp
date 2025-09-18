#include "ArgParser.h"
#include "Logger.h"
#include <iostream>
#include <string>

Args parseArgs(int argc, char** argv) {
    // If user asked for help, print usage and exit regardless of other args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            LOG_INFO("Usage (flags): --file <ROOT file> --tree <TTree name> --energy <energy config> [options]");
            LOG_INFO("Or (positional): <ROOT file> <TTree name> <energy config> [options]");
            LOG_INFO("Options:");
            LOG_INFO("  --file <ROOT file>         Input ROOT file");
            LOG_INFO("  --tree <TTree name>        Name of TTree inside the file");
            LOG_INFO("  --energy <energy config>   Energy configuration identifier");
            LOG_INFO("  --overwrite, -f            Overwrite outputs");
            LOG_INFO("  --outDir <dir>             Output directory (default out)");
            LOG_INFO("  --maxEntries <N>           Max entries to process");
            LOG_INFO("  --outFilename <filename>   Output filename");
            LOG_INFO("  --targetPolarization <v>   Target polarization value");
            LOG_INFO("  --n_injections <N>         Number of injections (default 10)");
            LOG_INFO("  --grid <X,Q,...>           Comma-separated list of grid variables (default X,Q)");
            LOG_INFO("  --bin_index <N>            Bin index to process");
            LOG_INFO("  --bin_index_start <N>  Start bin index (inclusive)");
            LOG_INFO("  --bin_index_end <N>    End bin index (inclusive)");
            LOG_INFO("  --extract_with_true <t/f>  Extract with true");
            LOG_INFO("  --A_opt <value>            Optional A value");
            exit(0);
        }
    }

    Args args;
    // Parse flags and/or positional arguments. Support both --file/--tree/--energy and legacy positional form.
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--file" && i + 1 < argc) {
            args.filename = argv[++i];
        } else if (arg == "--tree" && i + 1 < argc) {
            args.treename = argv[++i];
        } else if (arg == "--energy" && i + 1 < argc) {
            args.energyConfig = argv[++i];
        } else if (arg == "--overwrite" || arg == "-f") {
            args.overwrite = true;
        } else if (arg == "--outDir" && i + 1 < argc) {
            args.outDir = argv[++i];
        } else if (arg == "--maxEntries" && i + 1 < argc) {
            args.maxEntries = std::stoll(argv[++i]);
        } else if (arg == "--outFilename" && i + 1 < argc) {
            args.outFilename = argv[++i];
        } else if (arg == "--targetPolarization" && i + 1 < argc) {
            args.targetPolarization = std::stod(argv[++i]);
        } else if (arg == "--n_injections" && i + 1 < argc) {
            args.n_injections = std::stoi(argv[++i]);
        } else if (arg == "--grid" && i + 1 < argc) {
            // Convert comma separated list to vector
            std::string gridStr = argv[++i];
            size_t start = 0;
            size_t end = gridStr.find(',');
            while (end != std::string::npos) {
                std::string gridVar = gridStr.substr(start, end - start);
                if (gridVar != "X" && gridVar != "Q" && gridVar != "Z" && gridVar != "PhPerp") {
                    LOG_ERROR("Invalid grid variable: " + gridVar);
                    exit(1);
                }
                for (const auto& existingVar : args.grid) {
                    if (existingVar == gridVar) {
                        LOG_ERROR("Duplicate grid variable: " + gridVar);
                        exit(1);
                    }
                }
                args.grid.push_back(gridVar);

                start = end + 1;
                end = gridStr.find(',', start);
            }

            // Handle the last element
            std::string gridVar = gridStr.substr(start);
            if (!gridVar.empty()) {
                if (gridVar != "X" && gridVar != "Q" && gridVar != "Z" && gridVar != "PhPerp") {
                    LOG_ERROR("Invalid grid variable: " + gridVar);
                    exit(1);
                }
                for (const auto& existingVar : args.grid) {
                    if (existingVar == gridVar) {
                        LOG_ERROR("Duplicate grid variable: " + gridVar);
                        exit(1);
                    }
                }
                args.grid.push_back(gridVar);
            }
        } else if (arg == "--bin_index" && i + 1 < argc) {
            args.bin_index = std::stoi(argv[++i]);
        } else if (arg == "--bin_index_start" && i + 1 < argc) {
            args.bin_index_start = std::stoi(argv[++i]);
        } else if (arg == "--bin_index_end" && i + 1 < argc) {
            args.bin_index_end = std::stoi(argv[++i]);
        } else if (arg == "--extract_with_true" && i + 1 < argc) {
            std::string val = argv[++i];
            args.extract_with_true = (val == "1" || val == "true" || val == "True" || val == "TRUE" || val == "t" || val == "T" || val == "yes" || val == "Yes" || val == "YES");
        } else if (arg == "--A_opt" && i + 1 < argc) {
            args.A_opt = std::stod(argv[++i]);
        } else if (!arg.empty() && arg[0] != '-') {
            // treat as positional argument if not a flag
            if (args.filename.empty()) {
                args.filename = arg;
            } else if (args.treename.empty()) {
                args.treename = arg;
            } else if (args.energyConfig.empty()) {
                args.energyConfig = arg;
            } else {
                LOG_ERROR("Unknown argument: " + arg);
                exit(1);
            }
        } else {
            // Unrecognized flag
            if (arg.rfind("--", 0) == 0 || arg.rfind("-", 0) == 0) {
                LOG_ERROR("Unknown argument: " + arg);
                exit(1);
            }
        }
    }

    // Require filename, treename, and energyConfig
    if (args.filename.empty() || args.treename.empty() || args.energyConfig.empty()) {
        LOG_INFO("Missing required parameters. Use --help for usage.");
        exit(1);
    }

    return args;
}
