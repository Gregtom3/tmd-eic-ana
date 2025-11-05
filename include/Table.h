#ifndef TABLE_H
#define TABLE_H

#include "Grid.h"
#include "Constants.h"
#include <set>
#include <string>
#include <vector>

struct TableRow {
    int itar;
    int ihad;
    double X_min, X_max;
    double Q_min, Q_max;
    double Z_min, Z_max;
    double PhPerp_min, PhPerp_max;
    double Mh_min, Mh_max;
    double AUT;
};

class Table {
public:
    Table(channel_type channel);
    Table(channel_type channel, const std::string& tablePath);
    // Convenience ctor for legacy callers that only provide a table path (assume Hadron channel)
    Table(const std::string& tablePath);

    // Access parsed rows
    const std::vector<TableRow>& getRows() const;

    Grid buildGrid(const std::vector<std::string>& binNames) const;

    // Fast lookup of AUT given X, Q, Z, and PhPerp
    double lookupAUT_SIDIS(double X, double Q, double Z, double PhPerp) const;
    double lookupAUT_DISIDIS(double X, double Q, double Z, double Mh) const;

private:
    channel_type channel;
    std::vector<TableRow> rows;
    void readTable(const std::string& filename);
    void createDefaultTable();
};

#endif // TABLE_H