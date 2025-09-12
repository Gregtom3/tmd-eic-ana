#include "Inject.h"
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooGenericPdf.h>
#include <RooRealVar.h>
#include <RooFormulaVar.h>
#include <TMath.h>
#include <TRandom3.h>
#include <iostream>
#include <limits>

using namespace RooFit;

Inject::Inject(TTree* tree, const Table* table, double scale, double targetPolarization)
    : tree(tree)
    , table(table)
    , m_scale(scale)
    , targetPolarization(targetPolarization) {}
Inject::~Inject() {}

std::pair<double, double> Inject::injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt) {
    if (!tree) {
        std::cerr << "[Inject::injectExtractForBin] Error: TTree pointer is null." << std::endl;
        return std::make_pair(0.0, 0.0);
    }
    // Decide whether the tree provides Q/TrueQ or Q2/TrueQ2.
    const bool hasQ = (tree->GetBranch("Q") != nullptr);
    const bool hasQ2 = (tree->GetBranch("Q2") != nullptr);
    const bool hasTrueQ = (tree->GetBranch("TrueQ") != nullptr);
    const bool hasTrueQ2 = (tree->GetBranch("TrueQ2") != nullptr);
    RooRealVar Y("Y", "Y", 0.0, 1.0);
    RooRealVar PhiH("PhiH", "PhiH", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar PhiS("PhiS", "PhiS", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar X("X", "X", bin.getMin("X"), bin.getMax("X"));
    // Create a Depolarization FormulaVar from Y
    RooFormulaVar Depol1("Depol1", "(1 - Y)/(1 - Y + 0.5 * Y * Y)", RooArgList(Y));
    // Q handling: prefer Q, otherwise create a formula Q = sqrt(Q2)
    RooRealVar* Q_ptr = nullptr;
    RooRealVar* Q2_ptr = nullptr;
    RooFormulaVar* Q_formula = nullptr;
    if (hasQ) {
        Q_ptr = new RooRealVar("Q", "Q", bin.getMin("Q"), bin.getMax("Q"));
    } else if (hasQ2) {
        // create Q2 real var (will be filled from the tree) and a formula Q = sqrt(Q2)
        // Compute Q2 bounds safely: if the Q interval spans zero, Q2 minimum is 0.
        double qmin = bin.getMin("Q");
        double qmax = bin.getMax("Q");
        double q2min, q2max;
        if (qmin < 0.0 && qmax > 0.0) {
            q2min = 0.0;
            q2max = std::max(qmin * qmin, qmax * qmax);
        } else {
            q2min = std::min(qmin * qmin, qmax * qmax);
            q2max = std::max(qmin * qmin, qmax * qmax);
        }
        // Guard against degenerate range (shouldn't happen with above logic but be defensive)
        if (q2max <= q2min) q2max = q2min + 1e-6;
        Q2_ptr = new RooRealVar("Q2", "Q2", q2min, q2max);
        Q_formula = new RooFormulaVar("Q", "sqrt(Q2)", RooArgList(*Q2_ptr));
        Q_ptr = nullptr; // Q is represented by formula
    } else {
        std::cerr << "Inject: neither 'Q' nor 'Q2' branch found in tree" << std::endl;
    }

    RooRealVar Z("Z", "Z", bin.getMin("Z"), bin.getMax("Z"));
    RooRealVar PhPerp("PhPerp", "PhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"));
    RooRealVar TrueY("TrueY", "TrueY", -999, 999);
    RooFormulaVar TrueDepol1("TrueDepol1", "(1 - TrueY)/(1 - TrueY + 0.5 * TrueY * TrueY)", RooArgList(TrueY));
    RooRealVar TruePhiH("TruePhiH", "TruePhiH", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar TruePhiS("TruePhiS", "TruePhiS", -2*TMath::Pi(), 2*TMath::Pi());

    // TrueQ handling analogous to Q
    RooRealVar* TrueQ_ptr = nullptr;
    RooRealVar* TrueQ2_ptr = nullptr;
    RooFormulaVar* TrueQ_formula = nullptr;
    if (hasTrueQ) {
        TrueQ_ptr = new RooRealVar("TrueQ", "TrueQ", -999, 99999999); // wide range permissible in case of smearing
    } else if (hasTrueQ2) {
        double tqmin = bin.getMin("Q");
        double tqmax = bin.getMax("Q");
        double tq2min, tq2max;
        if (tqmin < 0.0 && tqmax > 0.0) {
            tq2min = 0.0;
            tq2max = std::max(tqmin * tqmin, tqmax * tqmax);
        } else {
            tq2min = std::min(tqmin * tqmin, tqmax * tqmax);
            tq2max = std::max(tqmin * tqmin, tqmax * tqmax);
        }
        if (tq2max <= tq2min) tq2max = tq2min + 1e-6;
        TrueQ2_ptr = new RooRealVar("TrueQ2", "TrueQ2", -999, 99999999); // wide range permissible in case of smearing
        TrueQ_formula = new RooFormulaVar("TrueQ", "sqrt(TrueQ2)", RooArgList(*TrueQ2_ptr));
        TrueQ_ptr = nullptr;
    } else {
        std::cerr << "Inject: neither 'TrueQ' nor 'TrueQ2' branch found in tree" << std::endl;
        // Return default value if neither TrueQ nor TrueQ2 is present
        return std::make_pair(0.0, 0.0);
    }
    RooRealVar TrueX("TrueX", "TrueX", -999, 999); // wide range permissible in case of smearing
    RooRealVar TrueZ("TrueZ", "TrueZ", -999, 999);
    RooRealVar TruePhPerp("TruePhPerp", "TruePhPerp", -999, 999);
    RooRealVar Spin_idx("Spin_idx", "Spin_idx", -1, 1);
    RooRealVar Weight("Weight", "Weight", 0, 1e9);
    RooRealVar tPol("tPol", "Target Polarization", targetPolarization); 
    tPol.setConstant(true);
    // Build the observable set. Use the RooFormulaVar objects if created.
    RooArgSet obs;
    obs.add(PhiH);
    obs.add(PhiS);
    obs.add(X);
    // RooDataSet cannot contain non-fundamental types (like RooFormulaVar).
    // Add the underlying real variable (Q or Q2) to the observable set.
    if (Q2_ptr) obs.add(*Q2_ptr); else if (Q_ptr) obs.add(*Q_ptr);
    obs.add(Z);
    obs.add(PhPerp);
    obs.add(TruePhiH);
    obs.add(TruePhiS);
    obs.add(TrueX);
    if (TrueQ2_ptr) obs.add(*TrueQ2_ptr); else if (TrueQ_ptr) obs.add(*TrueQ_ptr);
    obs.add(TrueZ);
    obs.add(TruePhPerp);
    obs.add(Spin_idx);
    obs.add(Weight);

    // Create a dataset from the tree, applying the bin cuts
    // If the tree only has Q2, express the cut in Q2 (square the Q bounds) so
    // RooFormula/TFormula doesn't attempt to compile a symbol 'Q' that isn't present.
    std::string cut;
    if (hasQ) {
        cut = "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
              " && Q >= " + std::to_string(bin.getMin("Q")) + " && Q <= " + std::to_string(bin.getMax("Q")) +
              " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
              " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
              " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));
    } else if (hasQ2) {
        const double qmin = bin.getMin("Q");
        const double qmax = bin.getMax("Q");
        const double q2min = qmin * qmin;
        const double q2max = qmax * qmax;
        cut = "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
              " && Q2 >= " + std::to_string(q2min) + " && Q2 <= " + std::to_string(q2max) +
              " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
              " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
              " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));
    } else {
        // Fallback: build cut without Q (will select all Q)
        cut = "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
              " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
              " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
              " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));
    }

    RooDataSet data("data", "injected data", obs, Import(*tree), Cut(cut.c_str()), WeightVar("Weight"));
    std::cout << "[Inject::injectExtractForBin] Selected " << data.numEntries() << " events for injection." << std::endl;
    // Add the helicity branch to the dataset
    RooDataSet dataUpdate("dataUpdate", "data with updated spin", obs, WeightVar("Weight"));
    TRandom3 rng(0);
    double expected_events = 0.0;
    for (Long64_t i = 0; i < data.numEntries(); ++i) {
        const RooArgSet* row = data.get(i);
        TruePhiH.setVal(row->getRealValue("TruePhiH"));
        TruePhiS.setVal(row->getRealValue("TruePhiS"));
        TrueY.setVal(row->getRealValue("TrueY"));
        double true_depol1 = TrueDepol1.getVal();
        double trueq_val = 0.0;
        if (hasTrueQ) {
            trueq_val = row->getRealValue("TrueQ");
        } else if (hasTrueQ2) {
            // TrueQ = sqrt(TrueQ2)
            trueq_val = std::sqrt(row->getRealValue("TrueQ2"));
        }
        // Determine asymmetry to inject
        double asymmetry = 0.0;
        if(A_opt.has_value()) {
            asymmetry = A_opt.value();
        }
        else{
            asymmetry = table->lookupAUT(row->getRealValue("TrueX"), trueq_val, row->getRealValue("TrueZ"),
                                        row->getRealValue("TruePhPerp"));
        }
        double pPlus = 0.5 * (1 + true_depol1 * asymmetry * std::sin(TruePhiH.getVal() + TruePhiS.getVal()));
        Spin_idx.setVal(rng.Rndm() < pPlus ? 1 : -1);
        if(rng.Rndm() > targetPolarization){
            // Set Spin_idx to -1 or 1 with 50/50 chance
            Spin_idx.setVal(rng.Rndm() < 0.5 ? 1 : -1);
        }
        X.setVal(row->getRealValue("X"));
        double q_val = 0.0;
        if (hasQ) {
            q_val = row->getRealValue("Q");
        } else if (hasQ2) {
            q_val = std::sqrt(row->getRealValue("Q2"));
        }
        // Set underlying variables: if Q is formula from Q2, set Q2; otherwise set Q
        if (Q_ptr) {
            Q_ptr->setVal(q_val);
        } else if (Q2_ptr) {
            Q2_ptr->setVal(q_val * q_val);
        }
        Z.setVal(row->getRealValue("Z"));
        PhPerp.setVal(row->getRealValue("PhPerp"));
        PhiH.setVal(row->getRealValue("PhiH"));
        PhiS.setVal(row->getRealValue("PhiS"));
        dataUpdate.add(obs, data.weight() * m_scale);
        expected_events+= data.weight() * m_scale;
    }

    // Save number of injection data points to bin
    bin.setEvents(dataUpdate.numEntries());
    bin.setExpectedEvents(static_cast<int>(std::round(expected_events)));

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);
    // Determine which PhiH/PhiS to use for extraction
    double val = 0.0;
    double error = 0.0;
    if (extract_with_true) {
        RooGenericPdf model("model", "1 + TrueDepol1 * tPol * Spin_idx * A * sin(TruePhiH+TruePhiS)", RooArgList(TruePhiH, TruePhiS, TrueDepol1, tPol, Spin_idx, A_fit));
        RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1), SumW2Error(kTRUE));
        val = A_fit.getVal();
        error = A_fit.getError();
        delete fitResult;
    } else {
        RooGenericPdf model("model", "1 + Depol1 * tPol * Spin_idx * A * sin(PhiH+PhiS)", RooArgList(PhiH, PhiS, Depol1, tPol, Spin_idx, A_fit));
        RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1), SumW2Error(kTRUE));
        val = A_fit.getVal();
        error = A_fit.getError();
        delete fitResult;
    }
    // Clean up any heap-allocated Roo objects
    if (Q_ptr) delete Q_ptr;
    if (Q2_ptr) delete Q2_ptr;
    if (Q_formula) delete Q_formula;
    if (TrueQ_ptr) delete TrueQ_ptr;
    if (TrueQ2_ptr) delete TrueQ2_ptr;
    if (TrueQ_formula) delete TrueQ_formula;
    return std::make_pair(val, error);
}