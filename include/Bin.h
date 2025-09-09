#ifndef BIN_H
#define BIN_H
#include <string>
#include <vector>
#include <map>

class Bin {
public:
    Bin();
    Bin(double X_min, double X_max, double Q_min, double Q_max, double Z_min, double Z_max, double PhPerp_min, double PhPerp_max);
    void addSubBin(const Bin& subBin);
    void incrementCount();
    int getCount() const;
    double getMin(const std::string& var) const;
    double getMax(const std::string& var) const;
private:
    double X_min, X_max;
    double Q_min, Q_max;
    double Z_min, Z_max;
    double PhPerp_min, PhPerp_max;
    int count;
    std::vector<Bin> subBins;
};

#endif // BIN_H
