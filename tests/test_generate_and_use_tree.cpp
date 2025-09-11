#include "Logger.h"
#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>

// This test generates a small ROOT file with the same structure as playground/generate_tree.C
// and then attempts to load it with TMD and exercise basic functionality.

int main() {

    const std::string outpath = "tests/output_generated.root";

    // Create a small toy tree
    TFile fout(outpath.c_str(), "RECREATE");
    TTree tree("tree", "Toy Tree");

    float X, Q2, Z, PhPerp, PhiH, PhiS;
    float Weight = 1.0f;
    float TrueX, TrueQ2, TrueZ, TruePhPerp, TruePhiH, TruePhiS;
    int Spin_idx;

    tree.Branch("X", &X, "X/F");
    tree.Branch("Q2", &Q2, "Q2/F");
    tree.Branch("Z", &Z, "Z/F");
    tree.Branch("PhPerp", &PhPerp, "PhPerp/F");
    tree.Branch("PhiH", &PhiH, "PhiH/F");
    tree.Branch("PhiS", &PhiS, "PhiS/F");

    tree.Branch("TrueX", &TrueX, "TrueX/F");
    tree.Branch("TrueQ2", &TrueQ2, "TrueQ2/F");
    tree.Branch("TrueZ", &TrueZ, "TrueZ/F");
    tree.Branch("TruePhPerp", &TruePhPerp, "TruePhPerp/F");
    tree.Branch("TruePhiH", &TruePhiH, "TruePhiH/F");
    tree.Branch("TruePhiS", &TruePhiS, "TruePhiS/F");
    tree.Branch("Weight", &Weight, "Weight/F");
    tree.Branch("Spin_idx", &Spin_idx, "Spin_idx/I");

    TRandom3 rng(123);
    const int N = 2000; // small for fast test
    for (int i = 0; i < N; ++i) {
        TrueX = rng.Uniform(0.0, 1.0);
        TrueQ2 = rng.Uniform(1.0, 10.0);
        TrueZ = rng.Uniform(0.0, 1.0);
        TruePhPerp = rng.Uniform(0.0, 5.0);
        TruePhiH = rng.Uniform(-TMath::Pi(), TMath::Pi());
        TruePhiS = rng.Uniform(-TMath::Pi(), TMath::Pi());

        X = TrueX + rng.Gaus(0, 0.01);
        Q2 = TrueQ2 + rng.Gaus(0, 0.1);
        Z = TrueZ + rng.Gaus(0, 0.01);
        PhPerp = TruePhPerp + rng.Gaus(0, 0.01);
        PhiH = TruePhiH + rng.Gaus(0, 0.01);
        PhiS = TruePhiS + rng.Gaus(0, 0.01);

        Weight = 1.0f;
        Spin_idx = 0;
        tree.Fill();
    }

    tree.Write();

    std::vector<double> XsTotal; XsTotal.push_back(1.0);
    std::vector<int> TotalEvents; TotalEvents.push_back(N);
    fout.WriteObject(&XsTotal, "XsTotal");
    fout.WriteObject(&TotalEvents, "TotalEvents");
    fout.Close();

    // Now attempt to use TMD on this file
    TMD tmd(outpath, "tree");
    if (!tmd.isLoaded()) {
        LOG_ERROR("TMD failed to load generated tree file");
        return 1;
    }

    LOG_INFO("TMD loaded generated file successfully");

    // Load table (default) and build a simple grid
    tmd.loadTable();
    tmd.buildGrid({"X"});
    std::cout << "Grid summary:" << std::endl;
    tmd.getGrid()->printGridSummary();

    // Fill histograms for variable X into a temporary directory
    tmd.fillHistograms("X", "tests/out", true);
    LOG_INFO("fillHistograms completed");

    // Run a simple inject/extract on bin 0
    tmd.inject_extract(0, 0.2);
    LOG_INFO("inject_extract completed");

    std::cout << "Test passed." << std::endl;
    return 0;
}
