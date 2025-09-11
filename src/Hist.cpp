#include "Hist.h"
#include "Logger.h"
#include "Style.h"
#include "TApplication.h"
#include "TArrow.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TEntryList.h"
#include "TFile.h"
#include "TH1D.h"
#include "TKey.h"
#include "TLatex.h"
#include "TTreeFormula.h"
#include "Utility.h"
#include <iostream>
#include <memory>

// Pre-determined params for recognized variables
const std::map<std::string, HistParams> Hist::varParams = {
    {"X", {100, 0, 10}}, {"Q", {100, 1, 1000}}, {"Z", {100, 0, 1}}, {"PhPerp", {100, 0, 5}}, {"Phi", {100, 0, 2 * 3.14159}}};
const HistParams Hist::defaultParams = {100, 0, 1};

HistParams Hist::getParams(const std::string& var, int nbins, double xmin, double xmax) const {
    auto it = varParams.find(var);
    HistParams params = (it != varParams.end()) ? it->second : defaultParams;
    if (nbins > 0)
        params.nbins = nbins;
    if (xmin >= 0)
        params.xmin = xmin;
    if (xmax >= 0)
        params.xmax = xmax;
    return params;
}

Hist::Hist(TTree* tree)
    : tree(tree) {
    m_hasWeightBranch = tree->GetBranch("Weight") != nullptr;
    if (m_hasWeightBranch) {
        LOG_INFO("TTree has a 'Weight' branch. It will be used in histogram filling.");
    }
}

void Hist::fillHistograms(const std::string& var, const std::map<std::string, TCut>& binTCuts, double scale) {
    // Prepare containers
    histMap[var].clear();
    binKeysMap[var].clear();
    binCutsMap[var].clear();

    auto params = getParams(var, -1, -1, -1);

    // Pre-create histograms and compile TTreeFormulas for cuts
    std::vector<std::unique_ptr<TTreeFormula>> binFormulas;
    std::vector<TH1D*> hists;
    std::vector<std::string> keys;
    std::vector<TCut> cuts;

    int idx = 0;
    for (const auto& binPair : binTCuts) {
        const std::string& binKey = binPair.first;
        const TCut& cut = binPair.second;
        std::string histName = "hist_" + binKey;
        TH1D* h = new TH1D(histName.c_str(), histName.c_str(), params.nbins, params.xmin, params.xmax);
        h->SetDirectory(nullptr);
        hists.push_back(h);
        keys.push_back(binKey);
        cuts.push_back(cut);
        // compile formula for the bin cut
        binFormulas.emplace_back(std::make_unique<TTreeFormula>(("cut_" + std::to_string(idx)).c_str(), cut.GetTitle(), tree));
        ++idx;
    }

    int totalBins = static_cast<int>(hists.size());
    if (totalBins == 0)
        return;

    // Prepare formulas for variable, weight, and mean variables
    std::unique_ptr<TTreeFormula> varFormula = std::make_unique<TTreeFormula>("varFormula", var.c_str(), tree);
    std::unique_ptr<TTreeFormula> weightFormula;
    if (m_hasWeightBranch)
        weightFormula = std::make_unique<TTreeFormula>("weightFormula", "Weight", tree);
    std::vector<std::string> meanVars = {"X", "Q", "Z", "PhPerp"};
    std::vector<std::unique_ptr<TTreeFormula>> meanFormulas;
    for (const auto& mvar : meanVars) {
        meanFormulas.emplace_back(std::make_unique<TTreeFormula>(("mean_" + mvar).c_str(), mvar.c_str(), tree));
    }

    // Accumulators for means: per-bin per-meanVar
    std::vector<std::vector<double>> sumW(totalBins, std::vector<double>(1, 0.0)); // total weight per bin
    std::vector<std::vector<double>> sumWV(totalBins, std::vector<double>(meanVars.size(), 0.0));

    // Single pass over entries with progress
    TEntryList* el = tree->GetEntryList();
    Long64_t nentries = (el ? el->GetN() : tree->GetEntries());
    util::ProgressBar pbar(static_cast<size_t>(nentries), 60, "Filling");
    for (Long64_t i = 0; i < nentries; ++i) {
        tree->GetEntry(i);

        double v = varFormula->EvalInstance();
        double w = m_hasWeightBranch ? weightFormula->EvalInstance() : 1.0;
        w *= scale;
        // evaluate mean vars once
        std::vector<double> mvals;
        mvals.reserve(meanFormulas.size());
        for (auto& mf : meanFormulas)
            mvals.push_back(mf->EvalInstance());

        // check each bin formula
        for (int b = 0; b < totalBins; ++b) {
            double pass = binFormulas[b]->EvalInstance();
            if (pass) {
                hists[b]->Fill(v, w);
                sumW[b][0] += w;
                for (size_t k = 0; k < meanVars.size(); ++k) {
                    sumWV[b][k] += mvals[k] * w;
                }
            }
        }

        if ((i & 0x3FF) == 0)
            pbar.update(static_cast<size_t>(i));
    }
    pbar.finish();

    // Move histograms and metadata into maps, compute means
    for (int b = 0; b < totalBins; ++b) {
        histMap[var].push_back(hists[b]);
        binKeysMap[var].push_back(keys[b]);
        binCutsMap[var].push_back(cuts[b]);
        // compute means for stored vars
        double totalW = sumW[b][0];
        std::string binKey = keys[b];
        if (totalW > 0.0) {
            for (size_t k = 0; k < meanVars.size(); ++k) {
                meanMap[binKey][meanVars[k]] = sumWV[b][k] / totalW;
            }
        } else {
            for (const auto& mv : meanVars)
                meanMap[binKey][mv] = 0.0;
        }
    }

    std::cout << "Processed " << nentries << " entries; filled " << totalBins << " histograms." << std::endl;
}

bool Hist::saveHistCache(const std::string& cacheFile, const std::string& var) const {
    auto it = histMap.find(var);
    if (it == histMap.end() || it->second.empty())
        return false;

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
        if (!h)
            continue;
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

// Save meanMap to cache as a TTree
bool Hist::saveMeanCache(const std::string& cacheFile, const std::string& var) const {
    std::unique_ptr<TFile> f(TFile::Open(cacheFile.c_str(), "UPDATE"));
    if (!f || f->IsZombie())
        return false;
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
    if (!f || f->IsZombie())
        return false;
    std::string treeName = var + "_means";
    TTree* t = (TTree*)f->Get(treeName.c_str());
    if (!t)
        return false;
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
        if (!h)
            continue;
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