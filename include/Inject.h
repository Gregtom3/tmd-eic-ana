#ifndef INJECT_H
#define INJECT_H

#include "Bin.h"
#include "Grid.h"
#include "TCut.h"
#include "TTree.h"
#include "Table.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

class Inject {
public:
    Inject(TTree* tree, const Table* table, double scale = 1.0);
    ~Inject();
    std::pair<double, double> injectExtractForBin(const Bin& bin, std::optional<double> A_opt = std::nullopt);

private:
    TTree* tree;
    const Table* table;
    double m_scale{1.0};
};

#endif // INJECT_H
