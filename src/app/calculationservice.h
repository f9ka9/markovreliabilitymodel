#ifndef CALCULATIONSERVICE_H
#define CALCULATIONSERVICE_H

#include <QString>

#include "calculationresult.h"
#include "editorschemamodel.h"
#include "schemamodel.h"

struct CalculationPreparation
{
    bool ok{false};
    QString errorMessage;
    SchemaModel model;
    Cyclogram cyclogram;
    int stateCount{0};
};

struct CalculationRun
{
    bool ok{false};
    QString errorMessage;
    CalculationResult result;
};

class CalculationService
{
public:
    explicit CalculationService(int maxStateCountForCalculation);

    CalculationPreparation prepare(const EditorSchemaModel& editorModel, const Cyclogram& cyclogram) const;
    CalculationRun calculate(const CalculationPreparation& preparation, const ProbabilityVector& initialProbabilities) const;

private:
    static int reliabilityNodeCount(const QList<SchemaNodeData>& nodes);
    bool validateModelAndCyclogram(const SchemaModel& model, const Cyclogram& cyclogram, int& stateCount, QString& errorMessage) const;
    static bool validateInitialProbabilities(const ProbabilityVector& probabilities, int stateCount, QString& errorMessage);

    int maxStateCountForCalculation{0};
};

#endif // CALCULATIONSERVICE_H
