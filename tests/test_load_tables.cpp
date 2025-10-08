#include "Logger.h"
#include "Table.h"
#include <iostream>

void testLoadTables() {
    const std::vector<std::string> energyConfigs = {"5x41", "10x100", "18x275"};
    const std::vector<std::string> tables = {"tables/xQZPhPerp_v0/AUT_average_PV20_EPIC_piplus_sqrts=28.636.txt","tables/xQZPhPerp_v0/AUT_average_PV20_EPIC_piplus_sqrts=63.246.txt","tables/xQZPhPerp_v0/AUT_average_PV20_EPIC_piplus_sqrts=140.712.txt"};
    for (int i = 0; i < 3; i++) {
        std::string config = energyConfigs.at(i);
        std::string tablePath = tables.at(i);
        LOG_INFO(std::string("Testing energy configuration: ") + config);
        Table table(tablePath,config);
        const auto& rows = table.getRows();

        if (rows.empty()) {
            LOG_ERROR(std::string("Failed to load rows for configuration: ") + config);
        } else {
            LOG_INFO(std::string("Successfully loaded ") + std::to_string(rows.size()) + " rows for configuration: " + config);
        }
    }
}

int main() {
    testLoadTables();
    return 0;
}
