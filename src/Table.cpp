#include "Table.h"
#include <fstream>
#include <sstream>
#include <iostream>

Table::Table(const std::string& energyConfig) {
    std::string filename = getFilename(energyConfig);
    readTable(filename);
}

std::string Table::getFilename(const std::string& energyConfig) const {
    if (energyConfig == "5x41")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt";
    if (energyConfig == "10x100")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=63.246.txt";
    if (energyConfig == "18x275")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=140.712.txt";
    std::cerr << "Unknown energy configuration: " << energyConfig << std::endl;
    return "";
}

void Table::readTable(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open table file: " << filename << std::endl;
        return;
    }
    std::string line;
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        TableRow row;
        char comma;
        iss >> row.itar >> comma
            >> row.ihad >> comma
            >> row.X_min >> comma
            >> row.X_max >> comma
            >> row.Q_min >> comma
            >> row.Q_max >> comma
            >> row.Z_min >> comma
            >> row.Z_max >> comma
            >> row.PhPerp_min >> comma
            >> row.PhPerp_max >> comma
            >> row.AUT;
        if (iss)
            rows.push_back(row);
    }
}

const std::vector<TableRow>& Table::getRows() const {
    return rows;
}