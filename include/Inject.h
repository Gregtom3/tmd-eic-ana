#ifndef INJECT_H
#define INJECT_H

#include "Bin.h"
#include "Grid.h"
#include "TCut.h"
#include "TTree.h"
#include "Table.h"
#include <RooArgSet.h>
#include <RooDataSet.h>
#include <RooRealVar.h>
#include <TRandom3.h>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Inject {
protected:
    struct InjectionStats {
        double expectedEvents{0.0};
        double sumW{0.0};
        double sumW2{0.0};
        double sumTrueAsymW{0.0};
        double sumRecoAsymW{0.0};
        double effectiveMCEvents{0.0};
        long long selectedEvents{0};
        double sumXW{0.0};
        double sumQW{0.0};
        double sumQ2W{0.0};
        double sumZW{0.0};
        double sumPhPerpW{0.0};
        double sumYW{0.0};
    };

public:
    // Base interface for injection classes
    Inject(TTree* tree, const Table* table, double scale = 1.0, double targetPolarization = 1.0)
        : tree(tree), table(table), m_scale(scale), targetPolarization(targetPolarization) {}
    virtual ~Inject() = default;
    virtual std::pair<double, double> injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt = std::nullopt) = 0;

    const InjectionStats& getLastStats() const { return lastStats; }

protected:
    struct EventRecord {
        float PhiH{0.0};
        float PhiS{0.0};
        float X{0.0};
        float Q2{0.0};
        float Z{0.0};
        float PhPerp{0.0};
        float Y{0.0};
        float TruePhiH{0.0};
        float TruePhiS{0.0};
        float TrueX{0.0};
        float TrueQ2{0.0};
        float TrueY{0.0};
        float TrueZ{0.0};
        float TruePhPerp{0.0};
        float Weight{0.0};
    };

    struct AsymmetryValues {
        double trueAsymmetry{0.0};
        double recoAsymmetry{0.0};
    };

    struct LoopVariables {
        LoopVariables(const Bin& bin, double targetPol);

        RooRealVar* addExtraRealVar(std::unique_ptr<RooRealVar> var);

        std::unique_ptr<RooRealVar> Y;
        std::unique_ptr<RooRealVar> PhiH;
        std::unique_ptr<RooRealVar> PhiS;
        std::unique_ptr<RooRealVar> X;
        std::unique_ptr<RooRealVar> Q2;
        std::unique_ptr<RooRealVar> Z;
        std::unique_ptr<RooRealVar> PhPerp;
        std::unique_ptr<RooRealVar> TruePhiH;
        std::unique_ptr<RooRealVar> TruePhiS;
        std::unique_ptr<RooRealVar> TrueX;
        std::unique_ptr<RooRealVar> TrueQ2;
        std::unique_ptr<RooRealVar> TrueY;
        std::unique_ptr<RooRealVar> TrueZ;
        std::unique_ptr<RooRealVar> TruePhPerp;
        std::unique_ptr<RooRealVar> SpinIdx;
        std::unique_ptr<RooRealVar> Weight;
        std::unique_ptr<RooRealVar> TotalWeight;
        std::unique_ptr<RooRealVar> Gamma;
        std::unique_ptr<RooRealVar> TrueGamma;
        std::unique_ptr<RooRealVar> ST;
        std::unique_ptr<RooRealVar> TrueST;
        std::unique_ptr<RooRealVar> TargetPolarization;
        RooArgSet observables;
        TRandom3 rng;
        std::vector<std::unique_ptr<RooRealVar>> extraReals;
    };

    struct LoopHandlers {
        std::function<void(TTree&)> bindCustomBranches;
        std::function<void(const EventRecord&, LoopVariables&)> updateCustomVariables;
        std::function<AsymmetryValues(const EventRecord&, const LoopVariables&, bool, std::optional<double>)> computeAsymmetries;
        std::function<double(const EventRecord&, const LoopVariables&, const AsymmetryValues&, bool)> computeSpinProbability;
    };

    struct LoopResult {
        std::unique_ptr<RooDataSet> dataSet;
        std::unique_ptr<LoopVariables> variables;
        InjectionStats stats;
    };

    std::unique_ptr<LoopVariables> createLoopVariables(const Bin& bin) const;
    LoopResult runInjectionLoop(const Bin& bin,
                                bool extract_with_true,
                                std::optional<double> A_override,
                                std::unique_ptr<LoopVariables> variables,
                                const LoopHandlers& handlers) const;

    static double computeGamma(double x, double q2);
    static double computeTransverseSpinMag(double gamma, double y, double phiS);

    std::string buildCutString(const Bin& bin, bool useTrueKinematics) const;

    TTree* tree{nullptr};
    const Table* table{nullptr};
    double m_scale{1.0};
    double targetPolarization{1.0};
    InjectionStats lastStats;
};


class InjectHadron : public Inject {
public:
    InjectHadron(TTree* tree, const Table* table, double scale = 1.0, double targetPolarization = 1.0);
    ~InjectHadron();
    std::pair<double, double> injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt = std::nullopt) override;
};

class InjectDihadron : public Inject {
public:
    InjectDihadron(TTree* tree, const Table* table, double scale = 1.0, double targetPolarization = 1.0);
    ~InjectDihadron();
    std::pair<double, double> injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt = std::nullopt) override;
};

#endif // INJECT_H
