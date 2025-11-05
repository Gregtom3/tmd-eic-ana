#ifndef DISIDISBIN_H
#define DISIDISBIN_H

#include "Bin.h"

class DiSidisBin : public Bin {
public:
    DiSidisBin();
    // Note: last pair interpreted as Mh for di-hadron bins
    DiSidisBin(double X_min, double X_max, double Q_min, double Q_max, double Z_min, double Z_max, double Mh_min, double Mh_max);

    // Override to use Mh instead of PhPerp
    double getMin(const std::string& var) const override;
    double getMax(const std::string& var) const override;
    void updateMin(const std::string& var, double value) override;
    void updateMax(const std::string& var, double value) override;

private:
    double Mh_min{10000.0}, Mh_max{-10000.0};
};

#endif // DISIDISBIN_H
