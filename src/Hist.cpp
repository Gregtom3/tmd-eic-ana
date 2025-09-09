#include "Hist.h"
#include "Logger.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TDirectory.h"

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

void Hist::fillHistograms(const std::string& var, const std::map<std::string, TCut>& binTCuts) {
    histMap[var].clear();
    binKeysMap[var].clear();
    binCutsMap[var].clear();

    auto params = getParams(var, -1, -1, -1);

    for (const auto& binPair : binTCuts) {
        const std::string& binKey = binPair.first;
        const TCut& cut = binPair.second;

        std::string histName = "hist_" + binKey;

        // Draw directly into a histogram managed by ROOT
        std::string drawCmd = var + ">>" + histName + "(" + 
                      std::to_string(params.nbins) + "," + 
                      std::to_string(params.xmin) + "," + 
                      std::to_string(params.xmax) + ")";
        tree->Draw(drawCmd.c_str(), cut, "goff");

        // Retrieve it from gDirectory
        TH1D* h = static_cast<TH1D*>(gDirectory->Get(histName.c_str()));

        if (!h) {
            std::cerr << "Warning: Histogram " << histName << " not found after Draw.\n";
            continue;
        }

        histMap[var].push_back(h);
        binKeysMap[var].push_back(binKey);
        binCutsMap[var].push_back(cut);

        break; // DEBUG
    }
}

void Hist::plotBin(const std::string& var, size_t binIndex) {
    if (histMap.find(var) == histMap.end() ||
        binIndex >= histMap.at(var).size()) {
        std::cerr << "Invalid bin index or variable: " << var << ", " << binIndex << std::endl;
        return;
    }

    std::cout << "TCut for bin " << binIndex << ": " << binCutsMap[var][binIndex].GetTitle() << std::endl;
    std::cout << "Bin key: " << binKeysMap[var][binIndex] << std::endl;

    TApplication app("app", nullptr, nullptr);
    TCanvas* c = new TCanvas("c","c",800,600);
    histMap[var][binIndex]->Draw();
    c->Update();
    app.Run();
}