#include "TMD.h"
#include <TFile.h>
#include <TTree.h>
#include <TRandom3.h>
#include <TMath.h>
#include <vector>
#include <iostream>

// This test generates a small ROOT file compatible with the analysis pipeline

int main() {

    const std::string outpath = "out/output.root";

    // Create a small toy tree
    TFile fout(outpath.c_str(), "RECREATE");
    TTree tree("tree", "Toy Tree");

    float X, Q2, Z, PhPerp, PhiH, PhiS, Y;
    float Weight = 1.0f;
    float TrueX, TrueQ2, TrueZ, TruePhPerp, TruePhiH, TruePhiS, TrueY;
    int Spin_idx;

    tree.Branch("X", &X, "X/F");
    tree.Branch("Q", &Q2, "Q/F");
    tree.Branch("Z", &Z, "Z/F");
    tree.Branch("PhPerp", &PhPerp, "PhPerp/F");
    tree.Branch("PhiH", &PhiH, "PhiH/F");
    tree.Branch("PhiS", &PhiS, "PhiS/F");
    tree.Branch("Y", &Y, "Y/F");
    tree.Branch("TrueX", &TrueX, "TrueX/F");
    tree.Branch("TrueQ", &TrueQ2, "TrueQ/F");
    tree.Branch("TrueZ", &TrueZ, "TrueZ/F");
    tree.Branch("TruePhPerp", &TruePhPerp, "TruePhPerp/F");
    tree.Branch("TruePhiH", &TruePhiH, "TruePhiH/F");
    tree.Branch("TruePhiS", &TruePhiS, "TruePhiS/F");
    tree.Branch("TrueY", &TrueY, "TrueY/F");
    tree.Branch("Weight", &Weight, "Weight/F");
    tree.Branch("Spin_idx", &Spin_idx, "Spin_idx/I");

    TRandom3 rng(516);
    const int N = 20000;
    for (int i = 0; i < N; ++i) {
        TrueX = rng.Uniform(0.0, 1.0);
        TrueQ2 = rng.Uniform(1.0, 10.0);
        TrueZ = rng.Uniform(0.0, 1.0);
        TruePhPerp = rng.Uniform(0.0, 5.0);
        TruePhiH = rng.Uniform(-TMath::Pi(), TMath::Pi());
        TruePhiS = rng.Uniform(-TMath::Pi(), TMath::Pi());
        TrueY = rng.Uniform(0.0, 1.0);

        X = TrueX + rng.Gaus(0, 0.01);
        Q2 = TrueQ2 + rng.Gaus(0, 0.1);
        Z = TrueZ + rng.Gaus(0, 0.01);
        PhPerp = TruePhPerp + rng.Gaus(0, 0.01);
        PhiH = TruePhiH + rng.Gaus(0, 0.01);
        PhiS = TruePhiS + rng.Gaus(0, 0.01);
        Y = TrueY + rng.Gaus(0,0.1);

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

    return 0;
}