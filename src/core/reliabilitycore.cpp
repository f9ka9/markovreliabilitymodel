#include "reliabilitycore.h"

double ReliabilityCore::failureRateForNode(const QList<SchemaNodeData>& nodes, int nodeId, OperationMode mode)
{
    for (const SchemaNodeData& node : nodes)
    {
        if (node.id == nodeId)
            return node.failureRates[static_cast<int>(mode)];
    }

    return 0.0;
}

QVector<double> ReliabilityCore::transitionRatesForMode(const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode)
{
    QVector<double> rates;
    rates.reserve(transitions.size());

    for (const ReliabilityTransition& transition : transitions)
        rates.append(failureRateForNode(nodes, transition.changedNodeId, mode));

    return rates;
}

QVector<double> ReliabilityCore::exitRatesByState(const QList<ReliabilityTransition>& transitions, const QVector<double>& transitionRates, int stateCount)
{
    QVector<double> exitRates(stateCount, 0.0);
    for (int i = 0; i < transitions.size() && i < transitionRates.size(); ++i)
    {
        const int source = transitions[i].sourceStateId;
        if (source >= 0 && source < exitRates.size())
            exitRates[source] += transitionRates[i];
    }

    return exitRates;
}

double ReliabilityCore::maxExitRate(const QVector<double>& exitRates)
{
    double result = 0.0;
    for (double rate : exitRates)
        result = qMax(result, rate);
    return result;
}

ProbabilityVector ReliabilityCore::multiplyByEmbeddedMatrix(const ProbabilityVector& probabilities, const QList<ReliabilityTransition>& transitions, const QVector<double>& transitionRates, const QVector<double>& exitRates, double uniformizationRate)
{
    ProbabilityVector result(probabilities.size(), 0.0);
    for (int state = 0; state < probabilities.size(); ++state)
        result[state] += probabilities[state] * (1.0 - exitRates[state] / uniformizationRate);

    for (int i = 0; i < transitions.size() && i < transitionRates.size(); ++i)
    {
        const ReliabilityTransition& transition = transitions[i];
        if (transition.sourceStateId < 0 || transition.sourceStateId >= probabilities.size() || transition.targetStateId < 0 || transition.targetStateId >= probabilities.size())
            continue;

        result[transition.targetStateId] += probabilities[transition.sourceStateId] * transitionRates[i] / uniformizationRate;
    }

    return result;
}

ProbabilityVector ReliabilityCore::advanceProbabilitiesSparse(const ProbabilityVector& probabilities, const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode, double duration)
{
    if (probabilities.isEmpty() || duration <= 0.0)
        return probabilities;

    const QVector<double> transitionRates = transitionRatesForMode(transitions, nodes, mode);
    const QVector<double> exitRates = exitRatesByState(transitions, transitionRates, probabilities.size());
    const double uniformizationRate = maxExitRate(exitRates);
    if (qFuzzyIsNull(uniformizationRate))
        return probabilities;

    ProbabilityVector result(probabilities.size(), 0.0);
    ProbabilityVector power = probabilities;
    const double mean = uniformizationRate * duration;
    double poissonWeight = qExp(-mean);

    for (int i = 0; i < result.size(); ++i)
        result[i] += power[i] * poissonWeight;

    constexpr int maxIterations = 10000;
    constexpr double tolerance = 1.0e-14;
    for (int iteration = 1; iteration <= maxIterations; ++iteration)
    {
        power = multiplyByEmbeddedMatrix(power, transitions, transitionRates, exitRates, uniformizationRate);
        poissonWeight *= mean / iteration;

        for (int i = 0; i < result.size(); ++i)
            result[i] += power[i] * poissonWeight;

        if (poissonWeight < tolerance)
            break;
    }

    return result;
}

CalculationResult ReliabilityCore::calculateCurrentLevel(const SchemaModel& model, StructureType structureType, int requiredElements, OperationMode mode) const
{
    Cyclogram cyclogram;
    cyclogram.append({"Functioning", mode, 1.0});
    return calculateCyclogram(model, structureType, requiredElements, cyclogram);
}

CalculationResult ReliabilityCore::calculateCyclogram(const SchemaModel& model, StructureType structureType, int requiredElements, const Cyclogram& cyclogram, const ProbabilityVector& initialProbabilities) const
{
    CalculationResult result;

    result.nodes = model.nodes;
    result.states = stateGenerator.generate(model.nodes, model.connections, structureType, requiredElements);
    result.transitions = transitionBuilder.build(result.states);
    result.initialProbabilities = probabilityCalculator.normalizedInitialProbabilities(initialProbabilities, result.states.size());

    ProbabilityVector currentProbabilities = result.initialProbabilities;
    for (const CyclogramStage& stage : cyclogram)
    {
        StageCalculationResult stageResult;
        stageResult.stage = stage;
        if (result.states.size() <= maxDenseMatrixStateCount)
        {
            stageResult.intensityMatrix = matrixBuilder.buildIntensityMatrix(result.states, result.transitions, model.nodes, stage.mode);
            stageResult.transitionMatrix = probabilityCalculator.buildTransitionMatrix(stageResult.intensityMatrix, stage.duration);
            currentProbabilities = probabilityCalculator.multiply(currentProbabilities, stageResult.transitionMatrix);
        }
        else
        {
            currentProbabilities = advanceProbabilitiesSparse(currentProbabilities, result.transitions, model.nodes, stage.mode, stage.duration);
        }
        stageResult.probabilities = currentProbabilities;
        result.stages.append(stageResult);
    }

    if (!result.stages.isEmpty())
    {
        result.intensityMatrix = result.stages.first().intensityMatrix;
        result.transitionMatrix = result.stages.first().transitionMatrix;
    }

    return result;
}
