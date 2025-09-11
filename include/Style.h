#ifndef HISTSTYLE_H
#define HISTSTYLE_H
#include "TH1.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TColor.h"
#include <memory>
#include <map>
#include <vector>

inline void ApplyHistStyle(TH1* h) {
    if (!h) return;
    // Fill color: light blue
    int fillColor = TColor::GetColor(173, 216, 230); // RGB for light blue
    h->SetFillColor(fillColor);
    h->SetLineColor(kBlack);
    h->SetLineWidth(3); // thick border
    h->SetMarkerStyle(20);
    h->SetMarkerColor(kBlack);
    h->SetMarkerSize(1.2);
    h->SetStats(0); // disables stats box for this hist
    h->GetXaxis()->SetLabelSize(0.05); // larger axis labels
    h->GetYaxis()->SetLabelSize(0.05);
    h->GetXaxis()->SetTitleSize(0.06);
    h->GetYaxis()->SetTitleSize(0.06);
}

inline void ApplyGlobalStyle() {
    // Disable global stats box
    gStyle->SetOptStat(0);
    gStyle->SetTitleFontSize(0.06);
    gStyle->SetLabelSize(0.05, "X");
    gStyle->SetLabelSize(0.05, "Y");
    gStyle->SetTitleSize(0.06, "X");
    gStyle->SetTitleSize(0.06, "Y");
}

inline void DrawMeanTLatex(const std::unordered_map<std::string, double>& meanMap,
                           const std::vector<std::string>& vars,
                           int precision = 3,
                           double x_ndc = 0.15,
                           double y_ndc = 0.92) {
    TLatex latex;
    latex.SetTextSize(0.045);
    latex.SetTextColor(kBlack);
    latex.SetTextFont(42);
    std::string label;
    char buf[64];
    for (size_t i = 0; i < vars.size(); ++i) {
        auto it = meanMap.find(vars[i]);
        double val = (it != meanMap.end()) ? it->second : 0.0;
        snprintf(buf, sizeof(buf), "<%s>=%.*f", vars[i].c_str(), precision, val);
        label += buf;
        if (i + 1 < vars.size()) label += "  ";
    }
    latex.DrawLatexNDC(x_ndc, y_ndc, label.c_str());
}

#endif // HISTSTYLE_H
