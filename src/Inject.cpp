#include "Inject.h"
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooGenericPdf.h>
#include <RooRealVar.h>
#include <TMath.h>
#include <TRandom3.h>
#include <iostream>

using namespace RooFit;

Inject::Inject(TTree* tree, const Table* table)
    : tree(tree)
    , table(table) {}
Inject::~Inject() {}

std::pair<double, double> Inject::injectExtractForBin(const Bin& bin, double A) {
    RooRealVar PhiH("PhiH", "PhiH", -TMath::Pi(), TMath::Pi());
    RooRealVar PhiS("PhiS", "PhiS", -TMath::Pi(), TMath::Pi());
    RooRealVar X("X", "X", bin.getMin("X"), bin.getMax("X"));
    RooRealVar Q("Q", "Q", bin.getMin("Q"), bin.getMax("Q"));
    RooRealVar Z("Z", "Z", bin.getMin("Z"), bin.getMax("Z"));
    RooRealVar PhPerp("PhPerp", "PhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"));
    RooRealVar TruePhiH("TruePhiH", "TruePhiH", -TMath::Pi(), TMath::Pi());
    RooRealVar TruePhiS("TruePhiS", "TruePhiS", -TMath::Pi(), TMath::Pi());
    RooRealVar TrueX("TrueX", "TrueX", bin.getMin("X"), bin.getMax("X"));
    RooRealVar TrueQ("TrueQ", "TrueQ", bin.getMin("Q"), bin.getMax("Q"));
    RooRealVar TrueZ("TrueZ", "TrueZ", bin.getMin("Z"), bin.getMax("Z"));
    RooRealVar TruePhPerp("TruePhPerp", "TruePhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"));

    RooRealVar Spin_idx("Spin_idx", "Spin_idx", -1, 1);
    RooRealVar Weight("Weight", "Weight", 0, 1e9);
    RooArgSet obs(PhiH, PhiS, X, Q, Z, PhPerp, TruePhiH, TruePhiS, TrueX, TrueQ, TrueZ, TruePhPerp, Spin_idx, Weight);

    // Create a dataset from the tree, applying the bin cuts
    std::string cut = "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
                      " && Q >= " + std::to_string(bin.getMin("Q")) + " && Q <= " + std::to_string(bin.getMax("Q")) +
                      " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
                      " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
                      " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));

    RooDataSet data("data", "injected data", obs, Import(*tree), Cut(cut.c_str()), WeightVar("Weight"));

    // Add the helicity branch to the dataset
    RooDataSet dataUpdate("dataUpdate", "data with updated spin", obs, WeightVar("Weight"));
    TRandom3 rng(0);
    for (Long64_t i = 0; i < data.numEntries(); ++i) {
        const RooArgSet* row = data.get(i);
        TruePhiH.setVal(row->getRealValue("TruePhiH"));
        TruePhiS.setVal(row->getRealValue("TruePhiS"));
        double asymmetry = table->lookupAUT(row->getRealValue("TrueX"), row->getRealValue("TrueQ"), row->getRealValue("TrueZ"),
                                            row->getRealValue("TruePhPerp"));
        asymmetry = 0.1;
        double pPlus = 0.5 * (1 + asymmetry * std::sin(TruePhiH.getVal() + TruePhiS.getVal()));
        Spin_idx.setVal(rng.Rndm() < pPlus ? 1 : -1);
        X.setVal(row->getRealValue("X"));
        Q.setVal(row->getRealValue("Q"));
        Z.setVal(row->getRealValue("Z"));
        PhPerp.setVal(row->getRealValue("PhPerp"));
        PhiH.setVal(row->getRealValue("PhiH"));
        PhiS.setVal(row->getRealValue("PhiS"));
        dataUpdate.add(obs, data.weight());
    }

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);
    RooGenericPdf model("model", "1 + Spin_idx * A * sin(PhiH+PhiS)", RooArgList(PhiH, PhiS, Spin_idx, A_fit));
    RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1), SumW2Error(kTRUE));
    double val = A_fit.getVal();
    double error = A_fit.getError();
    delete fitResult;
    return std::make_pair(val, error);
}