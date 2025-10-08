#ifndef TABLE_H
#define TABLE_H

#include "Grid.h"
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
    double AUT;
};

class Table {
public:
    Table();

    // energyConfig: "5x41", "10x100", "18x275"
    Table(const std::string& tablePath);

    // Access parsed rows
    const std::vector<TableRow>& getRows() const;

    Grid buildGrid(const std::vector<std::string>& binNames) const;

    // Fast lookup of AUT given X, Q, Z, and PhPerp
    double lookupAUT(double X, double Q, double Z, double PhPerp) const;

private:
    std::vector<TableRow> rows;
    void readTable(const std::string& filename);
    void createDefaultTable();
};

#endif // TABLE_H