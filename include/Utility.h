#ifndef UTILITY_H
#define UTILITY_H

#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <string>

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

#endif // UTILITY_H
