
#include "Grid.h"
#include <algorithm>

Grid::Grid(const std::vector<std::string>& mainNames)
    : mainBinNames(mainNames) {}

void Grid::addBin(const std::map<std::string, std::pair<double, double>>& binRanges) {
    // Main bin key: combo of mainBinNames
    std::string mainKey = "";
    std::vector<double> mainBinLeft;
    for (const auto& name : binNames) {  
        if (std::find(mainBinNames.begin(), mainBinNames.end(), name) != mainBinNames.end()) {
            auto it = binRanges.find(name);
            if (it != binRanges.end()) {
                mainKey += name + "[" + std::to_string(it->second.first) + "," + std::to_string(it->second.second) + "]";
                mainBinLeft.push_back(it->second.first);
            }
        }
    }
    // The structure of mainKey is like "X[0.1,0.2]Q[1.0,2.0]"
    // e.g. for mainBinNames = {"X", "Q"}  

    // If mainKey not in map, initialize
    if (mainBins.find(mainKey) == mainBins.end()) {
        mainBins[mainKey] = Bin();
        mainBinLefts[mainKey] = mainBinLeft;
    }
    mainBins[mainKey].incrementCount();

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
    LOG_DEBUG("Grid main bin names: ");
    for (const auto& name : mainBinNames) {
        LOG_DEBUG(name);
    }
    LOG_DEBUG("");
    int count = 0;
    int totalBins = 0;
    for (const auto& bin : mainBins) {
        totalBins += bin.second.getCount();
        if (maxEntries > 0 && count >= maxEntries) {continue;}
        LOG_DEBUG(std::string("Main bin: ") + bin.first);
        LOG_DEBUG(std::string("  Count: ") + std::to_string(bin.second.getCount()));
        for (const auto& name : binNames) {
            LOG_DEBUG(std::string("  ") + name + " range: [" + std::to_string(bin.second.getMin(name)) + ", " + std::to_string(bin.second.getMax(name)) + "]");
        }
        // Print mainBinIndices if available
        auto idxIt = mainBinIndices.find(bin.first);
        if (idxIt != mainBinIndices.end()) {
            std::string idxStr = "  Indices: [";
            for (size_t i = 0; i < idxIt->second.size(); ++i) {
                idxStr += std::to_string(idxIt->second[i]);
                if (i + 1 < idxIt->second.size()) idxStr += ", ";
            }
            idxStr += "]";
            LOG_DEBUG(idxStr);
        }
        ++count;
    }
    LOG_DEBUG(std::string("Total main bins: ") + std::to_string(mainBins.size()));
    LOG_DEBUG(std::string("Total bins: ") + std::to_string(totalBins));
}


// Compute integer indices for each main bin using sorted lefts (argsort-like)
void Grid::computeMainBinIndices() {
    // Hierarchical indexing for N dimensions
    // Prepare a vector of maps for each dimension: parent_key -> sorted unique values
    size_t ndim = mainBinNames.size();
    // For dim 0, parent_key is ""
    std::map<std::string, std::vector<double>> uniqueByParent[ndim];
    // First, collect all lefts by parent key for each dimension
    for (const auto& pair : mainBinLefts) {
        const auto& lefts = pair.second;
        std::string parent = "";
        for (size_t d = 0; d < ndim; ++d) {
            uniqueByParent[d][parent].push_back(lefts[d]);
            // For next dim, parent is concatenation of previous lefts
            parent += (d > 0 ? "," : "") + std::to_string(lefts[d]);
        }
    }
    // Sort and deduplicate
    for (size_t d = 0; d < ndim; ++d) {
        for (auto& kv : uniqueByParent[d]) {
            auto& vec = kv.second;
            std::sort(vec.begin(), vec.end());
            vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
        }
    }
    // Now assign indices
    for (const auto& pair : mainBinLefts) {
        const auto& key = pair.first;
        const auto& lefts = pair.second;
        std::vector<int> indices(ndim);
        std::string parent = "";
        for (size_t d = 0; d < ndim; ++d) {
            const auto& vec = uniqueByParent[d][parent];
            auto it = std::find(vec.begin(), vec.end(), lefts[d]);
            indices[d] = (it != vec.end()) ? std::distance(vec.begin(), it) : -1;
            parent += (d > 0 ? "," : "") + std::to_string(lefts[d]);
        }
        mainBinIndices[key] = indices;
    }
}