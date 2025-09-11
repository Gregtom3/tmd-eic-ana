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

class Inject {
public:
    Inject(TTree* tree, const Table* table);
    ~Inject();
    std::pair<double, double> injectExtractForBin(const Bin& bin, double A);

private:
    TTree* tree;
    const Table* table;
};

#endif // INJECT_H
