#include "Grid.h"
#include <algorithm>

Grid::Grid(const std::vector<std::string>& mainNames)
    : mainBinNames(mainNames) {}

void Grid::addBin(const std::map<std::string, std::pair<double, double>>& binRanges) {
    // Main bin key: combo of mainBinNames
    std::string mainKey = "";
    for (const auto& name : binNames) {
        if (std::find(mainBinNames.begin(), mainBinNames.end(), name) != mainBinNames.end()) {
            auto it = binRanges.find(name);
            if (it != binRanges.end()) {
                mainKey += name + "[" + std::to_string(it->second.first) + "," + std::to_string(it->second.second) + "]";
                mainBins[mainKey] = Bin(); // Initialize bin
            }
        }
        mainBins[mainKey].incrementCount();
    }
    for (const auto& name : binNames) {
        auto it = binRanges.find(name);
        if (it != binRanges.end()) {
            mainBins[mainKey].updateMin(name, it->second.first);
            mainBins[mainKey].updateMax(name, it->second.second);
        }
    }
}

const std::map<std::string, Bin>& Grid::getBins() const {
    return mainBins;
}

std::vector<std::string> Grid::getBinNames() const {
    return binNames;
}

std::vector<std::string> Grid::getMainBinNames() const {
    return mainBinNames;
}

void Grid::printGridSummary(int maxEntries) const {
    LOG_INFO("Grid main bin names: ");
    for (const auto& name : mainBinNames) {
        LOG_INFO(name);
    }
    LOG_INFO("");
    int count = 0;
    for (const auto& bin : mainBins) {
        if (maxEntries > 0 && count >= maxEntries) break;
        LOG_INFO(std::string("Main bin: ") + bin.first);
        LOG_INFO(std::string("  Count: ") + std::to_string(bin.second.getCount()));
        for (const auto& name : binNames) {
            LOG_INFO(std::string("  ") + name + " range: [" + std::to_string(bin.second.getMin(name)) + ", " + std::to_string(bin.second.getMax(name)) + "]");
        }
        ++count;
    }
    LOG_INFO(std::string("Total main bins: ") + std::to_string(mainBins.size()));
    LOG_INFO(std::string("Total bins: ") + std::to_string(mainBins.size() * binNames.size()));
}

