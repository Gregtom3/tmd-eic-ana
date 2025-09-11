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
    void computeMainBinIndices();
    const std::map<std::string, Bin>& getBins() const;
    const std::map<std::string, std::vector<int>>& getMainBinIndices() const { return mainBinIndices; }
private:
    const std::vector<std::string> binNames = {"X", "Q", "Z", "PhPerp"};
    std::vector<std::string> mainBinNames;
    std::map<std::string, std::vector<double>> mainBinLefts;
    std::map<std::string, std::vector<double>> mainBinRights;
    std::map<std::string, Bin> mainBins;
    std::map<std::string, std::vector<int>> mainBinIndices;

};

#endif // GRID_H
