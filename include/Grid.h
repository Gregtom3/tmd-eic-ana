#ifndef GRID_H
#define GRID_H
#include "Bin.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>

class Grid {
public:
    Grid(const std::vector<std::string>& binNames);
    void addBin(const std::vector<double>& mins, const std::vector<double>& maxs);
    const std::map<std::string, Bin>& getBins() const;
    std::vector<std::string> getBinNames() const;
    void printGridSummary() const;
private:
    std::vector<std::string> binNames;
    std::map<std::string, Bin> bins;
};

#endif // GRID_H
