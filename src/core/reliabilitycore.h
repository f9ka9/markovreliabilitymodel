#ifndef RELIABILITYCORE_H
#define RELIABILITYCORE_H
#include <QHash>
#include <QtMath>

#include "calculationresult.h"
#include "matrixbuilder.h"
#include "probabilitycalculator.h"
#include "schemamodel.h"
#include "stategenerator.h"
#include "transitionbuilder.h"

class ReliabilityCore
{
public:
    CalculationResult calculateCurrentLevel(const SchemaModel& model, StructureType structureType, int requiredElements, OperationMode mode) const;
    CalculationResult calculateCyclogram(const SchemaModel& model, StructureType structureType, int requiredElements, const Cyclogram& cyclogram, const ProbabilityVector& initialProbabilities = {}) const;

private:
    static constexpr int maxDenseMatrixStateCount = 250;

    static QHash<int, FailureRates> createFailureRatesByNodeId(const QList<SchemaNodeData>& nodes);
    static double failureRateForNode(const QHash<int, FailureRates>& failureRatesByNodeId, int nodeId, OperationMode mode);
    static QVector<double> transitionRatesForMode(const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode);
    static QVector<double> exitRatesByState(const QList<ReliabilityTransition>& transitions, const QVector<double>& transitionRates, int stateCount);
    static double maxExitRate(const QVector<double>& exitRates);
    static ProbabilityVector multiplyByEmbeddedMatrix(const ProbabilityVector& probabilities, const QList<ReliabilityTransition>& transitions, const QVector<double>& transitionRates, const QVector<double>& exitRates, double uniformizationRate);
    static ProbabilityVector advanceProbabilitiesSparse(const ProbabilityVector& probabilities, const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode, double duration);
    StateGenerator stateGenerator;
    TransitionBuilder transitionBuilder;
    MatrixBuilder matrixBuilder;
    ProbabilityCalculator probabilityCalculator;
};

#endif // RELIABILITYCORE_H
