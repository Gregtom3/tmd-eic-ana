#ifndef UTILITY_H
#define UTILITY_H

#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <string>
#include <map>
#include "Constants.h"

namespace util {

inline void printProgress(size_t current, size_t total, int width = 50, const std::string& label = "") {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    if (total == 0)
        return;
    double ratio = static_cast<double>(current) / static_cast<double>(total);
    if (ratio < 0)
        ratio = 0;
    if (ratio > 1)
        ratio = 1;
    int pos = static_cast<int>(std::round(width * ratio));
    std::cout << "\r";
    if (!label.empty())
        std::cout << label << " ";
    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    int percent = static_cast<int>(ratio * 100.0);
    std::cout << "] " << percent << "% (" << current << "/" << total << ")" << std::flush;
    if (current >= total)
        std::cout << std::endl;
}

class ProgressBar {
public:
    ProgressBar(size_t total, int width = 50, const std::string& label = "")
        : total_(total)
        , width_(width)
        , label_(label)
        , start_(std::chrono::steady_clock::now())
        , finished_(false) {}

    // update with current count (0..total)
    void update(size_t current) {
        if (finished_)
            return;
        printProgress(current, total_, width_, label_);
    }

    // mark finished (prints final newline)
    void finish() {
        if (finished_)
            return;
        printProgress(total_, total_, width_, label_);
        finished_ = true;
    }

private:
    size_t total_;
    int width_;
    std::string label_;
    std::chrono::steady_clock::time_point start_;
    bool finished_;
};

} // namespace util

namespace util {

// Compute scale factor and log mc_lumi, exp_lumi, and scale.
// totalEvents: number of MC events (integer)
// xsTotal: total cross section value (same units as expected by formula)
// energyConfig: key to look up integrated luminosity in IntegratedLuminosities
// out_mc and out_exp are set to the computed mc and expected luminosities (nb^-1)
inline double computeScale(long long totalEvents, double xsTotal, const std::string& energyConfig,
                           double& out_mc, double& out_exp) {
    // Find experimental integrated luminosity L
    double L = 0.0;
    auto it = IntegratedLuminosities.find(energyConfig);
    if (it == IntegratedLuminosities.end()) {
        std::cerr << "computeScale: unknown energyConfig '" << energyConfig << "' - cannot compute scale.\n";
        std::cerr << "Available configurations:";
        for (const auto& kv : IntegratedLuminosities) {
            std::cerr << " " << kv.first;
        }
        std::cerr << std::endl;
        std::cerr << "Returning scale=1.0" << std::endl;

        out_mc = 1.0;
        out_exp = 1.0;
        return 1.0;
    }
    L = it->second;

    // mc_lumi = TotalEvents / (XsTotal / 1e3)
    // XsTotal is divided by 1e3 per spec (unit conversion); result is in nb^-1
    double mc_lumi = 0.0;
    if (xsTotal == 0.0) {
        std::cerr << "computeScale: XsTotal is zero - cannot compute mc_lumi." << std::endl;
        out_mc = 0.0;
        out_exp = 0.0;
        return 1.0;
    }
    mc_lumi = static_cast<double>(totalEvents) / (xsTotal / 1e3);

    // exp_lumi = L * 1e6 (to convert from fb^-1 to nb^-1)
    double exp_lumi = L  * 1e6;

    double scale = exp_lumi / mc_lumi;

    std::cout << "computeScale: mc_lumi = " << mc_lumi << " nb^-1, exp_lumi = " << exp_lumi
              << " nb^-1, scale = " << scale << std::endl;

    out_mc = mc_lumi;
    out_exp = exp_lumi;
    return scale;
}

} // namespace util

#endif // UTILITY_H
