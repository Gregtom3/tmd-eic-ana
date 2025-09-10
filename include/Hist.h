#ifndef HIST_H
#define HIST_H
#include "TTree.h"
#include "TCut.h"
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include "TH1.h" // switch to base class
class TFile; // forward decl

struct HistParams {
    int nbins;
    double xmin;
    double xmax;
};

class Hist {
public:
    Hist(TTree* tree);
    void plot1D(const std::string& var, const TCut& cut, int nbins = -1, double xmin = -1, double xmax = -1);
    void fillHistograms(const std::string& var, const std::map<std::string, TCut>& binTCuts);
    void plotBin(const std::string& var, size_t binIndex);
    void plot2DMap(const std::string& var);
    HistParams getDefaultParams() const { return defaultParams; }
    static const std::map<std::string, HistParams>& getVarParams() { return varParams; }
    // Caching helpers
    bool loadHistCache(const std::string& cacheFile, const std::string& var);
    bool saveHistCache(const std::string& cacheFile, const std::string& var) const;
    // Save/load meanMap to/from cache
    bool saveMeanCache(const std::string& cacheFile, const std::string& var) const;
    bool loadMeanCache(const std::string& cacheFile, const std::string& var);
private:
    TTree* tree;
    static const std::map<std::string, HistParams> varParams;
    static const HistParams defaultParams;
    HistParams getParams(const std::string& var, int nbins, double xmin, double xmax) const;
    void computeMeans(const std::string& binKey, const TCut& cut);
    std::unordered_map<std::string, std::vector<TH1*>> histMap; // store generic TH1*
    std::unordered_map<std::string, std::vector<std::string>> binKeysMap; // var -> list of bin keys
    std::unordered_map<std::string, std::vector<TCut>> binCutsMap; // var -> list of cuts
    std::unordered_map<std::string, std::unordered_map<std::string, double>> meanMap; // var -> binKey -> mean
};
#endif // HIST_H
