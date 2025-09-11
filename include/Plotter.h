#ifndef PLOTTER_H
#define PLOTTER_H

#include "Hist.h"
#include "Grid.h"
#include <string>
#include <vector>
#include <map>

class Plotter {
public:
    Plotter() = default;
    void plot1DBin(const std::string& var, const Hist* hist, size_t binIndex, const std::string& outpath = "");
    void plot2DMap(const std::string& var, const Hist* hist, const Grid* grid, const std::string& outpath = "plot.png");

private:
    void setupCanvas(int nCols, int nRows);
};

#endif // PLOTTER_H
