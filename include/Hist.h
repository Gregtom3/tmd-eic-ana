#ifndef HIST_H
#define HIST_H
#include "TCut.h"
#include "TH1.h" // switch to base class
#include "TTree.h"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
class TFile; // forward decl

struct HistParams {
    int nbins;
    double xmin;
    double xmax;
};

class Hist {
public:
    Hist(TTree* tree);
    void fillHistograms(const std::string& var, const std::map<std::string, TCut>& binTCuts, double scale = 1.0);
    HistParams getDefaultParams() const {
        return defaultParams;
    }
    static const std::map<std::string, HistParams>& getVarParams() {
        return varParams;
    }
    // Caching helpers
    bool loadHistCache(const std::string& cacheFile, const std::string& var);
    bool saveHistCache(const std::string& cacheFile, const std::string& var) const;
    // Save/load meanMap to/from cache
    bool saveMeanCache(const std::string& cacheFile, const std::string& var) const;
    bool loadMeanCache(const std::string& cacheFile, const std::string& var);
    const auto& getHistMap() const {
        return histMap;
    }
    const auto& getBinKeysMap() const {
        return binKeysMap;
    }
    const auto& getMeans() const {
        return meanMap;
    }

private:
    TTree* tree;
    bool m_hasWeightBranch;
    static const std::map<std::string, HistParams> varParams;
    static const HistParams defaultParams;
    HistParams getParams(const std::string& var, int nbins, double xmin, double xmax) const;
    std::unordered_map<std::string, std::vector<TH1*>> histMap;                       // store generic TH1*
    std::unordered_map<std::string, std::vector<std::string>> binKeysMap;             // var -> list of bin keys
    std::unordered_map<std::string, std::vector<TCut>> binCutsMap;                    // var -> list of cuts
    std::unordered_map<std::string, std::unordered_map<std::string, double>> meanMap; // var -> binKey -> mean
};
#endif // HIST_H
