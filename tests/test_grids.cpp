#include "Logger.h"
#include "Table.h"
#include <iostream>
#include <vector>

int main() {
    Logger::setLevel(Logger::Level::Debug);
    std::string tablePath = "tables/x_only/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt";
    std::string energyConfig = "5x41";
    Table table(tablePath);
    std::vector<std::string> binNames1 = {"X"};
    try {
        Grid grid1 = table.buildGrid(binNames1);
        LOG_INFO("--- Grid 1 (X, Q) ---");
        grid1.printGridSummary();
    } catch (const std::exception& e) {
        LOG_ERROR(e.what());
        return 1;
    }
    return 0;
}
