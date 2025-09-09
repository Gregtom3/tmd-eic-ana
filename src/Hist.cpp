#include "Hist.h"
#include "Logger.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TApplication.h"

// Pre-determined params for recognized variables
const std::map<std::string, HistParams> Hist::varParams = {
    {"X", {100, 0, 10}},
    {"Q", {100, 1, 1000}},
    {"Z", {100, 0, 1}},
    {"PhPerp", {100, 0, 5}},
    {"Phi", {100, 0, 2 * 3.14159}}
};
const HistParams Hist::defaultParams = {100, 0, 1};

HistParams Hist::getParams(const std::string& var, int nbins, double xmin, double xmax) const {
    auto it = varParams.find(var);
    HistParams params = (it != varParams.end()) ? it->second : defaultParams;
    if (nbins > 0) params.nbins = nbins;
    if (xmin >= 0) params.xmin = xmin;
    if (xmax >= 0) params.xmax = xmax;
    return params;
}

Hist::Hist(TTree* tree) : tree(tree) {}

void Hist::plot1D(const std::string& var, const TCut& cut, int nbins, double xmin, double xmax) {
    if (!tree) {
        LOG_ERROR("TTree pointer is null in Hist::plot1D");
        return;
    }

    // Generate custom histName from cut
    std::string histName = "hist1D";
    if (cut) {
        histName += "_" + std::string(cut.GetTitle());
    }
    histName += "_" + var;

    LOG_INFO("Applying TCut: " + std::string(cut.GetTitle()));
    HistParams params = getParams(var, nbins, xmin, xmax);
    TApplication app("app", nullptr, nullptr);
    TCanvas* c = new TCanvas();
    TH1D* hist = new TH1D(histName.c_str(), (var + ";" + var).c_str(), params.nbins, params.xmin, params.xmax);
    tree->Draw((var + ">>" + histName).c_str(), cut);
    hist->Draw("hist");
    app.Run(true);
}
