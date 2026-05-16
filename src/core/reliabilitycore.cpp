#include "reliabilitycore.h"

#include <cmath>

namespace
{
constexpr double maxUniformizationMean = 16.0;
constexpr double poissonTolerance = 1.0e-14;
constexpr int maxUniformizationIterations = 10000;
constexpr int maxSparseChunks = 65536;

int chunkCountForMean(double mean)
{
    int chunks = 1;
    while (mean / chunks > maxUniformizationMean && chunks < maxSparseChunks)
        chunks *= 2;
    return chunks;
}
}

QHash<int, FailureRates> ReliabilityCore::createFailureRatesByNodeId(const QList<SchemaNodeData>& nodes)
{
    QHash<int, FailureRates> result;
    result.reserve(nodes.size());
    for (const SchemaNodeData& node : nodes)
        result.insert(node.id, node.failureRates);
    return result;
}

double ReliabilityCore::failureRateForNode(const QHash<int, FailureRates>& failureRatesByNodeId, int nodeId, OperationMode mode)
{
    return failureRatesByNodeId.value(nodeId)[static_cast<int>(mode)];
}

QVector<double> ReliabilityCore::transitionRatesForMode(const QList<ReliabilityTransition>& transitions, const QList<SchemaNodeData>& nodes, OperationMode mode)
{
    QVector<double> rates;
    rates.reserve(transitions.size());
    const QHash<int, FailureRates> failureRatesByNodeId = createFailureRatesByNodeId(nodes);

    for (const ReliabilityTransition& transition : transitions)
        rates.append(failureRateForNode(failureRatesByNodeId, transition.changedNodeId, mode));

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

    const double mean = uniformizationRate * duration;
    if (!std::isfinite(mean))
        return probabilities;

    const int chunkCount = chunkCountForMean(mean);
    if (chunkCount > 1)
    {
        ProbabilityVector current = probabilities;
        const double stepDuration = duration / chunkCount;
        for (int chunk = 0; chunk < chunkCount; ++chunk)
            current = advanceProbabilitiesSparse(current, transitions, nodes, mode, stepDuration);
        return current;
    }

    ProbabilityVector result(probabilities.size(), 0.0);
    ProbabilityVector power = probabilities;
    double poissonWeight = qExp(-mean);

    for (int i = 0; i < result.size(); ++i)
        result[i] += power[i] * poissonWeight;

    for (int iteration = 1; iteration <= maxUniformizationIterations; ++iteration)
    {
        power = multiplyByEmbeddedMatrix(power, transitions, transitionRates, exitRates, uniformizationRate);
        poissonWeight *= mean / iteration;

        for (int i = 0; i < result.size(); ++i)
            result[i] += power[i] * poissonWeight;

        if (iteration > mean && poissonWeight < poissonTolerance)
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
