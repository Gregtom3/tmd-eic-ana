#include "Hist.h"
#include "Logger.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TKey.h"
#include "Style.h"
#include "TLatex.h"
#include <memory>
#include <iostream>

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

    int totalBins = static_cast<int>(binTCuts.size());
    int idx = 0;

    for (const auto& binPair : binTCuts) {
        // Progress bar
        static const int barWidth = 50;
        float progressRatio = static_cast<float>(idx + 1) / totalBins;
        int pos = static_cast<int>(barWidth * progressRatio);
        std::cout << "[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progressRatio * 100.0) << "%\r";
        std::cout.flush();

        const std::string& binKey = binPair.first;
        const TCut& cut = binPair.second;
        std::string histName = "hist_" + binKey;
        std::string drawCmd = var + ">>" + histName + "(" +
            std::to_string(params.nbins) + "," +
            std::to_string(params.xmin) + "," +
            std::to_string(params.xmax) + ")";
        tree->Draw(drawCmd.c_str(), cut, "goff");

        TH1D* h = static_cast<TH1D*>(gDirectory->Get(histName.c_str()));
        if (!h) {
            std::cerr << "Warning: Histogram " << histName << " not found after Draw.\n";
            ++idx;
            continue;
        }
        histMap[var].push_back(h);
        binKeysMap[var].push_back(binKey);
        binCutsMap[var].push_back(cut);
        // Compute means for this bin only
        computeMeans(binKey, cut);
        ++idx;
    }
    std::cout << std::endl; // newline after progress bar
}

void Hist::plot2DMap(const std::string& var){
    TApplication app("app", nullptr, nullptr);
    TCanvas * c = new TCanvas("c2d", "2D Map", 800, 800);
    c->Divide(2,2);
    ApplyGlobalStyle();
    for(size_t binIndex = 0; binIndex < 4; ++binIndex){
        c->cd(binIndex+1);
        ApplyHistStyle(histMap[var][binIndex]);
        histMap[var][binIndex]->Draw();
        //histMap[var][binIndex]->SetTitle("");
    }
    c->Update();
    app.Run();
    delete c; c=nullptr;
}

void Hist::plotBin(const std::string& var, size_t binIndex) {
    if (histMap.find(var) == histMap.end() || binIndex >= histMap.at(var).size()) {
        std::cerr << "Invalid bin index or variable: " << var << ", " << binIndex << std::endl;
        return;
    }
    TApplication app("app", nullptr, nullptr);
    TCanvas* c = new TCanvas("c","c",800,600);
    ApplyGlobalStyle();
    ApplyHistStyle(histMap[var][binIndex]);
    histMap[var][binIndex]->Draw();

    const std::string& binKey = binKeysMap[var][binIndex];
    std::vector<std::string> meanVars = {"X", "Q", "Z", "PhPerp"};
    int meanPrecision = 3;
    DrawMeanTLatex(meanMap[binKey], meanVars, meanPrecision, 0.15, 0.92);

    c->Update();
    app.Run();
    delete c; c=nullptr;
}

bool Hist::saveHistCache(const std::string& cacheFile, const std::string& var) const {
    auto it = histMap.find(var);
    if (it == histMap.end() || it->second.empty()) return false;

    std::unique_ptr<TFile> f(TFile::Open(cacheFile.c_str(), "RECREATE"));
    if (!f || f->IsZombie()) {
        LOG_ERROR("Failed to create cache file: " + cacheFile);
        return false; 
    }

    f->mkdir(var.c_str());
    f->cd(var.c_str());

    const auto& hists = it->second;
    for (size_t i = 0; i < hists.size(); ++i) {
        TH1* h = hists[i];
        if (!h) continue;
        std::string binKey = binKeysMap.at(var)[i];
        std::string cutExpr = binCutsMap.at(var)[i].GetTitle();
        h->SetName(binKey.c_str());
        h->SetTitle(cutExpr.c_str());
        h->Write();
    }
    f->Write();
    LOG_INFO("Saved histogram cache: " + cacheFile);
    return true;
}

// Compute means for a single bin using temporary 1D histograms
void Hist::computeMeans(const std::string& binKey, const TCut& cut) {
    std::vector<std::string> vars = {"X", "Q", "Z", "PhPerp"};
    for (const auto& var : vars) {
        std::string histName = "mean_tmp_" + var + "_" + binKey;
        auto params = getParams(var, -1, -1, -1);
        std::string drawCmd = var + ">>" + histName + "(" +
            std::to_string(params.nbins) + "," +
            std::to_string(params.xmin) + "," +
            std::to_string(params.xmax) + ")";
        tree->Draw(drawCmd.c_str(), cut, "goff");
        TH1D* h = static_cast<TH1D*>(gDirectory->Get(histName.c_str()));
        double mean = h ? h->GetMean() : 0.0;
        meanMap[binKey][var] = mean;
        if (h) { h->Delete(); }
    }
}

// Save meanMap to cache as a TTree
bool Hist::saveMeanCache(const std::string& cacheFile, const std::string& var) const {
    std::unique_ptr<TFile> f(TFile::Open(cacheFile.c_str(), "UPDATE"));
    if (!f || f->IsZombie()) return false;
    std::string treeName = var + "_means";
    TTree* t = new TTree(treeName.c_str(), "Bin means");
    char binKey[256];
    double X, Q, Z, PhPerp;
    t->Branch("binKey", binKey, "binKey/C");
    t->Branch("X", &X, "X/D");
    t->Branch("Q", &Q, "Q/D");
    t->Branch("Z", &Z, "Z/D");
    t->Branch("PhPerp", &PhPerp, "PhPerp/D");
    for (const auto& binPair : meanMap) {
        strncpy(binKey, binPair.first.c_str(), 255);
        binKey[255] = '\0';
        X = binPair.second.at("X");
        Q = binPair.second.at("Q");
        Z = binPair.second.at("Z");
        PhPerp = binPair.second.at("PhPerp");
        t->Fill();
    }
    f->cd();
    t->Write(treeName.c_str(), TObject::kOverwrite);
    f->Write();
    delete t;
    return true;
}

// Load meanMap from cache TTree
bool Hist::loadMeanCache(const std::string& cacheFile, const std::string& var) {
    std::unique_ptr<TFile> f(TFile::Open(cacheFile.c_str(), "READ"));
    if (!f || f->IsZombie()) return false;
    std::string treeName = var + "_means";
    TTree* t = (TTree*)f->Get(treeName.c_str());
    if (!t) return false;
    char binKey[256];
    double X, Q, Z, PhPerp;
    t->SetBranchAddress("binKey", binKey);
    t->SetBranchAddress("X", &X);
    t->SetBranchAddress("Q", &Q);
    t->SetBranchAddress("Z", &Z);
    t->SetBranchAddress("PhPerp", &PhPerp);
    meanMap.clear();
    Long64_t nentries = t->GetEntries();
    for (Long64_t i = 0; i < nentries; ++i) {
        t->GetEntry(i);
        meanMap[binKey]["X"] = X;
        meanMap[binKey]["Q"] = Q;
        meanMap[binKey]["Z"] = Z;
        meanMap[binKey]["PhPerp"] = PhPerp;
    }
    return true;
}

bool Hist::loadHistCache(const std::string& cacheFile, const std::string& var) {
    std::unique_ptr<TFile> f(TFile::Open(cacheFile.c_str(), "READ"));
    if (!f || f->IsZombie()) {
        return false;
    }
    TDirectory* dir = dynamic_cast<TDirectory*>(f->Get(var.c_str()));
    if (!dir) {
        return false;
    }
    dir->cd();
    histMap[var].clear();
    binKeysMap[var].clear();
    binCutsMap[var].clear();
    TIter next(dir->GetListOfKeys());
    TKey* key;
    while ((key = (TKey*)next())) {
        TObject* obj = key->ReadObj();
        TH1* h = dynamic_cast<TH1*>(obj);
        if (!h) continue;
        TH1* hc = (TH1*)h->Clone();
        hc->SetDirectory(nullptr); // detach from file
        histMap[var].push_back(hc);
        std::string binKey = h->GetName();
        binKeysMap[var].push_back(binKey);
        std::string cutExpr = h->GetTitle();
        binCutsMap[var].push_back(TCut(cutExpr.c_str()));
    }
    bool ok = !histMap[var].empty();
    if (ok) {
        LOG_INFO("Loaded histogram cache from: " + cacheFile);
    }
    return ok;
}