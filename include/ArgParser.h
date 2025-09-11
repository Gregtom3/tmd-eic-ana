#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>
#include <cstdint>

struct Args {
    std::string filename;
    std::string treename;
    std::string energyConfig;
    bool overwrite = false;
    std::string outDir = "out";
    long long maxEntries = -1;
};

Args parseArgs(int argc, char** argv);

#endif // ARG_PARSER_H
