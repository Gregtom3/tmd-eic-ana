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

    std::string cut = "";
    if(extract_with_true){
        cut =
            "TrueX >= " + std::to_string(bin.getMin("X")) + " && TrueX <= " + std::to_string(bin.getMax("X")) +
            " && TrueQ2 >= " + std::to_string(bin.getMin("Q")*bin.getMin("Q")) +
            " && TrueQ2 <= " + std::to_string(bin.getMax("Q")*bin.getMax("Q")) +
            " && TrueZ >= " + std::to_string(bin.getMin("Z")) + " && TrueZ <= " + std::to_string(bin.getMax("Z")) +
            " && TruePhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
            " && TruePhPerp <= " + std::to_string(bin.getMax("PhPerp"));
    }
    else{
        cut =
        "X >= " + std::to_string(bin.getMin("X")) + " && X <= " + std::to_string(bin.getMax("X")) +
        " && Q2 >= " + std::to_string(bin.getMin("Q")*bin.getMin("Q")) +
        " && Q2 <= " + std::to_string(bin.getMax("Q")*bin.getMax("Q")) +
        " && Z >= " + std::to_string(bin.getMin("Z")) + " && Z <= " + std::to_string(bin.getMax("Z")) +
        " && PhPerp >= " + std::to_string(bin.getMin("PhPerp")) +
        " && PhPerp <= " + std::to_string(bin.getMax("PhPerp"));
    }
    
    // Set up branch variables
    double b_PhiH=0, b_PhiS=0, b_X=0, b_Q2=0, b_Z=0, b_PhPerp=0;
    double b_TruePhiH=0, b_TruePhiS=0, b_TrueX=0, b_TrueQ2=0, b_TrueY=0, b_TrueZ=0, b_TruePhPerp=0;
    double b_Weight=0, b_Y=0;

    tree->SetBranchAddress("PhiH", &b_PhiH);
    tree->SetBranchAddress("PhiS", &b_PhiS);
    tree->SetBranchAddress("X", &b_X);
    tree->SetBranchAddress("Q2", &b_Q2);
    tree->SetBranchAddress("Z", &b_Z);
    tree->SetBranchAddress("PhPerp", &b_PhPerp);
    tree->SetBranchAddress("TruePhiH", &b_TruePhiH);
    tree->SetBranchAddress("TruePhiS", &b_TruePhiS);
    tree->SetBranchAddress("TrueX", &b_TrueX);
    tree->SetBranchAddress("TrueQ2", &b_TrueQ2);
    tree->SetBranchAddress("TrueY", &b_TrueY);
    tree->SetBranchAddress("TrueZ", &b_TrueZ);
    tree->SetBranchAddress("TruePhPerp", &b_TruePhPerp);
    tree->SetBranchAddress("Weight", &b_Weight);
    tree->SetBranchAddress("Y", &b_Y);

    obs.add(S_T);
    obs.add(TrueS_T);
    RooDataSet dataUpdate("dataUpdate", "data with updated spin", obs, WeightVar(TotalWeight));
    TRandom3 rng(0);
    double expected_events = 0.0;
    double sumW = 0.0;
    double sumW2 = 0.0;
    double sumTrueAsymW = 0.0;
    double sumRecoAsymW = 0.0;

    // Precompute selection bounds for speed
    const double minX = bin.getMin("X");
    const double maxX = bin.getMax("X");
    const double minQ2 = bin.getMin("Q") * bin.getMin("Q");
    const double maxQ2 = bin.getMax("Q") * bin.getMax("Q");
    const double minZ = bin.getMin("Z");
    const double maxZ = bin.getMax("Z");
    const double minPhPerp = bin.getMin("PhPerp");
    const double maxPhPerp = bin.getMax("PhPerp");

    Long64_t nentries = tree->GetEntries();
    Long64_t selected_count = 0;
    // Prepare progress bar printing
    const Long64_t progress_steps = std::min<Long64_t>(100, std::max<Long64_t>(1, nentries/100));
    Long64_t next_progress = progress_steps;
    for (Long64_t i = 0; i < nentries; ++i) {
        tree->GetEntry(i);
        // Update progress bar occasionally
        if (i >= next_progress || i == 0 || i == nentries-1) {
            int percent = static_cast<int>(100.0 * (i+1) / std::max<Long64_t>(1, nentries));
            std::cout << "\r[" << percent << "%] Processing entry " << (i+1) << " / " << nentries << std::flush;
            next_progress = i + progress_steps;
            if (i == nentries-1) std::cout << std::endl;
        }
        // Apply selection cuts using either true or reconstructed variables
        if (extract_with_true) {
            if (!(b_TrueX >= minX && b_TrueX <= maxX && b_TrueQ2 >= minQ2 && b_TrueQ2 <= maxQ2 && b_TrueZ >= minZ && b_TrueZ <= maxZ && b_TruePhPerp >= minPhPerp && b_TruePhPerp <= maxPhPerp)) continue;
        } else {
            if (!(b_X >= minX && b_X <= maxX && b_Q2 >= minQ2 && b_Q2 <= maxQ2 && b_Z >= minZ && b_Z <= maxZ && b_PhPerp >= minPhPerp && b_PhPerp <= maxPhPerp)) continue;
        }
        ++selected_count;

        // Populate RooRealVars from branch values
        TruePhiH.setVal(b_TruePhiH);
        TruePhiS.setVal(b_TruePhiS);
        TrueY.setVal(b_TrueY);
        TrueX.setVal(b_TrueX);
        X.setVal(b_X);
        Z.setVal(b_Z);
        PhPerp.setVal(b_PhPerp);
        PhiH.setVal(b_PhiH);
        PhiS.setVal(b_PhiS);
        Y.setVal(b_Y);
        Weight.setVal(b_Weight);
        TotalWeight.setVal(Weight.getVal() * m_scale);

        // Compute gammas from X and Q2 (protect against non-positive Q2)
        if (b_Q2 > 0) {
            Gamma.setVal(2.0 * X.getVal() * 0.938272 / std::sqrt(b_Q2));
        } else {
            Gamma.setVal(0.0);
        }
        if (b_TrueQ2 > 0) {
            TrueGamma.setVal(2.0 * TrueX.getVal() * 0.938272 / std::sqrt(b_TrueQ2));
        } else {
            TrueGamma.setVal(0.0);
        }

        double true_depol1 = TrueDepol1.getVal();
        double q_val     = std::sqrt(std::max(0.0, b_Q2));
        double trueq_val = std::sqrt(std::max(0.0, b_TrueQ2));
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
        double trueAsymmetry = 0.0; // asymmetry corresponding to the actual physics process
        double recoAsymmetry = 0.0; // asymmetry expected if we believed the reconstructed event to be true
                                    // with more smearing, these two values are expected to differ
        if(A_opt.has_value()) {
            trueAsymmetry = A_opt.value();
            recoAsymmetry = A_opt.value();
        }
        else{
            trueAsymmetry = table->lookupAUT(TrueX.getVal(), trueq_val, TrueZ.getVal(), TruePhPerp.getVal());
            recoAsymmetry = table->lookupAUT(X.getVal(), q_val, Z.getVal(), PhPerp.getVal());
        }
        double pPlus = 0.5 * (1 + TrueST_val * true_depol1 * trueAsymmetry * std::sin(TruePhiH.getVal() + TruePhiS.getVal()));
        Spin_idx.setVal(rng.Rndm() < pPlus ? 1 : -1);
        if(rng.Rndm() > targetPolarization){
            // Set Spin_idx to -1 or 1 with 50/50 chance
            Spin_idx.setVal(rng.Rndm() < 0.5 ? 1 : -1);
        }

        dataUpdate.add(obs);
        expected_events+=TotalWeight.getVal();
        sumW += Weight.getVal();
        sumW2 += Weight.getVal()*Weight.getVal();
        sumTrueAsymW += Weight.getVal()*trueAsymmetry;
        sumRecoAsymW += Weight.getVal()*recoAsymmetry;
    }
    std::cout << "[Inject::injectExtractForBin] Selected " << selected_count << " events for injection (after tree loop)." << std::endl;

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
    // Get effective true injected asymmetry
    double eff_inj_tasym = sumTrueAsymW/sumW;
    // Get effective reco asymmetry
    double eff_inj_rasym = sumRecoAsymW/sumW;

    std::cout << "======================== Asymmetry Results ========================\n" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << " bool extract_with_true = " << extract_with_true << std::endl;
    std::cout << " ------------------------------------------------------------------" << std::endl;
    std::cout << " Asymmetry Extracted = " << val << " +/- " << A_fit.getError() << std::endl;
    std::cout << " Asymmetry Extracted (w/ scaled EIC errors) = " << val << " +/- " << error << std::endl;
    std::cout << " Effective Truth Injected Asymmetry = " << eff_inj_tasym << std::endl;
    std::cout << " Effective Reco Injected Asymmetry = " << eff_inj_rasym << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    return std::make_pair(val, error);
}
