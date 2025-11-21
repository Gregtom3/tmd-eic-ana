#include "Inject.h"
#include <RooArgList.h>
#include <RooFit.h>
#include <RooFitResult.h>
#include <RooFormulaVar.h>
#include <RooGenericPdf.h>
#include <cmath>
#include <iostream>

using namespace RooFit;

InjectHadron::InjectHadron(TTree* tree, const Table* table, double scale, double targetPolarization, channel_type channel)
    : Inject(tree, table, scale, targetPolarization, channel) {}

InjectHadron::~InjectHadron() = default;

std::pair<double, double> InjectHadron::injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt) {
    LOG_DEBUG("InjectHadron::injectExtractForBin: Starting injection/extraction for bin.");
    auto variables = createLoopVariables(bin);
    LoopHandlers handlers;
    LOG_DEBUG("InjectHadron::injectExtractForBin: Setting up loop handlers.");
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
            values.trueAsymmetry = table->lookupAUT_SIDIS(record.TrueX, trueQ, record.TrueZ, record.TruePhPerp);
            values.recoAsymmetry = table->lookupAUT_SIDIS(record.X, recoQ, record.Z, record.PhPerp);
        }
        return values;
    };
    LOG_DEBUG("InjectHadron::injectExtractForBin: Setting up spin probability handler.");
    handlers.computeSpinProbability = [&](const EventRecord& record, const LoopVariables& vars, const AsymmetryValues& asym, bool) {
        const double denom = 1.0 - record.TrueY + 0.5 * record.TrueY * record.TrueY;
        double trueDepol1 = 0.0;
        if (std::abs(denom) > 1e-12) {
            trueDepol1 = (1.0 - record.TrueY) / denom;
        }
        const double trueST = vars.TrueST->getVal();
        const double phase = record.TruePhiH + record.TruePhiS;
        return 0.5 * (1.0 + trueST * trueDepol1 * asym.trueAsymmetry * std::sin(phase));
    };
    LOG_DEBUG("InjectHadron::injectExtractForBin: Running injection loop.");
    auto loopResult = runInjectionLoop(bin, extract_with_true, A_opt, std::move(variables), handlers);
    LOG_DEBUG("InjectHadron::injectExtractForBin: Finished injection loop.");
    lastStats = loopResult.stats;
    if (!loopResult.dataSet || loopResult.dataSet->numEntries() == 0) {
        return {0.0, 0.0};
    }

    RooFormulaVar Depol1("Depol1", "(1 - Y)/(1 - Y + 0.5 * Y * Y)", RooArgList(*loopResult.variables->Y));
    RooFormulaVar TrueDepol1("TrueDepol1", "(1 - TrueY)/(1 - TrueY + 0.5 * TrueY * TrueY)", RooArgList(*loopResult.variables->TrueY));

    RooRealVar A_fit("A", "A", 0.0, -1.0, 1.0);

    double val = 0.0;
    double rawError = 0.0;
    LOG_DEBUG("InjectHadron::injectExtractForBin: Setting up fitting model.");
    if (extract_with_true) {
    RooGenericPdf model("model", "1 + TrueS_T * TrueDepol1 * tPol * Spin_idx * A * sin(TruePhiH+TruePhiS)",
                 RooArgList(*loopResult.variables->TrueST,
                                        TrueDepol1,
                                        *loopResult.variables->TargetPolarization,
                                        *loopResult.variables->SpinIdx,
                                        A_fit,
                                        *loopResult.variables->TruePhiH,
                                        *loopResult.variables->TruePhiS));
        std::unique_ptr<RooFitResult> fitResult(model.fitTo(*loopResult.dataSet, Save(), PrintLevel(-1), SumW2Error(kFALSE)));
        (void)fitResult; // suppress unused warning when compiled without RTTI
        val = A_fit.getVal();
        rawError = A_fit.getError();
    } else {
        RooGenericPdf model("model", "1 + S_T * Depol1 * tPol * Spin_idx * A * sin(PhiH+PhiS)",
                             RooArgList(*loopResult.variables->ST,
                                        Depol1,
                                        *loopResult.variables->TargetPolarization,
                                        *loopResult.variables->SpinIdx,
                                        A_fit,
                                        *loopResult.variables->PhiH,
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
