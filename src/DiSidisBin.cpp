#include "DiSidisBin.h"
#include <algorithm>

DiSidisBin::DiSidisBin()
    : Bin()
{}

DiSidisBin::DiSidisBin(double Xmin, double Xmax, double Qmin, double Qmax, double Zmin, double Zmax, double Mhmin, double Mhmax)
    : Bin(Xmin, Xmax, Qmin, Qmax, Zmin, Zmax, 0.0, 0.0)
{
    Mh_min = Mhmin;
    Mh_max = Mhmax;
}

// Override to use Mh instead of PhPerp
void DiSidisBin::updateMin(const std::string& var, double value) {
    if (var == "X")
        Bin::updateMin(var, value);
    else if (var == "Q")
        Bin::updateMin(var, value);
    else if (var == "Z")
        Bin::updateMin(var, value);
    else if (var == "Mh")
        Mh_min = std::min(Mh_min, value);
}

void DiSidisBin::updateMax(const std::string& var, double value) {
    if (var == "X")
        Bin::updateMax(var, value);
    else if (var == "Q")
        Bin::updateMax(var, value);
    else if (var == "Z")
        Bin::updateMax(var, value);
    else if (var == "Mh")
        Mh_max = std::max(Mh_max, value);
}

double DiSidisBin::getMin(const std::string& var) const {
    if (var == "X")
        return Bin::getMin(var);
    if (var == "Q")
        return Bin::getMin(var);
    if (var == "Z")
        return Bin::getMin(var);
    if (var == "Mh")
        return Mh_min;
    return 0;
}

double DiSidisBin::getMax(const std::string& var) const {
    if (var == "X")
        return Bin::getMax(var);
    if (var == "Q")
        return Bin::getMax(var);
    if (var == "Z")
        return Bin::getMax(var);
    if (var == "Mh")
        return Mh_max;
    return 0;
}
