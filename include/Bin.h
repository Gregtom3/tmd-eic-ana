#ifndef BIN_H
#define BIN_H
#include <map>
#include <string>
#include <vector>

class Bin {
public:
    Bin();
    Bin(double X_min, double X_max, double Q_min, double Q_max, double Z_min, double Z_max, double PhPerp_min, double PhPerp_max);
    void incrementCount();
    int getCount() const;
    int getEvents() const;
    void setEvents(int _events) const;
    int getExpectedEvents() const;
    void setExpectedEvents(int _expected_events) const;
    double getMin(const std::string& var) const;
    double getMax(const std::string& var) const;
    void updateMin(const std::string& var, double value);
    void updateMax(const std::string& var, double value);

private:
    double X_min, X_max;
    double Q_min, Q_max;
    double Z_min, Z_max;
    double PhPerp_min, PhPerp_max;
    int count;  // Number of sub-bins within this bin
    mutable int events; // Number of events in this bin 
    mutable int expected_events; // Expected number of events in this bin
};

#endif // BIN_H
