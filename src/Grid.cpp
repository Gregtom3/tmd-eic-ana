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

void Grid::printGridSummary() const {
    std::cout << "Grid main bin names: ";
    for (const auto& name : mainBinNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    for (const auto& bin : mainBins) {
        std::cout << "Main bin: " << bin.first << std::endl;

        std::cout << "  Count: " << bin.second.getCount() << std::endl;
        for (const auto& name : binNames) {
            std::cout << "  " << name << " range: [" << bin.second.getMin(name) << ", " << bin.second.getMax(name) << "]" << std::endl;
        }
    }
    std::cout << "Total main bins: " << mainBins.size() << std::endl;
    std::cout << "Total bins: " << mainBins.size() * binNames.size() << std::endl;
}

