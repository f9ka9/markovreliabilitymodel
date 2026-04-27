#ifndef EDITORSCHEMAMODEL_H
#define EDITORSCHEMAMODEL_H

#include <QList>
#include <QPointF>

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
    int currentParentId{0};
};

#endif // EDITORSCHEMAMODEL_H
