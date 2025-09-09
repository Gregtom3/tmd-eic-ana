#ifndef HIST_H
#define HIST_H
#include "TTree.h"
#include "TCut.h"
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include "TH1D.h"

struct HistParams {
    int nbins;
    double xmin;
    double xmax;
};

class Hist {
public:
    Hist(TTree* tree);
    void plot1D(const std::string& var, const TCut& cut, int nbins = -1, double xmin = -1, double xmax = -1);
    void plotBin1D(size_t binIndex, const std::string& var, const TCut& cut, const std::string& binKey, int nbins = -1, double xmin = -1, double xmax = -1);
    void fillHistograms(const std::string& var, const std::map<std::string, TCut>& binTCuts);
    void plotBin(const std::string& var, size_t binIndex);
    HistParams getDefaultParams() const { return defaultParams; }
    static const std::map<std::string, HistParams>& getVarParams() { return varParams; }
private:
    TTree* tree;
    static const std::map<std::string, HistParams> varParams;
    static const HistParams defaultParams;
    HistParams getParams(const std::string& var, int nbins, double xmin, double xmax) const;
    std::unordered_map<std::string, std::vector<TH1D*>> histMap; // var -> list of hists
    std::unordered_map<std::string, std::vector<std::string>> binKeysMap; // var -> list of bin keys
    std::unordered_map<std::string, std::vector<TCut>> binCutsMap; // var -> list of cuts
};
#endif // HIST_H
