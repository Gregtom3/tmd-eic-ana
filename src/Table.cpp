#include "Table.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <stdexcept>

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
    LOG_ERROR(std::string("Unknown energy configuration: ") + energyConfig);
    return "";
}

void Table::readTable(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_ERROR(std::string("Failed to open table file: ") + filename);
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        TableRow row{};
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;

        // Split on commas
        while (std::getline(ss, field, ',')) {
            // trim whitespace
            field.erase(0, field.find_first_not_of(" \t"));
            field.erase(field.find_last_not_of(" \t") + 1);
            if (!field.empty())
                fields.push_back(field);
        }

        if (fields.size() != 11) {
            LOG_WARN(std::string("Skipping malformed row (") + std::to_string(fields.size()) + ") in table: " + line);
            continue;
        }

        try {
            row.itar        = std::stoi(fields[0]);
            row.ihad        = std::stoi(fields[1]);
            row.X_min       = std::stod(fields[2]);
            row.X_max       = std::stod(fields[3]);
            row.Q_min       = std::stod(fields[4]);
            row.Q_max       = std::stod(fields[5]);
            row.Z_min       = std::stod(fields[6]);
            row.Z_max       = std::stod(fields[7]);
            row.PhPerp_min  = std::stod(fields[8]);
            row.PhPerp_max  = std::stod(fields[9]);
            row.AUT         = std::stod(fields[10]);
        } catch (const std::exception& e) {
            LOG_ERROR(std::string("Conversion error: ") + e.what() + " in line: " + line);
            continue;
        }

        rows.push_back(row);
    }
}



const std::vector<TableRow>& Table::getRows() const {
    return rows;
}

Grid Table::buildGrid(const std::vector<std::string>& binNames) const {
    // Validate bin names
    std::set<std::string> allowed = {"X", "Q", "Z", "PhPerp"};
    std::set<std::string> unique(binNames.begin(), binNames.end());
    if (unique.size() != binNames.size()) {
        throw std::invalid_argument("Duplicate bin names detected");
    }
    for (const auto& name : binNames) {
        if (allowed.find(name) == allowed.end()) {
            throw std::invalid_argument("Invalid bin name: " + name);
        }
    }
    Grid grid(binNames);
    std::vector<std::string> allBinNames =  {"X", "Q", "Z", "PhPerp"};
    for (const auto& row : rows) {
        std::map<std::string, std::pair<double, double>> binRanges;
        for (const auto& name : allBinNames) {
            if (name == "X") binRanges["X"] = {row.X_min, row.X_max};
            else if (name == "Q") binRanges["Q"] = {row.Q_min, row.Q_max};
            else if (name == "Z") binRanges["Z"] = {row.Z_min, row.Z_max};
            else if (name == "PhPerp") binRanges["PhPerp"] = {row.PhPerp_min, row.PhPerp_max};
        }
        grid.addBin(binRanges);
    }
    return grid;
}