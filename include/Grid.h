#ifndef GRID_H
#define GRID_H
#include "Bin.h"
#include "Logger.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <iostream>

class Grid {
public:
    Grid(const std::vector<std::string>& mainBinNames);
    void addBin(const std::map<std::string, std::pair<double, double>>& binRanges);
    std::vector<std::string> getBinNames() const;
    std::vector<std::string> getMainBinNames() const;
    void printGridSummary(int maxEntries = -1) const; 
    const std::map<std::string, Bin>& getBins() const;
private:
    const std::vector<std::string> binNames = {"X", "Q", "Z", "PhPerp"};
    std::vector<std::string> mainBinNames;
    std::map<std::string, std::vector<double>> mainBinMeans;
    std::map<std::string, Bin> mainBins;
    std::map<std::string, std::vector<int>> mainBinIndices;
public:
    // Call after all bins are added to compute integer indices
    void computeMainBinIndices();
    const std::map<std::string, std::vector<int>>& getMainBinIndices() const { return mainBinIndices; }
};

#endif // GRID_H
