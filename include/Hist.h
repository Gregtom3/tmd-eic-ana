#ifndef HIST_H
#define HIST_H
#include "TTree.h"
#include "TCut.h"
#include <string>
#include <map>

struct HistParams {
    int nbins;
    double xmin;
    double xmax;
};

class Hist {
public:
    Hist(TTree* tree);
    void plot1D(const std::string& var, const TCut& cut, int nbins = -1, double xmin = -1, double xmax = -1);
private:
    TTree* tree;
    static const std::map<std::string, HistParams> varParams;
    static const HistParams defaultParams;
    HistParams getParams(const std::string& var, int nbins, double xmin, double xmax) const;
};
#endif // HIST_H
