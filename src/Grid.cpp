#include "Grid.h"

Grid::Grid(const std::vector<std::string>& names) : binNames(names) {}

void Grid::addBin(const std::vector<double>& mins, const std::vector<double>& maxs) {
    // Compose bin key from mins/maxs
    std::string key;
    for (size_t i = 0; i < binNames.size(); ++i) {
        key += binNames[i] + "[" + std::to_string(mins[i]) + "," + std::to_string(maxs[i]) + "]";
        if (i < binNames.size() - 1) key += ",";
    }
    bins[key] = Bin(
        binNames[0] == "X" ? mins[0] : 0,
        binNames[0] == "X" ? maxs[0] : 0,
        binNames[0] == "Q" ? mins[0] : 0,
        binNames[0] == "Q" ? maxs[0] : 0,
        binNames[0] == "Z" ? mins[0] : 0,
        binNames[0] == "Z" ? maxs[0] : 0,
        binNames[0] == "PhPerp" ? mins[0] : 0,
        binNames[0] == "PhPerp" ? maxs[0] : 0
    );
}

const std::map<std::string, Bin>& Grid::getBins() const {
    return bins;
}

std::vector<std::string> Grid::getBinNames() const {
    return binNames;
}

void Grid::printGridSummary() const {
    std::cout << "Grid bin names: ";
    for (const auto& name : binNames) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    std::cout << "Total bins: " << bins.size() << std::endl;
    for (const auto& [key, bin] : bins) {
        std::cout << "Bin: " << key << ", count: " << bin.getCount() << std::endl;
        std::cout << "  X: [" << bin.getMin("X") << ", " << bin.getMax("X") << "]";
        std::cout << " Q: [" << bin.getMin("Q") << ", " << bin.getMax("Q") << "]";
        std::cout << " Z: [" << bin.getMin("Z") << ", " << bin.getMax("Z") << "]";
        std::cout << " PhPerp: [" << bin.getMin("PhPerp") << ", " << bin.getMax("PhPerp") << "]" << std::endl;
    }
}

