#include "Table.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

Table::Table() {
    createDefaultTable();
}

Table::Table(const std::string& energyConfig) {
    std::string filename = getFilename(energyConfig);
    readTable(filename);
}

std::string Table::getFilename(const std::string& energyConfig) const {
    if (energyConfig == "0x0")
        return "tables/AUT_0x0_XQZPhPerp.txt"; // Debug, to remove eventually
    if (energyConfig == "5x41")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt";
    if (energyConfig == "10x100")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=63.246.txt";
    if (energyConfig == "18x275")
        return "tables/AUT_average_PV20_EPIC_piplus_sqrts=140.712.txt";
    LOG_ERROR(std::string("Unknown energy configuration: ") + energyConfig);
    return "";
}

void Table::createDefaultTable() {
    // Use lower and upper bound from compiler
    TableRow row{};
    row.itar = 1;
    row.ihad = 1;
    row.X_min = -999999;
    row.X_max = 999999;
    row.Q_min = -999999;
    row.Q_max = 999999;
    row.Z_min = -999999;
    row.Z_max = 999999;
    row.PhPerp_min = -999999;
    row.PhPerp_max = 999999;
    row.AUT = 0.0;
    rows.push_back(row);
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
        if (line.empty())
            continue;

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
            row.itar = std::stoi(fields[0]);
            row.ihad = std::stoi(fields[1]);
            row.X_min = std::stod(fields[2]);
            row.X_max = std::stod(fields[3]);
            row.Q_min = std::stod(fields[4]);
            row.Q_max = std::stod(fields[5]);
            row.Z_min = std::stod(fields[6]);
            row.Z_max = std::stod(fields[7]);
            row.PhPerp_min = std::stod(fields[8]);
            row.PhPerp_max = std::stod(fields[9]);
            row.AUT = std::stod(fields[10]);
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
    std::vector<std::string> allBinNames = {"X", "Q", "Z", "PhPerp"};
    for (const auto& row : rows) {
        std::map<std::string, std::pair<double, double>> binRanges;
        for (const auto& name : allBinNames) {
            if (name == "X")
                binRanges["X"] = {row.X_min, row.X_max};
            else if (name == "Q")
                binRanges["Q"] = {row.Q_min, row.Q_max};
            else if (name == "Z")
                binRanges["Z"] = {row.Z_min, row.Z_max};
            else if (name == "PhPerp")
                binRanges["PhPerp"] = {row.PhPerp_min, row.PhPerp_max};
        }
        grid.addBin(binRanges);
    }
    grid.computeMainBinIndices();
    return grid;
}

double Table::lookupAUT(double X, double Q, double Z, double PhPerp) const {
    std::function<double(const std::vector<TableRow>&, double, double, double, double, size_t, size_t)> binarySearch =
        [&](const std::vector<TableRow>& rows, double X, double Q, double Z, double PhPerp, size_t left, size_t right) -> double {
        if (left > right) {
            return 0.0; // Base case: no match found
        }

        size_t mid = left + (right - left) / 2;
        const auto& row = rows[mid];

        if (X >= row.X_min && X <= row.X_max && Q >= row.Q_min && Q <= row.Q_max && Z >= row.Z_min && Z <= row.Z_max &&
            PhPerp >= row.PhPerp_min && PhPerp <= row.PhPerp_max) {
            return row.AUT; // Match found
        }

        if (X < row.X_min || (X == row.X_min && Q < row.Q_min) || (X == row.X_min && Q == row.Q_min && Z < row.Z_min) ||
            (X == row.X_min && Q == row.Q_min && Z == row.Z_min && PhPerp < row.PhPerp_min)) {
            return binarySearch(rows, X, Q, Z, PhPerp, left, mid - 1);
        } else {
            return binarySearch(rows, X, Q, Z, PhPerp, mid + 1, right);
        }
    };

    return binarySearch(rows, X, Q, Z, PhPerp, 0, rows.size() - 1);
}