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
#include "TArrow.h"
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

void Hist::plot2DMap(const std::string& var, const std::map<std::string, std::vector<int>>& mainBinIndices, std::vector<std::string>& axisLabels){
    int maxRow = 0;
    int maxCol = 0;
    for (const auto& pair : mainBinIndices) {
        maxCol = std::max(maxCol, pair.second.at(0));
        maxRow = std::max(maxRow, pair.second.at(1));
    }
    int nCols = maxCol + 2; // add one for leftmost blank column
    int nRows = maxRow + 2; // add one for bottommost blank row
    int windowX = 400;
    int windowY = 400;
    TCanvas * c = new TCanvas("c2d", "2D Map", windowX*nCols, windowY*nRows);
    c->Divide(nCols, nRows, 0, 0);
    ApplyGlobalStyle();

    // Set pad margins: only far left and far bottom pads have margin for axes
    double leftMargin = 0.15;
    double bottomMargin = 0.15;
    double zeroMargin = 0.0;
    for (int row = 0; row < nRows; ++row) {
        for (int col = 0; col < nCols; ++col) {
            int flippedRow = (nRows - 1) - row;
            int padIndex = col + flippedRow*nCols + 1;
            c->cd(padIndex);
            TPad* pad = (TPad*)c->GetPad(padIndex);
            if (col == 1) {
                pad->SetLeftMargin(leftMargin);
            } else {
                pad->SetLeftMargin(zeroMargin);
            }
            if (row == 1) {
                pad->SetBottomMargin(bottomMargin);
            } else {
                pad->SetBottomMargin(zeroMargin);
            }
            pad->SetRightMargin(zeroMargin);
            pad->SetTopMargin(zeroMargin);
        }
    }

    // Find global min/max for y-axis
    double globalMin = std::numeric_limits<double>::max();
    double globalMax = -std::numeric_limits<double>::max();
    for (size_t binIndex = 0; binIndex < histMap[var].size(); ++binIndex) {
        TH1* h = histMap[var][binIndex];
        if (!h) continue;
        double min = h->GetMinimum();
        double max = h->GetMaximum();
        if (min < globalMin) globalMin = min;
        if (max > globalMax) globalMax = max;
    }

    // Set y-axis limits for all histograms and draw
    // Draw histograms in the (nCols-1)x(nRows-1) grid, skipping leftmost column and bottommost row
    double minPadX = std::numeric_limits<double>::max();
    double maxPadX = -std::numeric_limits<double>::max();
    double minPadY = std::numeric_limits<double>::max();
    double maxPadY = -std::numeric_limits<double>::max();

    for (size_t binIndex = 0; binIndex < histMap[var].size(); ++binIndex) {
        auto binKey = binKeysMap[var][binIndex];
        auto binPos = mainBinIndices.at(binKey);
        int col = binPos[0] + 1; // shift right by 1
        int row = binPos[1] + 1; // shift up by 1
        int flippedRow = (nRows - 1) - row;
        int padIndex = col + flippedRow*nCols + 1;
        c->cd(padIndex);
        ApplyHistStyle(histMap[var][binIndex]);
        //histMap[var][binIndex]->SetMinimum(globalMin);
        //histMap[var][binIndex]->SetMaximum(globalMax);
        histMap[var][binIndex]->Draw();
        DrawMeanTLatex(meanMap[binKey], axisLabels, 3, 0.15, 0.92);
        LOG_PRINT("Drawing bin " + binKey + " at " + std::to_string(padIndex));
        histMap[var][binIndex]->SetTitle("");
        double x_left  = gPad->GetXlowNDC();             // left edge of pad in canvas NDC
        double x_right = x_left + gPad->GetWNDC();       // right edge
        double y_low   = gPad->GetYlowNDC();             // bottom edge
        double y_high  = y_low + gPad->GetHNDC();        // top edge
        minPadX = std::min(minPadX, x_left);
        maxPadX = std::max(maxPadX, x_right);
        minPadY = std::min(minPadY, y_low);
        maxPadY = std::max(maxPadY, y_high);
    }
    c->Update();
    // Draw global canvas
    c->cd();
    // Draw a line with an arrow tip pointing right from minPadX to maxPadX at the bottom of the pads
    double arrowY = minPadY - 0.03; // slightly below the bottom edge
    TArrow* arrow = new TArrow(minPadX, arrowY, maxPadX, arrowY, 0.02, ">");
    arrow->SetLineWidth(3);
    arrow->SetLineColor(kBlack);
    arrow->SetFillColor(kBlack);
    arrow->Draw();
    // Label the arrow with 'X'
    TLatex* latex = new TLatex((minPadX + maxPadX) / 2.0, arrowY - 0.02, axisLabels[0].c_str());
    latex->SetTextAlign(22);
    latex->SetTextSize(0.05);
    latex->Draw();
    // Draw a vertical arrow for the Y axis at the leftmost pad edge
    double arrowX = minPadX - 0.03; // slightly left of the left edge
    TArrow* yarrow = new TArrow(arrowX, minPadY, arrowX, maxPadY, 0.02, ">");
    yarrow->SetLineWidth(3);
    yarrow->SetLineColor(kBlack);
    yarrow->SetFillColor(kBlack);
    yarrow->Draw();
    // Label the Y arrow
    TLatex* ylatex = new TLatex(arrowX - 0.02, (minPadY + maxPadY) / 2.0, axisLabels[1].c_str());
    ylatex->SetTextAlign(22);
    ylatex->SetTextSize(0.05);
    ylatex->Draw();
    c->SaveAs("playground/test.png");
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