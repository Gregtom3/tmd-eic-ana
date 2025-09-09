#include "../include/Table.h"
#include <iostream>

void testLoadTables() {
    const std::vector<std::string> energyConfigs = {"5x41", "10x100", "18x275"};

    for (const auto& config : energyConfigs) {
        std::cout << "Testing energy configuration: " << config << std::endl;
        Table table(config);
        const auto& rows = table.getRows();

        if (rows.empty()) {
            std::cerr << "Failed to load rows for configuration: " << config << std::endl;
        } else {
            std::cout << "Successfully loaded " << rows.size() << " rows for configuration: " << config << std::endl;
        }
    }
}

int main() {
    testLoadTables();
    return 0;
}
