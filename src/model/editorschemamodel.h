#ifndef EDITORSCHEMAMODEL_H
#define EDITORSCHEMAMODEL_H

#include <QList>
#include <QPointF>
#include <QVector>

#include "cyclogram.h"
#include "node.h"

struct EditorSchemaNodeData
{
    int id{0};
    int parentId{0};
    QPointF position;
    NodeConfiguration configuration;
};

struct EditorSchemaConnectionData
{
    int sourceNodeId{0};
    int targetNodeId{0};
    QList<QPointF> bendPoints;
};

struct EditorSchemaModel
{
    QList<EditorSchemaNodeData> nodes;
    QList<EditorSchemaConnectionData> connections;
    NodeConfiguration defaultNodeConfiguration;
    StructureType topLevelStructureType{StructureType::Series};
    int topLevelRequiredElements{1};
    Cyclogram cyclogram;
    QVector<double> initialProbabilities;
    int currentParentId{0};
};

#endif // EDITORSCHEMAMODEL_H
