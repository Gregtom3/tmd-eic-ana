#include "Table.h"
#include "Logger.h"
#include <iostream>

void testLoadTables() {
    const std::vector<std::string> energyConfigs = {"5x41", "10x100", "18x275"};

    for (const auto& config : energyConfigs) {
        LOG_INFO(std::string("Testing energy configuration: ") + config);
        Table table(config);
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
