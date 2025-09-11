#ifndef INJECTION_PROJECT_H
#define INJECTION_PROJECT_H

#include "Bin.h"
#include "Grid.h"
#include "TCut.h"
#include "TTree.h"
#include "Table.h"
#include <optional>
#include <string>
#include <vector>
#include <tuple>

// Small utility to run many inject/extract trials across bins and emit a YAML summary
class InjectionProject {
public:
    InjectionProject(TTree* tree, const Table* table, double scale, const Grid* grid);

    // Add a job: run `n_injections` inject/extracts for `bin_index`. If A_opt has value,
    // use that injected asymmetry; otherwise use table.lookupAUT.
    void addJob(int bin_index, int n_injections, std::optional<double> A_opt = std::nullopt);

    // Execute all queued jobs and write results to outPrefix.yaml and per-job CSVs (outPrefix_bin<idx>.csv).
    // Returns true on success.
    bool run(const std::string& outPrefix);

private:
    struct Job {
        int bin_index;
        int n;
        std::optional<double> A_opt;
    };

    TTree* tree;
    const Table* table;
    double scale;
    const Grid* grid;
    std::vector<Job> jobs;
};

#endif // INJECTION_PROJECT_H
