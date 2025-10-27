#ifndef INJECT_H
#define INJECT_H

#include "Bin.h"
#include "Grid.h"
#include "TCut.h"
#include "TTree.h"
#include "Table.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>

class Inject {
public:
    // Base interface for injection classes
    Inject(TTree* tree, const Table* table, double scale = 1.0, double targetPolarization = 1.0)
        : tree(tree), table(table), m_scale(scale), targetPolarization(targetPolarization) {}
    virtual ~Inject() = default;
    virtual std::pair<double, double> injectExtractForBin(const Bin& bin, bool extract_with_true, std::optional<double> A_opt = std::nullopt) = 0;

protected:
    TTree* tree{nullptr};
    const Table* table{nullptr};
    double m_scale{1.0};
    double targetPolarization{1.0};
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
