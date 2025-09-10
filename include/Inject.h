#ifndef INJECT_H
#define INJECT_H

#include <string>
#include <vector>
#include <memory>
#include "TTree.h"
#include "TCut.h"
#include "Bin.h"
#include "Grid.h"

class Inject {
public:
    Inject(TTree* tree);
    ~Inject();
    // Inject and extract asymmetry for a user-desired bin
    std::pair<double, double> injectExtractForBin(const Bin& bin, double A = 0.1);
private:
    TTree* tree;
};

#endif // INJECT_H
