#include "Logger.h"
#include "TMD.h"
#include <filesystem>
#include <iostream>

int main() {
    const std::string artifactDir = "artifacts/plots_2D";
    // Ensure the artifacts directory exists
    std::filesystem::create_directories(artifactDir);

    const std::string outpath = "out/output.root";
    TMD tmd(outpath, "tree");
    if (!tmd.isLoaded()) {
        LOG_ERROR("TMD failed to load generated tree file");
        return 1;
    }

    LOG_INFO("TMD loaded generated file successfully");

    tmd.loadTable("0x0");
    tmd.buildGrid({"X", "Q"});
    LOG_INFO("Grid built successfully");

    tmd.fillHistograms("PhPerp", artifactDir, true);
    tmd.plot2DMap("PhPerp", artifactDir + "/PhPerp_2D_X_Q_10x100.png");

    tmd.fillHistograms("Z", artifactDir, true);
    tmd.plot2DMap("Z", artifactDir + "/Z_2D_X_Q_10x100.png");

    tmd.fillHistograms("PhiH", artifactDir, true);
    tmd.plot2DMap("PhiH", artifactDir + "/PhiH_2D_X_Q_10x100.png");

    LOG_INFO("2D plots generated successfully");
    return 0;
}