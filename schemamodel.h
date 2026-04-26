#ifndef SCHEMAMODEL_H
#define SCHEMAMODEL_H

#include <QList>
#include <QString>

#include "node.h"

struct SchemaNodeData
{
    int id{0};
    int parentId{0};
    int depth{1};
    QString name;
    QString groupName;
    FailureRates failureRates{};
    Node::StructureType structureType{Node::StructureType::Element};
    int requiredElements{1};
};

struct SchemaConnectionData
{
    int sourceNodeId{0};
    int targetNodeId{0};
};

struct SchemaModel
{
    QList<SchemaNodeData> nodes;
    QList<SchemaConnectionData> connections;
    int currentParentId{0};
};

#endif // SCHEMAMODEL_H
