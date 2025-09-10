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
    void plot2DMap(const std::string& var, const Hist* hist, const Grid* grid, const std::string& outpath = "plot.png");

private:
    void setupCanvas(int nCols, int nRows);
};

#endif // PLOTTER_H
