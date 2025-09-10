
#include "Inject.h"
#include <TRandom3.h>
#include <TMath.h>
#include <RooRealVar.h>
#include <RooDataSet.h>
#include <RooArgSet.h>
#include <RooGenericPdf.h>
#include <RooFitResult.h>
#include <RooFit.h>
#include <iostream>

using namespace RooFit;

Inject::Inject(TTree* tree, const Table* table) : tree(tree), table(table) {}
Inject::~Inject() {}

std::pair<double, double> Inject::injectExtractForBin(const Bin& bin, double A) {
    RooRealVar PhiH("PhiH", "PhiH", -TMath::Pi(), TMath::Pi());
    RooRealVar PhiS("PhiS", "PhiS", -TMath::Pi(), TMath::Pi());
    RooRealVar X("X", "X", bin.getMin("X"), bin.getMax("X"));
    RooRealVar Q("Q", "Q", bin.getMin("Q"), bin.getMax("Q"));
    RooRealVar Z("Z", "Z", bin.getMin("Z"), bin.getMax("Z"));
    RooRealVar PhPerp("PhPerp", "PhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"));
    RooRealVar Spin_idx("Spin_idx", "Spin_idx", -1, 1);
    RooArgSet obs(PhiH, PhiS, X, Q, Z, PhPerp, Spin_idx);

    // Create a dataset from the tree, applying the bin cuts
    std::string cut = "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
                      " && Q >= " + std::to_string(bin.getMin("Q")) + " && Q <= " + std::to_string(bin.getMax("Q")) +
                      " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
                      " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) + " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));

    RooDataSet data("data", "injected data", obs, Import(*tree), Cut(cut.c_str()));

    // Add the helicity branch to the dataset
    RooDataSet dataUpdate("dataUpdate", "data with updated spin", obs);
    TRandom3 rng(0);
    for (Long64_t i = 0; i < data.numEntries(); ++i) {
        const RooArgSet* row = data.get(i);
        PhiH.setVal(row->getRealValue("PhiH"));
        PhiS.setVal(row->getRealValue("PhiS"));
        double asymmetry = table->getAUT(row->getRealValue("X"), row->getRealValue("Q"), row->getRealValue("Z"), row->getRealValue("PhPerp"));
        double pPlus = 0.5 * (1 + asymmetry * std::sin(PhiH.getVal()));
        Spin_idx.setVal(rng.Rndm() < pPlus ? 1 : -1);
        X.setVal(row->getRealValue("X"));
        Q.setVal(row->getRealValue("Q"));
        Z.setVal(row->getRealValue("Z"));
        PhPerp.setVal(row->getRealValue("PhPerp"));
        dataUpdate.add(obs);
    }

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);
    RooGenericPdf model("model", "1 + Spin_idx * A * sin(PhiH)", RooArgList(PhiH, Spin_idx, A_fit));
    RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1));
    double val = A_fit.getVal();
    double error = A_fit.getError();
    delete fitResult;
    return std::make_pair(val, error);
}