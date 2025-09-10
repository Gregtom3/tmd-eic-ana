#include "Plotter.h"
#include "Style.h"
#include "Logger.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TArrow.h"
#include "TLatex.h"
#include <limits>

void Plotter::plot2DMap(const std::string& var, const Hist* hist, const Grid* grid, const std::string& outpath) {
    if (!hist || !grid) {
        LOG_ERROR("Hist or Grid is null in Plotter::plot2DMap");
        return;
    }

    const auto& histMap = hist->getHistMap();
    if (histMap.find(var) == histMap.end()) {
        LOG_ERROR("Variable " + var + " not found in histMap.");
        return;
    }
    const auto& hists = histMap.at(var);
    const auto& binKeysMap = hist->getBinKeysMap();
    const auto& binKeys = binKeysMap.at(var);
    const auto& mainBinIndices = grid->getMainBinIndices();
    auto axisLabels = grid->getMainBinNames();

    int maxRow = 0;
    int maxCol = 0;
    for (const auto& pair : mainBinIndices) {
        maxCol = std::max(maxCol, pair.second.at(0));
        maxRow = std::max(maxRow, pair.second.at(1));
    }
    int nCols = maxCol + 2;
    int nRows = maxRow + 2;
    int windowX = 400;
    int windowY = 400;
    TCanvas* c = new TCanvas("c2d", "2D Map", windowX * nCols, windowY * nRows);
    c->Divide(nCols, nRows, 0, 0);
    ApplyGlobalStyle();

    double leftMargin = 0.15, bottomMargin = 0.15, zeroMargin = 0.0;
    for (int row = 0; row < nRows; ++row) {
        for (int col = 0; col < nCols; ++col) {
            int flippedRow = (nRows - 1) - row;
            int padIndex = col + flippedRow * nCols + 1;
            c->cd(padIndex);
            TPad* pad = (TPad*)c->GetPad(padIndex);
            pad->SetLeftMargin((col == 1) ? leftMargin : zeroMargin);
            pad->SetBottomMargin((row == 1) ? bottomMargin : zeroMargin);
            pad->SetRightMargin(zeroMargin);
            pad->SetTopMargin(zeroMargin);
        }
    }

    double minPadX = std::numeric_limits<double>::max(), maxPadX = -std::numeric_limits<double>::max();
    double minPadY = std::numeric_limits<double>::max(), maxPadY = -std::numeric_limits<double>::max();

    const auto& meanMap = hist->getMeans();

    for (size_t binIndex = 0; binIndex < hists.size(); ++binIndex) {
        auto binKey = binKeys[binIndex];
        auto binPos = mainBinIndices.at(binKey);
        int col = binPos[0] + 1;
        int row = binPos[1] + 1;
        int flippedRow = (nRows - 1) - row;
        int padIndex = col + flippedRow * nCols + 1;
        c->cd(padIndex);
        ApplyHistStyle(hists[binIndex]);
        if (hists[binIndex]->GetEntries() > 10) {
            hists[binIndex]->Draw();
            if (meanMap.count(binKey)) {
                DrawMeanTLatex(meanMap.at(binKey), grid->getMainBinNames(), 6, 0.15, 0.92);
            }
        } else {
            continue;
        }
        hists[binIndex]->SetTitle("");
        double x_left = gPad->GetXlowNDC(), x_right = x_left + gPad->GetWNDC();
        double y_low = gPad->GetYlowNDC(), y_high = y_low + gPad->GetHNDC();
        minPadX = std::min(minPadX, x_left);
        maxPadX = std::max(maxPadX, x_right);
        minPadY = std::min(minPadY, y_low);
        maxPadY = std::max(maxPadY, y_high);
    }
    c->Update();
    c->cd();

    double arrowY = minPadY - 0.03;
    TArrow* arrow = new TArrow(minPadX, arrowY, maxPadX, arrowY, 0.02, ">");
    arrow->SetLineWidth(3);
    arrow->SetLineColor(kBlack);
    arrow->SetFillColor(kBlack);
    arrow->Draw();

    TLatex* latex = new TLatex((minPadX + maxPadX) / 2.0, arrowY - 0.02, axisLabels[0].c_str());
    latex->SetTextAlign(22);
    latex->SetTextSize(0.05);
    latex->Draw();

    double arrowX = minPadX - 0.03;
    TArrow* yarrow = new TArrow(arrowX, minPadY, arrowX, maxPadY, 0.02, ">");
    yarrow->SetLineWidth(3);
    yarrow->SetLineColor(kBlack);
    yarrow->SetFillColor(kBlack);
    yarrow->Draw();

    TLatex* ylatex = new TLatex(arrowX - 0.02, (minPadY + maxPadY) / 2.0, axisLabels[1].c_str());
    ylatex->SetTextAlign(22);
    ylatex->SetTextSize(0.05);
    ylatex->SetTextAngle(90);
    ylatex->Draw();

    c->SaveAs(outpath.c_str());
    delete c;
}
