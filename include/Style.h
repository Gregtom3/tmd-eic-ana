#ifndef HISTSTYLE_H
#define HISTSTYLE_H
#include "TColor.h"
#include "TH1.h"
#include "TLatex.h"
#include "TStyle.h"
#include <map>
#include <memory>
#include <vector>

inline void ApplyHistStyle(TH1* h, float y_axis_max_factor = 1.1) {
    if (!h)
        return;
    // Fill color: light blue
    int fillColor = TColor::GetColor(173, 216, 230); // RGB for light blue
    h->SetFillColor(fillColor);
    h->SetLineColor(kBlack);
    h->SetLineWidth(1);
    h->SetMarkerStyle(20);
    h->SetMarkerColor(kBlack);
    h->SetMarkerSize(1.2); 
    h->SetStats(0);                    // disables stats box for this hist
    h->SetTitle("");                  // disable histogram title (handled by style)
    h->GetXaxis()->SetLabelSize(0.05); // larger axis labels
    h->GetYaxis()->SetLabelSize(0.05);
    h->GetXaxis()->SetTitleSize(0.06);
    h->GetYaxis()->SetTitleSize(0.06);
    h->GetYaxis()->SetRangeUser(1, h->GetMaximum() * y_axis_max_factor); // Slightly increase Y axis range for better visuals
    // Make sure the Y title has room
    h->GetYaxis()->SetTitleOffset(1.2);
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

inline void DrawMeanTLatex(const std::unordered_map<std::string, double>& meanMap, const std::vector<std::string>& vars,
                           int precision = 3, double x_ndc = 0.15, double y_ndc = 0.92, float textSize = 0.045) {
    TLatex latex;
    latex.SetTextSize(textSize);
    latex.SetTextColor(kBlack);
    latex.SetTextFont(42);
    std::string label;
    char buf[64];
    for (size_t i = 0; i < vars.size(); ++i) {
        auto it = meanMap.find(vars[i]);
        double val = (it != meanMap.end()) ? it->second : 0.0;
        if (std::fabs(val) >= 1e4 || (std::fabs(val) > 0 && std::fabs(val) < 1e-3)) {
            snprintf(buf, sizeof(buf), "<%s>=%.*e", vars[i].c_str(), precision, val);
        } else {
            snprintf(buf, sizeof(buf), "<%s>=%.*f", vars[i].c_str(), precision, val);
        }
        label += buf;
        if (i + 1 < vars.size())
            label += "  ";
    }
    latex.DrawLatexNDC(x_ndc, y_ndc, label.c_str());
}

#endif // HISTSTYLE_H
