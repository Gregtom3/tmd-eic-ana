#ifndef INJECTION_PROJECT_H
#define INJECTION_PROJECT_H

#include "Bin.h"
#include "Grid.h"
#include "TCut.h"
#include "TTree.h"
#include "Table.h"
#include "Inject.h"
#include "Logger.h"
#include <optional>
#include <string>
#include <vector>
#include <tuple>
#include <filesystem>

// Small utility to run many inject/extract trials across bins and emit a YAML summary
class InjectionProject {
public:
    
    struct Job {
        int bin_index = 0;
        int n = 1;
        bool extract_with_true = false;
        std::optional<double> A_opt;
    };

    InjectionProject(const std::string& filename, TTree* tree, const Table* table, double scale, const Grid* grid, double targetPolarization, const std::string& outDir, const std::string& outFilename);
    void addJob(const Job& job);
    bool run();

private:

    std::string filename;
    TTree* tree;
    std::string outPrefix;
    const Table* table;
    double scale;
    const Grid* grid;
    double targetPolarization;
    std::string outDir;
    std::string outFilename;
    std::vector<Job> jobs;
};

#endif // INJECTION_PROJECT_H
