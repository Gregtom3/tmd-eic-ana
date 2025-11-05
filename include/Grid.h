#ifndef GRID_H
#define GRID_H
#include "Bin.h"
#include "Logger.h"
#include "Constants.h"
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

class Grid {
public:
    Grid(channel_type channel, const std::vector<std::string>& mainBinNames);
    void addBin(const std::map<std::string, std::pair<double, double>>& binRanges);
    std::vector<std::string> getBinNames() const;
    std::vector<std::string> getMainBinNames() const;
    void printGridSummary(int maxEntries = -1) const;
    void computeMainBinIndices();
    const std::map<std::string, std::unique_ptr<Bin>>& getBins() const;
    const std::map<std::string, std::vector<int>>& getMainBinIndices() const {
        return mainBinIndices;
    }
    const Bin& getBinByIndex(int index) const {
        if (index < 0 || static_cast<size_t>(index) >= mainBins.size()) {
            LOG_ERROR("Grid: getBinByIndex: index out of range: " + std::to_string(index));
            static Bin empty;
            return empty;
        }
        auto it = mainBins.begin();
        std::advance(it, index);
        return *(it->second);
    }

private:
    channel_type channel;
    const std::vector<std::string> binNames = {"X", "Q", "Z", "PhPerp", "Mh"};
    std::vector<std::string> mainBinNames;
    std::map<std::string, std::vector<double>> mainBinLefts;
    std::map<std::string, std::vector<double>> mainBinRights;
    std::map<std::string, std::unique_ptr<Bin>> mainBins;
    std::map<std::string, std::vector<int>> mainBinIndices;
};

#endif // GRID_H
