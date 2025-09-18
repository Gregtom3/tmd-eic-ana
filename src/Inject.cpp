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

    RooRealVar Y("Y", "Y", 0.0, 1.0);
    RooRealVar PhiH("PhiH", "PhiH", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar PhiS("PhiS", "PhiS", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar X("X", "X", bin.getMin("X"), bin.getMax("X"));
    RooFormulaVar Depol1("Depol1", "(1 - Y)/(1 - Y + 0.5 * Y * Y)", RooArgList(Y));

    RooRealVar Q2("Q2", "Q2", bin.getMin("Q")*bin.getMin("Q"), bin.getMax("Q")*bin.getMax("Q"));
    RooRealVar Z("Z", "Z", bin.getMin("Z"), bin.getMax("Z"));
    RooRealVar PhPerp("PhPerp", "PhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"));

    RooRealVar TrueY("TrueY", "TrueY", -999, 999);
    RooFormulaVar TrueDepol1("TrueDepol1", "(1 - TrueY)/(1 - TrueY + 0.5 * TrueY * TrueY)", RooArgList(TrueY));
    RooRealVar TruePhiH("TruePhiH", "TruePhiH", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar TruePhiS("TruePhiS", "TruePhiS", -2*TMath::Pi(), 2*TMath::Pi());
    RooRealVar TrueQ2("TrueQ2", "TrueQ2", -999, 99999999);
    RooRealVar TrueX("TrueX", "TrueX", -999, 999);
    RooRealVar TrueZ("TrueZ", "TrueZ", -999, 999);
    RooRealVar TruePhPerp("TruePhPerp", "TruePhPerp", -999, 999);

    RooRealVar Spin_idx("Spin_idx", "Spin_idx", -1, 1);
    RooRealVar Weight("Weight", "Weight", 0, 1e9);
    RooRealVar TotalWeight("TotalWeight", "TotalWeight", 0, 1e9);
    RooRealVar tPol("tPol", "Target Polarization", targetPolarization); 
    tPol.setConstant(true);

    RooRealVar Gamma("Gamma", "Gamma (2*X*0.938272/sqrt(Q2))", -999, 999);
    RooRealVar TrueGamma("TrueGamma", "TrueGamma (2*TrueX*0.938272/sqrt(TrueQ2))", -999, 999);
    RooRealVar S_T("S_T", "Transverse spin magnitude S_T", -999, 999);
    RooRealVar TrueS_T("TrueS_T", "Transverse spin magnitude TrueS_T", -999, 999);

    RooArgSet obs;
    obs.add(PhiH);
    obs.add(PhiS);
    obs.add(X);
    obs.add(Q2);
    obs.add(Z);
    obs.add(Y);
    obs.add(PhPerp);
    obs.add(TruePhiH);
    obs.add(TruePhiS);
    obs.add(TrueX);
    obs.add(TrueQ2);
    obs.add(TrueY);
    obs.add(TrueZ);
    obs.add(TruePhPerp);
    obs.add(Spin_idx);
    obs.add(Weight);

    std::string cut =
        "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
        " && Q2 >= " + std::to_string(bin.getMin("Q")*bin.getMin("Q")) +
        " && Q2 <= " + std::to_string(bin.getMax("Q")*bin.getMax("Q")) +
        " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
        " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
        " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));

    RooDataSet data("data", "injected data", obs, Import(*tree), Cut(cut.c_str()));
    std::cout << "[Inject::injectExtractForBin] Selected " << data.numEntries() << " events for injection." << std::endl;
    obs.add(S_T);
    obs.add(TrueS_T);
    // obs.add(TotalWeight);
    RooDataSet dataUpdate("dataUpdate", "data with updated spin", obs, WeightVar(TotalWeight));
    TRandom3 rng(0);
    double expected_events = 0.0;
    double sumW = 0.0;
    double sumW2 = 0.0;

    for (Long64_t i = 0; i < data.numEntries(); ++i) {
        const RooArgSet* row = data.get(i);
        TruePhiH.setVal(row->getRealValue("TruePhiH"));
        TruePhiS.setVal(row->getRealValue("TruePhiS"));
        TrueY.setVal(row->getRealValue("TrueY"));
        TrueX.setVal(row->getRealValue("TrueX"));
        X.setVal(row->getRealValue("X"));
        Z.setVal(row->getRealValue("Z"));
        PhPerp.setVal(row->getRealValue("PhPerp"));
        PhiH.setVal(row->getRealValue("PhiH"));
        PhiS.setVal(row->getRealValue("PhiS"));
        Weight.setVal(row->getRealValue("Weight"));
        TotalWeight.setVal(Weight.getVal() * m_scale);
        double true_depol1 = TrueDepol1.getVal();
        double trueq_val = std::sqrt(row->getRealValue("TrueQ2"));
        double gamma_val = Gamma.getVal();
        double y_val = Y.getVal();
        double inner = (1.0 - y_val - 0.25 * y_val * y_val * gamma_val * gamma_val) / (1.0 + gamma_val * gamma_val);
        if (inner < 0.0) inner = 0.0;
        double sinTheta = gamma_val * std::sqrt(inner);
        if (sinTheta > 1.0) sinTheta = 1.0;
        double cosTheta = std::sqrt(std::max(0.0, 1.0 - sinTheta * sinTheta));
        double phiS_val = PhiS.getVal();
        double denom_ST = std::sqrt(std::max(1e-12, 1.0 - sinTheta*sinTheta*std::sin(phiS_val)*std::sin(phiS_val)));
        double ST_val = cosTheta / denom_ST;
        if (!std::isfinite(ST_val)) ST_val = 0.0;
        S_T.setVal(ST_val);

        double true_gamma_val = TrueGamma.getVal();
        double true_y_val = TrueY.getVal();
        double inner_true = (1.0 - true_y_val - 0.25 * true_y_val * true_y_val * true_gamma_val * true_gamma_val) / (1.0 + true_gamma_val * true_gamma_val);
        if (inner_true < 0.0) inner_true = 0.0;
        double sinTheta_true = true_gamma_val * std::sqrt(inner_true);
        if (sinTheta_true > 1.0) sinTheta_true = 1.0;
        double cosTheta_true = std::sqrt(std::max(0.0, 1.0 - sinTheta_true * sinTheta_true));
        double truePhiS_val = TruePhiS.getVal();
        double denom_TrueST = std::sqrt(std::max(1e-12, 1.0 - sinTheta_true*sinTheta_true*std::sin(truePhiS_val)*std::sin(truePhiS_val)));
        double TrueST_val = cosTheta_true / denom_TrueST;
        if (!std::isfinite(TrueST_val)) TrueST_val = 0.0;
        TrueS_T.setVal(TrueST_val);
        if(TrueST_val<0) LOG_DEBUG("Warning: TrueS_T < 0: " + std::to_string(TrueST_val) + " (cosTheta_true=" + std::to_string(cosTheta_true) + ", denom_TrueST=" + std::to_string(denom_TrueST) + ", sinTheta_true=" + std::to_string(sinTheta_true) + ", truePhiS_val=" + std::to_string(truePhiS_val) + ")");
        // Determine asymmetry to inject
        double asymmetry = 0.0;
        if(A_opt.has_value()) {
            asymmetry = A_opt.value();
        }
        else{
            asymmetry = table->lookupAUT(TrueX.getVal(), trueq_val, TrueZ.getVal(), TruePhPerp.getVal());
        }
        double pPlus = 0.5 * (1 + TrueST_val * true_depol1 * asymmetry * std::sin(TruePhiH.getVal() + TruePhiS.getVal()));
        Spin_idx.setVal(rng.Rndm() < pPlus ? 1 : -1);
        if(rng.Rndm() > targetPolarization){
            // Set Spin_idx to -1 or 1 with 50/50 chance
            Spin_idx.setVal(rng.Rndm() < 0.5 ? 1 : -1);
        }

        dataUpdate.add(obs);
        expected_events+=TotalWeight.getVal();
        sumW += Weight.getVal();
        sumW2 += Weight.getVal()*Weight.getVal();
    }

    // Get effective MC events
    double n_eff_mc = (sumW*sumW)/sumW2;
    // Save number of injection data points to bin
    bin.setEvents(dataUpdate.numEntries());
    bin.setExpectedEvents(static_cast<int>(std::round(expected_events)));

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);
    // Determine which PhiH/PhiS to use for extraction
    double val = 0.0;
    double error = 0.0;
    if (extract_with_true) {
        RooGenericPdf model("model", "1 + TrueS_T * TrueDepol1 * tPol * Spin_idx * A * sin(TruePhiH+TruePhiS)", RooArgList(TrueS_T, TruePhiH, TruePhiS, TrueDepol1, tPol, Spin_idx, A_fit));
        RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1), SumW2Error(kTRUE));
        val = A_fit.getVal();
        error = A_fit.getError() * std::sqrt(n_eff_mc/expected_events);
        delete fitResult;
    } else {
        RooGenericPdf model("model", "1 + S_T * Depol1 * tPol * Spin_idx * A * sin(PhiH+PhiS)", RooArgList(S_T, PhiH, PhiS, Depol1, tPol, Spin_idx, A_fit));
        RooFitResult* fitResult = model.fitTo(dataUpdate, Save(), PrintLevel(-1), SumW2Error(kTRUE));
        val = A_fit.getVal();
        error = A_fit.getError() * std::sqrt(n_eff_mc/expected_events);
        delete fitResult;
    }
    return std::make_pair(val, error);
}
