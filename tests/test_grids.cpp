#include "Logger.h"
#include "Table.h"
#include <iostream>
#include <vector>

int main() {
    Table table("tables/xQZPhPerp_v0/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt","5x41");
    std::vector<std::string> binNames1 = {"X", "Q"};
    std::vector<std::string> binNames2 = {"Z", "PhPerp"};
    try {
        Grid grid1 = table.buildGrid(binNames1);
        LOG_INFO("--- Grid 1 (X, Q) ---");
        grid1.printGridSummary();
        // Grid grid2 = table.buildGrid(binNames2);
        // LOG_INFO("--- Grid 2 (Z, PhPerp) ---");
        // grid2.printGridSummary();
    } catch (const std::exception& e) {
        LOG_ERROR(e.what());
        return 1;
    }
    return 0;
}
