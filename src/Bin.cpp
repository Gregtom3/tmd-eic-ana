#include "../include/Bin.h"

Bin::Bin() : X_min(0), X_max(0), Q_min(0), Q_max(0), Z_min(0), Z_max(0), PhPerp_min(0), PhPerp_max(0), count(0) {}

Bin::Bin(double Xmin, double Xmax, double Qmin, double Qmax, double Zmin, double Zmax, double PhPerpmin, double PhPerpmax)
    : X_min(Xmin), X_max(Xmax), Q_min(Qmin), Q_max(Qmax), Z_min(Zmin), Z_max(Zmax), PhPerp_min(PhPerpmin), PhPerp_max(PhPerpmax), count(0) {}

void Bin::addSubBin(const Bin& subBin) {
    subBins.push_back(subBin);
}

void Bin::incrementCount() {
    ++count;
}

int Bin::getCount() const {
    return count;
}

double Bin::getMin(const std::string& var) const {
    if (var == "X") return X_min;
    if (var == "Q") return Q_min;
    if (var == "Z") return Z_min;
    if (var == "PhPerp") return PhPerp_min;
    return 0;
}

double Bin::getMax(const std::string& var) const {
    if (var == "X") return X_max;
    if (var == "Q") return Q_max;
    if (var == "Z") return Z_max;
    if (var == "PhPerp") return PhPerp_max;
    return 0;
}
