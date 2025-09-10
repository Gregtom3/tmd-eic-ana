#ifndef INJECT_H
#define INJECT_H

#include <string>
#include <vector>
#include <memory>
#include "TTree.h"
#include "TCut.h"
#include "Bin.h"
#include "Grid.h"
#include "Table.h"

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
