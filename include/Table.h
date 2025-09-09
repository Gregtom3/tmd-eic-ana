#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <vector>
#include <set>
#include "Grid.h"

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
    // energyConfig: "5x41", "10x100", "18x275"
    Table(const std::string& energyConfig);

    // Access parsed rows
    const std::vector<TableRow>& getRows() const;

    Grid buildGrid(const std::vector<std::string>& binNames) const;

private:
    std::vector<TableRow> rows;
    std::string getFilename(const std::string& energyConfig) const;
    void readTable(const std::string& filename);
};

#endif // TABLE_H