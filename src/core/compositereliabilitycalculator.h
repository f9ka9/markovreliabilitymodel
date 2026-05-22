#ifndef COMPOSITERELIABILITYCALCULATOR_H
#define COMPOSITERELIABILITYCALCULATOR_H

#include <QHash>
#include <QList>
#include <QString>
#include <cmath>
#include <QtMath>
#include <QSet>

#include "calculationresult.h"
#include "editorschemamodel.h"
#include "schemamodel.h"
#include "reliabilitycore.h"

class CompositeReliabilityCalculator
{
public:
    SchemaModel buildEffectiveModel(const EditorSchemaModel& editorModel, int parentId) const;

private:
    QHash<int, EditorSchemaNodeData> createNodesById(const EditorSchemaModel& editorModel) const;
    QHash<int, QList<EditorSchemaNodeData>> createChildrenByParentId(const EditorSchemaModel& editorModel) const;
    SchemaModel buildEffectiveModel(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    SchemaNodeData createEffectiveNodeData(const EditorSchemaModel& editorModel, const EditorSchemaNodeData& node, int depth, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    QList<SchemaConnectionData> directConnections(const EditorSchemaModel& editorModel, int parentId, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    FailureRates effectiveFailureRates(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    double effectiveFailureRate(const EditorSchemaModel& editorModel, int parentId, OperationMode mode, const QHash<int, EditorSchemaNodeData>& nodesById, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    double workProbability(const CalculationResult& result) const;
    bool hasChildren(int nodeId, const QHash<int, QList<EditorSchemaNodeData>>& childrenByParentId) const;
    StructureType structureTypeForParent(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById) const;
    int requiredElementsForParent(const EditorSchemaModel& editorModel, int parentId, const QHash<int, EditorSchemaNodeData>& nodesById) const;

    mutable QHash<QString, double> effectiveFailureRateCache;
};

#endif // COMPOSITERELIABILITYCALCULATOR_H
