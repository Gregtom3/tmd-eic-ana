#include "../include/Table.h"
#include <iostream>
#include <vector>

int main() {
    Table table("5x41");
    std::vector<std::string> binNames1 = {"X", "Q"};
    std::vector<std::string> binNames2 = {"Z", "PhPerp"};
    try {
        Grid grid1 = table.buildGrid(binNames1);
        std::cout << "--- Grid 1 (X, Q) ---" << std::endl;
        grid1.printGridSummary();
        Grid grid2 = table.buildGrid(binNames2);
        std::cout << "--- Grid 2 (Z, PhPerp) ---" << std::endl;
        grid2.printGridSummary();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
