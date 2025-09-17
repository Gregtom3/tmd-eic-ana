#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>
#include <cstdint>
#include <optional>
struct Args {
    std::string filename;
    std::string treename;
    std::string energyConfig;
    bool overwrite = false;
    std::string outDir = "out";
    std::string outFilename = "";
    long long maxEntries = -1;
    double targetPolarization = 1.0;
    int n_injections = 10;
    int bin_index = 0;
    int bin_index_start = 0;
    int bin_index_end = -1;
    bool extract_with_true = false;
    std::optional<double> A_opt;
};

Args parseArgs(int argc, char** argv);

#endif // ARG_PARSER_H
