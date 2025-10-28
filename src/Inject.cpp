#include "Inject.h"
#include "Logger.h"
#include <TMath.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace {
constexpr double kProtonMass = 0.938272;
constexpr double kPiRange = 2.0 * TMath::Pi();

inline double clampProbability(double value) {
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}
}

Inject::LoopVariables::LoopVariables(const Bin& bin, double targetPol)
    : Y(std::make_unique<RooRealVar>("Y", "Y", 0.0, 1.0)),
      PhiH(std::make_unique<RooRealVar>("PhiH", "PhiH", -kPiRange, kPiRange)),
      PhiS(std::make_unique<RooRealVar>("PhiS", "PhiS", -kPiRange, kPiRange)),
      X(std::make_unique<RooRealVar>("X", "X", bin.getMin("X"), bin.getMax("X"))),
      Q2(std::make_unique<RooRealVar>("Q2", "Q2", bin.getMin("Q") * bin.getMin("Q"), bin.getMax("Q") * bin.getMax("Q"))),
      Z(std::make_unique<RooRealVar>("Z", "Z", bin.getMin("Z"), bin.getMax("Z"))),
      PhPerp(std::make_unique<RooRealVar>("PhPerp", "PhPerp", bin.getMin("PhPerp"), bin.getMax("PhPerp"))),
      TruePhiH(std::make_unique<RooRealVar>("TruePhiH", "TruePhiH", -kPiRange, kPiRange)),
      TruePhiS(std::make_unique<RooRealVar>("TruePhiS", "TruePhiS", -kPiRange, kPiRange)),
      TrueX(std::make_unique<RooRealVar>("TrueX", "TrueX", -999.0, 999.0)),
      TrueQ2(std::make_unique<RooRealVar>("TrueQ2", "TrueQ2", -999.0, 9.9999999e7)),
      TrueY(std::make_unique<RooRealVar>("TrueY", "TrueY", -999.0, 999.0)),
      TrueZ(std::make_unique<RooRealVar>("TrueZ", "TrueZ", -999.0, 999.0)),
      TruePhPerp(std::make_unique<RooRealVar>("TruePhPerp", "TruePhPerp", -999.0, 999.0)),
      SpinIdx(std::make_unique<RooRealVar>("Spin_idx", "Spin_idx", -1.0, 1.0)),
      Weight(std::make_unique<RooRealVar>("Weight", "Weight", 0.0, 1e9)),
      TotalWeight(std::make_unique<RooRealVar>("TotalWeight", "TotalWeight", 0.0, 1e9)),
      Gamma(std::make_unique<RooRealVar>("Gamma", "Gamma (2*X*M/sqrt(Q2))", -999.0, 999.0)),
      TrueGamma(std::make_unique<RooRealVar>("TrueGamma", "TrueGamma (2*TrueX*M/sqrt(TrueQ2))", -999.0, 999.0)),
      ST(std::make_unique<RooRealVar>("S_T", "Transverse spin magnitude S_T", -999.0, 999.0)),
      TrueST(std::make_unique<RooRealVar>("TrueS_T", "Transverse spin magnitude TrueS_T", -999.0, 999.0)),
    TargetPolarization(std::make_unique<RooRealVar>("tPol", "Target Polarization", targetPol)),
    observables("observables"),
      rng(0) {
    TargetPolarization->setConstant(true);

    observables.add(*PhiH);
    observables.add(*PhiS);
    observables.add(*X);
    observables.add(*Q2);
    observables.add(*Z);
    observables.add(*Y);
    observables.add(*PhPerp);
    observables.add(*TruePhiH);
    observables.add(*TruePhiS);
    observables.add(*TrueX);
    observables.add(*TrueQ2);
    observables.add(*TrueY);
    observables.add(*TrueZ);
    observables.add(*TruePhPerp);
    observables.add(*SpinIdx);
    observables.add(*Weight);
    observables.add(*ST);
    observables.add(*TrueST);
}

RooRealVar* Inject::LoopVariables::addExtraRealVar(std::unique_ptr<RooRealVar> var) {
    if (!var) {
        return nullptr;
    }
    RooRealVar* ptr = var.get();
    extraReals.push_back(std::move(var));
    observables.add(*ptr);
    return ptr;
}

std::unique_ptr<Inject::LoopVariables> Inject::createLoopVariables(const Bin& bin) const {
    return std::make_unique<LoopVariables>(bin, targetPolarization);
}

double Inject::computeGamma(double x, double q2) {
    if (q2 <= 0.0) {
        return 0.0;
    }
    return 2.0 * x * kProtonMass / std::sqrt(q2);
}

double Inject::computeTransverseSpinMag(double gamma, double y, double phiS) {
    const double denomBase = 1.0 + gamma * gamma;
    if (denomBase <= 0.0) {
        return 0.0;
    }
    double inner = (1.0 - y - 0.25 * y * y * gamma * gamma) / denomBase;
    if (inner < 0.0) {
        inner = 0.0;
    }
    double sinTheta = gamma * std::sqrt(inner);
    if (sinTheta > 1.0) {
        sinTheta = 1.0;
    }
    double cosTheta = std::sqrt(std::max(0.0, 1.0 - sinTheta * sinTheta));
    double denom = std::sqrt(std::max(1e-12, 1.0 - sinTheta * sinTheta * std::sin(phiS) * std::sin(phiS)));
    double ST_val = cosTheta / denom;
    if (!std::isfinite(ST_val)) {
        return 0.0;
    }
    return ST_val;
}

std::string Inject::buildCutString(const Bin& bin, bool useTrueKinematics) const {
    std::ostringstream oss;
    const std::string prefix = useTrueKinematics ? "True" : "";
    oss << prefix << "X >= " << bin.getMin("X") << " && " << prefix << "X <= " << bin.getMax("X")
        << " && " << prefix << "Q2 >= " << bin.getMin("Q") * bin.getMin("Q")
        << " && " << prefix << "Q2 <= " << bin.getMax("Q") * bin.getMax("Q")
        << " && " << prefix << "Z >= " << bin.getMin("Z") << " && " << prefix << "Z <= " << bin.getMax("Z")
        << " && " << prefix << "PhPerp >= " << bin.getMin("PhPerp")
        << " && " << prefix << "PhPerp <= " << bin.getMax("PhPerp");
    return oss.str();
}

Inject::LoopResult Inject::runInjectionLoop(const Bin& bin,
                                            bool extract_with_true,
                                            std::optional<double> A_override,
                                            std::unique_ptr<LoopVariables> variables,
                                            const LoopHandlers& handlers) const {
    LoopResult result;
    result.variables = std::move(variables);
    if (!result.variables) {
        return result;
    }

    auto& vars = *result.variables;
    result.stats = {};

    if (!tree) {
        std::cerr << "[Inject::runInjectionLoop] Error: TTree pointer is null." << std::endl;
        return result;
    }

    // Prepare branch buffers for common quantities
    double b_PhiH = 0.0, b_PhiS = 0.0, b_X = 0.0, b_Q2 = 0.0, b_Z = 0.0, b_PhPerp = 0.0;
    double b_TruePhiH = 0.0, b_TruePhiS = 0.0, b_TrueX = 0.0, b_TrueQ2 = 0.0, b_TrueY = 0.0;
    double b_TrueZ = 0.0, b_TruePhPerp = 0.0;
    double b_Weight = 0.0, b_Y = 0.0;

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

    if (handlers.bindCustomBranches) {
        handlers.bindCustomBranches(*tree);
    }

    auto dataSet = std::make_unique<RooDataSet>("dataUpdate", "data with updated spin", vars.observables, RooFit::WeightVar(*vars.TotalWeight));

    const double minX = bin.getMin("X");
    const double maxX = bin.getMax("X");
    const double minQ2 = bin.getMin("Q") * bin.getMin("Q");
    const double maxQ2 = bin.getMax("Q") * bin.getMax("Q");
    const double minZ = bin.getMin("Z");
    const double maxZ = bin.getMax("Z");
    const double minPhPerp = bin.getMin("PhPerp");
    const double maxPhPerp = bin.getMax("PhPerp");

    EventRecord record;

    const Long64_t nentries = tree->GetEntries();
    const Long64_t progress_step = std::min<Long64_t>(100, std::max<Long64_t>(static_cast<Long64_t>(1), nentries / 100));
    Long64_t next_progress = progress_step;

    for (Long64_t i = 0; i < nentries; ++i) {
        tree->GetEntry(i);

        if (i >= next_progress || i == 0 || i == nentries - 1) {
            const Long64_t denomEntries = std::max<Long64_t>(static_cast<Long64_t>(1), nentries);
            int percent = static_cast<int>(100.0 * static_cast<double>(i + 1) / static_cast<double>(denomEntries));
            std::cout << "\r[" << percent << "%] Processing entry " << (i + 1) << " / " << nentries << std::flush;
            next_progress = i + progress_step;
            if (i == nentries - 1) {
                std::cout << std::endl;
            }
        }

        record.PhiH = b_PhiH;
        record.PhiS = b_PhiS;
        record.X = b_X;
        record.Q2 = b_Q2;
        record.Z = b_Z;
        record.PhPerp = b_PhPerp;
        record.Y = b_Y;
        record.TruePhiH = b_TruePhiH;
        record.TruePhiS = b_TruePhiS;
        record.TrueX = b_TrueX;
        record.TrueQ2 = b_TrueQ2;
        record.TrueY = b_TrueY;
        record.TrueZ = b_TrueZ;
        record.TruePhPerp = b_TruePhPerp;
        record.Weight = b_Weight;

        bool passesCuts = false;
        if (extract_with_true) {
            passesCuts = (record.TrueX >= minX && record.TrueX <= maxX &&
                          record.TrueQ2 >= minQ2 && record.TrueQ2 <= maxQ2 &&
                          record.TrueZ >= minZ && record.TrueZ <= maxZ &&
                          record.TruePhPerp >= minPhPerp && record.TruePhPerp <= maxPhPerp);
        } else {
            passesCuts = (record.X >= minX && record.X <= maxX &&
                          record.Q2 >= minQ2 && record.Q2 <= maxQ2 &&
                          record.Z >= minZ && record.Z <= maxZ &&
                          record.PhPerp >= minPhPerp && record.PhPerp <= maxPhPerp);
        }

        if (!passesCuts) {
            continue;
        }

        ++result.stats.selectedEvents;

        result.stats.sumXW += record.X * record.Weight;
        result.stats.sumQW += std::sqrt(record.Q2) * record.Weight;
        result.stats.sumQ2W += record.Q2 * record.Weight;
        result.stats.sumZW += record.Z * record.Weight;
        result.stats.sumPhPerpW += record.PhPerp * record.Weight;

        vars.PhiH->setVal(record.PhiH);
        vars.PhiS->setVal(record.PhiS);
        vars.X->setVal(record.X);
        vars.Q2->setVal(record.Q2);
        vars.Z->setVal(record.Z);
        vars.PhPerp->setVal(record.PhPerp);
        vars.Y->setVal(record.Y);
        vars.TruePhiH->setVal(record.TruePhiH);
        vars.TruePhiS->setVal(record.TruePhiS);
        vars.TrueX->setVal(record.TrueX);
        vars.TrueQ2->setVal(record.TrueQ2);
        vars.TrueY->setVal(record.TrueY);
        vars.TrueZ->setVal(record.TrueZ);
        vars.TruePhPerp->setVal(record.TruePhPerp);

        const double gamma = computeGamma(record.X, record.Q2);
        const double trueGamma = computeGamma(record.TrueX, record.TrueQ2);
        vars.Gamma->setVal(gamma);
        vars.TrueGamma->setVal(trueGamma);

        const double ST_val = computeTransverseSpinMag(gamma, record.Y, record.PhiS);
        const double TrueST_val = computeTransverseSpinMag(trueGamma, record.TrueY, record.TruePhiS);
        vars.ST->setVal(ST_val);
        vars.TrueST->setVal(TrueST_val); 
        if (TrueST_val < 0.0) {
            LOG_DEBUG("Warning: TrueS_T < 0: " + std::to_string(TrueST_val));
        }

        if (handlers.updateCustomVariables) {
            handlers.updateCustomVariables(record, vars);
        }

        AsymmetryValues asym;
        if (handlers.computeAsymmetries) {
            asym = handlers.computeAsymmetries(record, vars, extract_with_true, A_override);
        }

        double pPlus = 0.5;
        if (handlers.computeSpinProbability) {
            pPlus = handlers.computeSpinProbability(record, vars, asym, extract_with_true);
        }
        pPlus = clampProbability(pPlus);

        double spin = (vars.rng.Rndm() < pPlus) ? 1.0 : -1.0;
        if (vars.rng.Rndm() > targetPolarization) {
            spin = (vars.rng.Rndm() < 0.5) ? 1.0 : -1.0;
        }
        vars.SpinIdx->setVal(spin);

        vars.Weight->setVal(record.Weight);
        vars.TotalWeight->setVal(record.Weight * m_scale);

        dataSet->add(vars.observables);

        result.stats.expectedEvents += vars.TotalWeight->getVal();
        result.stats.sumW += record.Weight;
        result.stats.sumW2 += record.Weight * record.Weight;
        result.stats.sumTrueAsymW += record.Weight * asym.trueAsymmetry;
        result.stats.sumRecoAsymW += record.Weight * asym.recoAsymmetry;
    }

    if (nentries == 0) {
        std::cout << "[Inject::runInjectionLoop] Input tree contains no entries." << std::endl;
    }

    result.stats.effectiveMCEvents = (result.stats.sumW2 > 0.0)
        ? (result.stats.sumW * result.stats.sumW) / result.stats.sumW2
        : 0.0;

    bin.setEvents(dataSet->numEntries());
    bin.setExpectedEvents(static_cast<int>(std::round(result.stats.expectedEvents)));

    std::cout << "[Inject::runInjectionLoop] Selected " << result.stats.selectedEvents
              << " events for injection (after tree loop)." << std::endl;

    result.dataSet = std::move(dataSet);
    return result;
}
