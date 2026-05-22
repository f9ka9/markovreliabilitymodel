#include "calculationresultexporter.h"

#include <QJsonObject>

QJsonDocument CalculationResultExporter::toJson(const CalculationResult& result)
{
    QJsonObject root;
    root["version"] = 1;

    QJsonArray nodes;
    for (const SchemaNodeData& node : result.nodes)
    {
        QJsonObject object;
        object["id"] = node.id;
        object["name"] = node.name;
        object["groupName"] = node.groupName;
        nodes.append(object);
    }
    root["nodes"] = nodes;

    QJsonArray states;
    for (const ReliabilityState& state : result.states)
    {
        QJsonObject object;
        object["id"] = state.id;
        object["name"] = state.name;
        object["systemState"] = resultSystemStateName(state.systemState);

        QJsonArray failedNodeIds;
        for (int nodeId : state.failedNodeIds)
            failedNodeIds.append(nodeId);
        object["failedNodeIds"] = failedNodeIds;
        states.append(object);
    }
    root["states"] = states;

    QJsonArray transitions;
    for (const ReliabilityTransition& transition : result.transitions)
    {
        QJsonObject object;
        object["sourceStateId"] = transition.sourceStateId;
        object["targetStateId"] = transition.targetStateId;
        object["changedNodeId"] = transition.changedNodeId;
        object["fromState"] = "working";
        object["toState"] = "failed";
        transitions.append(object);
    }
    root["transitions"] = transitions;

    root["initialProbabilities"] = vectorToJson(result.initialProbabilities);

    QJsonArray stages;
    for (const StageCalculationResult& stageResult : result.stages)
    {
        QJsonObject object;
        object["name"] = stageResult.stage.name;
        object["mode"] = operationModeName(stageResult.stage.mode);
        object["duration"] = stageResult.stage.duration;
        object["Q"] = matrixToJson(stageResult.intensityMatrix);
        object["P"] = matrixToJson(stageResult.transitionMatrix);
        object["probabilities"] = vectorToJson(stageResult.probabilities);
        stages.append(object);
    }
    root["stages"] = stages;

    return QJsonDocument(root);
}

QJsonArray CalculationResultExporter::matrixToJson(const ReliabilityMatrix& matrix)
{
    QJsonArray rows;
    for (const QVector<double>& row : matrix)
    {
        QJsonArray values;
        for (double value : row)
            values.append(value);
        rows.append(values);
    }
    return rows;
}

QJsonArray CalculationResultExporter::vectorToJson(const ProbabilityVector& vector)
{
    QJsonArray values;
    for (double value : vector)
        values.append(value);
    return values;
}

QString CalculationResultExporter::operationModeName(OperationMode mode)
{
    switch (mode)
    {
    case OperationMode::Transportation:
        return "transportation";
    case OperationMode::Storage:
        return "storage";
    case OperationMode::Functioning:
        return "functioning";
    case OperationMode::Off:
        return "off";
    }

    return "functioning";
}

QString CalculationResultExporter::resultSystemStateName(ReliabilitySystemState state)
{
    switch (state)
    {
    case ReliabilitySystemState::Working:
        return "working";
    case ReliabilitySystemState::PartialFailure:
        return "partialFailure";
    case ReliabilitySystemState::Failure:
        return "failure";
    }

    return "unknown";
}
