#ifndef SIDISBIN_H
#define SIDISBIN_H

#include "Bin.h"

class SidisBin : public Bin {
public:
    SidisBin();
    SidisBin(double X_min, double X_max, double Q_min, double Q_max, double Z_min, double Z_max, double PhPerp_min, double PhPerp_max);

    // Override the virtual functions if needed; for SIDIS they behave like the base class
    double getMin(const std::string& var) const override;
    double getMax(const std::string& var) const override;
    void updateMin(const std::string& var, double value) override;
    void updateMax(const std::string& var, double value) override;
};

#endif // SIDISBIN_H
