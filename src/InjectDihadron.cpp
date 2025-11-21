#include "Inject.h"
#include <RooArgList.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooGenericPdf.h>
#include <cmath>
#include <TMath.h>
#include <iostream>

using namespace RooFit;

InjectDihadron::InjectDihadron(TTree* tree, const Table* table, double scale, double targetPolarization, channel_type channel)
    : Inject(tree, table, scale, targetPolarization, channel) {}
InjectDihadron::~InjectDihadron() {}

std::pair<double, double> InjectDihadron::injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt) {
    auto variables = createLoopVariables(bin);

    // Custom dihadron variables
    auto* PhiRperp = variables->addExtraRealVar(std::make_unique<RooRealVar>("PhiRperp", "PhiRperp", -2 * TMath::Pi(), 2 * TMath::Pi()));
    auto* Depol1   = variables->addExtraRealVar(std::make_unique<RooRealVar>("Depol1", "Depol1", 0.0, 1.0));
    auto* Depol2   = variables->addExtraRealVar(std::make_unique<RooRealVar>("Depol2", "Depol2", 0.0, 1.0));
    auto* Depol3   = variables->addExtraRealVar(std::make_unique<RooRealVar>("Depol3", "Depol3", 0.0, 1.0));
    auto* Depol4   = variables->addExtraRealVar(std::make_unique<RooRealVar>("Depol4", "Depol4", 0.0, 1.0));
    auto* TruePhiRperp = variables->addExtraRealVar(std::make_unique<RooRealVar>("TruePhiRperp", "TruePhiRperp", -2 * TMath::Pi(), 2 * TMath::Pi()));
    auto* TrueDepol1   = variables->addExtraRealVar(std::make_unique<RooRealVar>("TrueDepol1", "TrueDepol1", 0.0, 1.0));
    auto* TrueDepol2   = variables->addExtraRealVar(std::make_unique<RooRealVar>("TrueDepol2", "TrueDepol2", 0.0, 1.0));
    auto* TrueDepol3   = variables->addExtraRealVar(std::make_unique<RooRealVar>("TrueDepol3", "TrueDepol3", 0.0, 1.0));
    auto* TrueDepol4   = variables->addExtraRealVar(std::make_unique<RooRealVar>("TrueDepol4", "TrueDepol4", 0.0, 1.0));

    // Local buffers for branch binding
    double b_PhiRperp = 0.0;
    double b_Depol1 = 0.0, b_Depol2 = 0.0, b_Depol3 = 0.0, b_Depol4 = 0.0;
    double b_TruePhiRperp = 0.0;
    double b_TrueDepol1 = 0.0, b_TrueDepol2 = 0.0, b_TrueDepol3 = 0.0, b_TrueDepol4 = 0.0;

    LoopHandlers handlers;

    handlers.bindCustomBranches = [&](TTree& t) {
        t.SetBranchAddress("PhiRperp", &b_PhiRperp);
        t.SetBranchAddress("Depol1", &b_Depol1);
        t.SetBranchAddress("Depol2", &b_Depol2);
        t.SetBranchAddress("Depol3", &b_Depol3);
        t.SetBranchAddress("Depol4", &b_Depol4);
        t.SetBranchAddress("TruePhiRperp", &b_TruePhiRperp);
        t.SetBranchAddress("TrueDepol1", &b_TrueDepol1);
        t.SetBranchAddress("TrueDepol2", &b_TrueDepol2);
        t.SetBranchAddress("TrueDepol3", &b_TrueDepol3);
        t.SetBranchAddress("TrueDepol4", &b_TrueDepol4);
    };

    handlers.updateCustomVariables = [&](const EventRecord&, LoopVariables& vars) {
        (void)vars; // vars used indirectly by pointers
        PhiRperp->setVal(b_PhiRperp);
        Depol1->setVal(b_Depol1);
        Depol2->setVal(b_Depol2);
        Depol3->setVal(b_Depol3);
        Depol4->setVal(b_Depol4);
        TruePhiRperp->setVal(b_TruePhiRperp);
        TrueDepol1->setVal(b_TrueDepol1);
        TrueDepol2->setVal(b_TrueDepol2);
        TrueDepol3->setVal(b_TrueDepol3);
        TrueDepol4->setVal(b_TrueDepol4);
    };

    handlers.computeAsymmetries = [&](const EventRecord& record, const LoopVariables&, bool, std::optional<double> overrideA) {
        AsymmetryValues values;
        if (overrideA.has_value()) {
            values.trueAsymmetry = overrideA.value();
            values.recoAsymmetry = overrideA.value();
            return values;
        }
        const double recoQ = record.Q2 > 0.0 ? std::sqrt(record.Q2) : 0.0;
        const double trueQ = record.TrueQ2 > 0.0 ? std::sqrt(record.TrueQ2) : 0.0;
        if (table) {
            values.trueAsymmetry = table->lookupAUT_DISIDIS(record.TrueX, trueQ, record.TrueZ, record.TrueMh);
            values.recoAsymmetry = table->lookupAUT_DISIDIS(record.X, recoQ, record.Z, record.Mh);
        }
        return values;
    };

    handlers.computeSpinProbability = [&](const EventRecord& record, const LoopVariables& vars, const AsymmetryValues& asym, bool) {
        const double trueST = vars.TrueST->getVal();
        const double depRatio = (TrueDepol1->getVal() != 0.0) ? (TrueDepol2->getVal() / TrueDepol1->getVal()) : 0.0;
        const double phase = TruePhiRperp->getVal() + record.TruePhiS;
        return 0.5 * (1.0 + trueST * depRatio * asym.trueAsymmetry * std::sin(phase));
    };

    auto loopResult = runInjectionLoop(bin, extract_with_true, A_opt, std::move(variables), handlers);

    lastStats = loopResult.stats;
    if (!loopResult.dataSet || loopResult.dataSet->numEntries() == 0) {
        return {0.0, 0.0};
    }

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);

    double val = 0.0;
    double rawError = 0.0;
    if (extract_with_true) {
    RooGenericPdf model("model", "1 + TrueS_T * TrueDepol2 * (1/TrueDepol1) * tPol * Spin_idx * A * sin(TruePhiRperp+TruePhiS)",
                 RooArgList(*loopResult.variables->TrueST,
                                        *TrueDepol2,
                                        *TrueDepol1,
                                        *loopResult.variables->TargetPolarization,
                                        *loopResult.variables->SpinIdx,
                                        A_fit,
                                        *TruePhiRperp,
                                        *loopResult.variables->TruePhiS));
        std::unique_ptr<RooFitResult> fitResult(model.fitTo(*loopResult.dataSet, Save(), PrintLevel(-1), SumW2Error(kFALSE)));
        (void)fitResult;
        val = A_fit.getVal();
        rawError = A_fit.getError();
    } else {
        RooGenericPdf model("model", "1 + S_T * Depol2 * (1/Depol1) * tPol * Spin_idx * A * sin(PhiRperp+PhiS)",
                             RooArgList(*loopResult.variables->ST,
                                        *Depol2,
                                        *Depol1,
                                        *loopResult.variables->TargetPolarization,
                                        *loopResult.variables->SpinIdx,
                                        A_fit,
                                        *PhiRperp,
                                        *loopResult.variables->PhiS));
        std::unique_ptr<RooFitResult> fitResult(model.fitTo(*loopResult.dataSet, Save(), PrintLevel(-1), SumW2Error(kFALSE)));
        (void)fitResult;
        val = A_fit.getVal();
        rawError = A_fit.getError();
    }

    double scaledError = 0.0;
    if (loopResult.stats.expectedEvents > 0.0 && loopResult.stats.effectiveMCEvents > 0.0) {
        scaledError = rawError * std::sqrt(loopResult.stats.effectiveMCEvents / loopResult.stats.expectedEvents);
    }

    const double sumW = loopResult.stats.sumW;
    const double eff_inj_tasym = (sumW != 0.0) ? loopResult.stats.sumTrueAsymW / sumW : 0.0;
    const double eff_inj_rasym = (sumW != 0.0) ? loopResult.stats.sumRecoAsymW / sumW : 0.0;

    std::cout << "======================== Asymmetry Results ========================\n" << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << " bool extract_with_true = " << extract_with_true << std::endl;
    std::cout << " ------------------------------------------------------------------" << std::endl;
    std::cout << " Asymmetry Extracted = " << val << " +/- " << rawError << std::endl;
    std::cout << " Asymmetry Extracted (w/ scaled EIC errors) = " << val << " +/- " << scaledError << std::endl;
    std::cout << " Effective Truth Injected Asymmetry = " << eff_inj_tasym << std::endl;
    std::cout << " Effective Reco Injected Asymmetry = " << eff_inj_rasym << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

    return {val, scaledError};
}
