#include "SidisBin.h"
#include <algorithm>

SidisBin::SidisBin()
    : Bin()
{}

SidisBin::SidisBin(double Xmin, double Xmax, double Qmin, double Qmax, double Zmin, double Zmax, double PhPerpmin, double PhPerpmax)
    : Bin(Xmin, Xmax, Qmin, Qmax, Zmin, Zmax, PhPerpmin, PhPerpmax)
{}

// For SIDIS the behavior is identical to the base Bin implementations, but we override
// so the derived class can be used polymorphically if needed.

double SidisBin::getMin(const std::string& var) const {
    return Bin::getMin(var);
}

double SidisBin::getMax(const std::string& var) const {
    return Bin::getMax(var);
}

void SidisBin::updateMin(const std::string& var, double value) {
    Bin::updateMin(var, value);
}

void SidisBin::updateMax(const std::string& var, double value) {
    Bin::updateMax(var, value);
}
