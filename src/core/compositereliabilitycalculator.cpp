#include "compositereliabilitycalculator.h"

#include <cmath>

#include <QtMath>
#include <QSet>

#include "reliabilitycore.h"

SchemaModel CompositeReliabilityCalculator::buildEffectiveModel(const EditorSchemaModel& editorModel, int parentId) const
{
    effectiveFailureRateCache.clear();
    const QHash<int, EditorSchemaNodeData> nodesById = createNodesById(editorModel);
    const QHash<int, QList<EditorSchemaNodeData>> childrenByParentId = createChildrenByParentId(editorModel);
    return buildEffectiveModel(editorModel, parentId, nodesById, childrenByParentId);
}

QHash<int, EditorSchemaNodeData> CompositeReliabilityCalculator::createNodesById(const EditorSchemaModel& editorModel) const
{
    QHash<int, EditorSchemaNodeData> result;
    for (const EditorSchemaNodeData& node : editorModel.nodes)
        result.insert(node.id, node);
    return result;
}

QHash<int, QList<EditorSchemaNodeData>> CompositeReliabilityCalculator::createChildrenByParentId(const EditorSchemaModel& editorModel) const
{
    QHash<int, QList<EditorSchemaNodeData>> result;
    for (const EditorSchemaNodeData& node : editorModel.nodes)
        result[node.parentId].append(node);
    return result;
}

SchemaModel CompositeReliabilityCalculator::buildEffectiveModel(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    SchemaModel model;
    model.currentParentId = parentId;
    model.structureType = structureTypeForParent(editorModel, parentId, nodesById);
    model.requiredElements = requiredElementsForParent(editorModel, parentId, nodesById);

    const QList<EditorSchemaNodeData> children = childrenByParentId.value(parentId);
    for (const EditorSchemaNodeData& node : children)
        model.nodes.append(createEffectiveNodeData(editorModel, node, 1, nodesById, childrenByParentId));

    model.connections = directConnections(editorModel, parentId, childrenByParentId);
    return model;
}

SchemaNodeData CompositeReliabilityCalculator::createEffectiveNodeData(const EditorSchemaModel& editorModel, const EditorSchemaNodeData& node, int depth, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    SchemaNodeData nodeData;
    nodeData.id = node.id;
    nodeData.parentId = node.parentId;
    nodeData.depth = depth;
    nodeData.name = node.configuration.name;
    nodeData.groupName = node.configuration.groupName;
    nodeData.nodeKind = node.configuration.nodeKind;
    nodeData.failureRates = resolvedFailureRates(node.configuration.lambdaDefinitions);
    nodeData.structureType = node.configuration.structureType;
    nodeData.requiredElements = qMax(1, node.configuration.requiredElements);

    if (node.configuration.nodeKind != NodeKind::Normal)
    {
        nodeData.failureRates = FailureRates{};
        nodeData.structureType = StructureType::Element;
        nodeData.requiredElements = 1;
        return nodeData;
    }

    if (node.configuration.structureType != StructureType::Element && hasChildren(node.id, childrenByParentId))
    {
        nodeData.failureRates = effectiveFailureRates(editorModel, node.id, nodesById, childrenByParentId);
        nodeData.structureType = StructureType::Element;
        nodeData.requiredElements = 1;
    }

    return nodeData;
}

QList<SchemaConnectionData> CompositeReliabilityCalculator::directConnections(const EditorSchemaModel& editorModel, int parentId, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    QSet<int> childIds;
    for (const EditorSchemaNodeData& child : childrenByParentId.value(parentId))
        childIds.insert(child.id);

    QList<SchemaConnectionData> result;
    for (const EditorSchemaConnectionData& connection : editorModel.connections)
    {
        if (!childIds.contains(connection.sourceNodeId) || !childIds.contains(connection.targetNodeId))
            continue;

        SchemaConnectionData connectionData;
        connectionData.sourceNodeId = connection.sourceNodeId;
        connectionData.targetNodeId = connection.targetNodeId;
        result.append(connectionData);
    }
    return result;
}

FailureRates CompositeReliabilityCalculator::effectiveFailureRates(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    FailureRates result{};
    for (int i = 0; i < static_cast<int>(result.size()); ++i)
        result[i] = effectiveFailureRate(editorModel, parentId, static_cast<OperationMode>(i), nodesById, childrenByParentId);
    return result;
}

double CompositeReliabilityCalculator::effectiveFailureRate(const EditorSchemaModel& editorModel, int parentId, OperationMode mode, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    const QString cacheKey = QString("%1:%2").arg(parentId).arg(static_cast<int>(mode));
    if (effectiveFailureRateCache.contains(cacheKey))
        return effectiveFailureRateCache.value(cacheKey);

    const SchemaModel childModel = buildEffectiveModel(editorModel, parentId, nodesById, childrenByParentId);
    if (childModel.nodes.isEmpty())
    {
        effectiveFailureRateCache.insert(cacheKey, 0.0);
        return 0.0;
    }

    Cyclogram cyclogram;
    cyclogram.append({"Composite", mode, 1.0});

    ReliabilityCore core;
    const CalculationResult result = core.calculateCyclogram(childModel, childModel.structureType, childModel.requiredElements, cyclogram);
    const double probability = qBound(1.0e-15, workProbability(result), 1.0);
    if (probability >= 1.0)
    {
        effectiveFailureRateCache.insert(cacheKey, 0.0);
        return 0.0;
    }

    const double lambda = -std::log(probability);
    effectiveFailureRateCache.insert(cacheKey, lambda);
    return lambda;
}

double CompositeReliabilityCalculator::workProbability(const CalculationResult& result) const
{
    ProbabilityVector probabilities = result.initialProbabilities;
    if (!result.stages.isEmpty())
        probabilities = result.stages.last().probabilities;

    double value = 0.0;
    for (int i = 0; i < result.states.size() && i < probabilities.size(); ++i)
    {
        if (result.states[i].systemState != ReliabilitySystemState::Failure)
            value += probabilities[i];
    }
    return value;
}

bool CompositeReliabilityCalculator::hasChildren(int nodeId, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const
{
    return !childrenByParentId.value(nodeId).isEmpty();
}

StructureType CompositeReliabilityCalculator::structureTypeForParent(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById) const
{
    if (parentId == 0)
        return editorModel.topLevelStructureType;
    return nodesById.value(parentId).configuration.structureType;
}

int CompositeReliabilityCalculator::requiredElementsForParent(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById) const
{
    if (parentId == 0)
        return qMax(1, editorModel.topLevelRequiredElements);
    return qMax(1, nodesById.value(parentId).configuration.requiredElements);
}
