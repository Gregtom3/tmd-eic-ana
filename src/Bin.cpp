#include "Bin.h"

Bin::Bin()
    : X_min(10000.0)
    , X_max(-10000.0)
    , Q_min(10000.0)
    , Q_max(-10000.0)
    , Z_min(10000.0)
    , Z_max(-10000.0)
    , PhPerp_min(10000.0)
    , PhPerp_max(-10000.0)
    , count(0)
    , events(0) {}

Bin::Bin(double Xmin, double Xmax, double Qmin, double Qmax, double Zmin, double Zmax, double PhPerpmin, double PhPerpmax)
    : X_min(Xmin)
    , X_max(Xmax)
    , Q_min(Qmin)
    , Q_max(Qmax)
    , Z_min(Zmin)
    , Z_max(Zmax)
    , PhPerp_min(PhPerpmin)
    , PhPerp_max(PhPerpmax)
    , count(0)
    , events(0) {}

void Bin::incrementCount() {
    ++count;
}


int Bin::getCount() const {
    return count;
}

int Bin::getEvents() const {
    return events;
}

void Bin::setEvents(int _events) const {
    events = _events;
}

void Bin::updateMin(const std::string& var, double value) {
    if (var == "X")
        X_min = std::min(X_min, value);
    else if (var == "Q")
        Q_min = std::min(Q_min, value);
    else if (var == "Z")
        Z_min = std::min(Z_min, value);
    else if (var == "PhPerp")
        PhPerp_min = std::min(PhPerp_min, value);
}

void Bin::updateMax(const std::string& var, double value) {
    if (var == "X")
        X_max = std::max(X_max, value);
    else if (var == "Q")
        Q_max = std::max(Q_max, value);
    else if (var == "Z")
        Z_max = std::max(Z_max, value);
    else if (var == "PhPerp")
        PhPerp_max = std::max(PhPerp_max, value);
}

double Bin::getMin(const std::string& var) const {
    if (var == "X")
        return X_min;
    if (var == "Q")
        return Q_min;
    if (var == "Z")
        return Z_min;
    if (var == "PhPerp")
        return PhPerp_min;
    return 0;
}

double Bin::getMax(const std::string& var) const {
    if (var == "X")
        return X_max;
    if (var == "Q")
        return Q_max;
    if (var == "Z")
        return Z_max;
    if (var == "PhPerp")
        return PhPerp_max;
    return 0;
}
