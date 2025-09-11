
#include "Grid.h"
#include <algorithm>

Grid::Grid(const std::vector<std::string>& mainNames)
    : mainBinNames(mainNames) {}

void Grid::addBin(const std::map<std::string, std::pair<double, double>>& binRanges) {
    // Main bin key: combo of mainBinNames
    std::string mainKey = "";
    std::vector<double> mainBinLeft;
    std::vector<double> mainBinRight;
    for (const auto& name : binNames) {  
        if (std::find(mainBinNames.begin(), mainBinNames.end(), name) != mainBinNames.end()) {
            auto it = binRanges.find(name);
            if (it != binRanges.end()) {
                mainKey += name + "[" + std::to_string(it->second.first) + "," + std::to_string(it->second.second) + "]";
                mainBinLeft.push_back(it->second.first);
                mainBinRight.push_back(it->second.second);
            }
        }
    }
    // The structure of mainKey is like "X[0.1,0.2]Q[1.0,2.0]"
    // e.g. for mainBinNames = {"X", "Q"}  

    // If mainKey not in map, initialize
    if (mainBins.find(mainKey) == mainBins.end()) {
        mainBins[mainKey] = Bin();
        mainBinLefts[mainKey] = mainBinLeft;
        mainBinRights[mainKey] = mainBinRight;
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


// Compute integer indices for each main bin using containment-aware intervals
void Grid::computeMainBinIndices() {
    size_t ndim = mainBinNames.size();

    // Instead of storing just left edges, store full intervals [low, high]
    std::map<std::string, std::vector<std::pair<double,double>>> uniqueByParent[ndim];

    // Collect all intervals by parent key
    for (const auto& pair : mainBinLefts) {
        const auto& key    = pair.first;
        const auto& lefts  = pair.second;
        const auto& rights = mainBinRights.at(key);

        std::string parent = "";
        for (size_t d = 0; d < ndim; ++d) {
            double low  = lefts[d];
            double high = rights[d];

            uniqueByParent[d][parent].push_back({low, high});

            // Update parent string for the next dimension
            parent += (d > 0 ? "," : "") + std::to_string(low);
        }
    }

    // Debug: show collected intervals
    LOG_DEBUG("=== Collected intervals ===");
    for (size_t d = 0; d < ndim; ++d) {
        LOG_DEBUG("Dim " + std::to_string(d) + " (" + mainBinNames[d] + "):");
        for (auto& kv : uniqueByParent[d]) {
            LOG_DEBUG("  Parent " + kv.first + ": ");
            for (auto& iv : kv.second) {
                LOG_DEBUG("[" + std::to_string(iv.first) + "," + std::to_string(iv.second) + "] ");
            }
            LOG_DEBUG("\n");
        }
    }

    // Sort, deduplicate, and merge containment for each dimension
    for (size_t d = 0; d < ndim; ++d) {
        for (auto& kv : uniqueByParent[d]) {
            auto& vec = kv.second;

            // Sort by low edge, then by high edge
            std::sort(vec.begin(), vec.end(),
                      [](auto& a, auto& b) {
                          if (a.first == b.first) return a.second < b.second;
                          return a.first < b.first;
                      });

            // Merge by containment
            std::vector<std::pair<double,double>> merged;
            if (!vec.empty()) {
                auto current = vec[0];
                for (size_t i = 1; i < vec.size(); ++i) {
                    if (vec[i].first >= current.first && vec[i].second <= current.second) {
                        // vec[i] is contained in current → skip it
                        continue;
                    } else if (vec[i].first == current.first) {
                        // same start → extend
                        current.second = std::max(current.second, vec[i].second);
                    } else {
                        merged.push_back(current);
                        current = vec[i];
                    }
                }
                merged.push_back(current);
            }
            vec = merged;
        }
    }

    // Debug: show merged intervals
    LOG_DEBUG("=== Merged intervals ===");
    for (size_t d = 0; d < ndim; ++d) {
        LOG_DEBUG("Dim " + std::to_string(d) + " (" + mainBinNames[d] + "):");
        for (auto& kv : uniqueByParent[d]) {
            LOG_DEBUG("  Parent " + kv.first + ": ");
            for (auto& iv : kv.second) {
                LOG_DEBUG("[" + std::to_string(iv.first) + "," + std::to_string(iv.second) + "] ");
            }
            LOG_DEBUG("\n");
        }
    }

    // Assign indices for each bin
    for (const auto& pair : mainBinLefts) {
        const auto& key    = pair.first;
        const auto& lefts  = pair.second;
        const auto& rights = mainBinRights.at(key);

        std::vector<int> indices(ndim);

        std::string parent = "";
        for (size_t d = 0; d < ndim; ++d) {
            double low  = lefts[d];
            double high = rights[d];

            const auto& vec = uniqueByParent[d][parent];

            int idx = -1;
            for (size_t i = 0; i < vec.size(); ++i) {
                if (vec[i].first <= low && high <= vec[i].second) {
                    idx = static_cast<int>(i);
                    break;
                }
            }
            indices[d] = idx;

            parent += (d > 0 ? "," : "") + std::to_string(low);
        }

        mainBinIndices[key] = indices;

        // Debug: show assignment
        LOG_DEBUG(key + " → [");
        for (size_t i = 0; i < indices.size(); ++i) {
            LOG_DEBUG(std::to_string(indices[i]) + (i + 1 < indices.size() ? "," : ""));
        }
        LOG_DEBUG("]");
    }
}

