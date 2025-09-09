#ifndef GRID_H
#define GRID_H
#include "Bin.h"
#include "Logger.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>

class Grid {
public:
    Grid(const std::vector<std::string>& mainBinNames);
    void addBin(const std::map<std::string, std::pair<double, double>>& binRanges);
    std::vector<std::string> getBinNames() const;
    std::vector<std::string> getMainBinNames() const;
    void printGridSummary() const;
    const std::map<std::string, Bin>& getBins() const;
private:
    const std::vector<std::string> binNames = {"X", "Q", "Z", "PhPerp"};
    std::vector<std::string> mainBinNames;
    std::map<std::string, Bin> mainBins;
};

#endif // GRID_H
