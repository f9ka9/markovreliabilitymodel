#ifndef STATEGENERATOR_H
#define STATEGENERATOR_H

#include <QHash>
#include <QList>
#include <QSet>
#include <QStringList>

#include "reliabilitystate.h"
#include "schemamodel.h"

class StateGenerator
{
public:
    QList<ReliabilityState> generate(const QList<SchemaNodeData>& nodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements, int maxStateCount = 0) const;

private:
    QList<SchemaNodeData> reliabilityNodes(const QList<SchemaNodeData>& nodes) const;
    QString initialStateKey(int stateCount) const;
    QString withFailedNode(QString key, int nodeIndex) const;

    ReliabilityState createState(int stateId, const QString& stateKey, const QList<SchemaNodeData>& stateNodes, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements) const;
    ReliabilitySystemState classifyState(const ReliabilityState& state, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements) const;

    bool structureWorksByType(const ReliabilityState& state, int totalCount, StructureType structureType, int requiredElements) const;
    bool structureWorksByConnections(const ReliabilityState& state, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections) const;
    bool nodeWorksById(const ReliabilityState& state, int nodeId) const;
    bool nodeWorks(const ReliabilityState& state, int index) const;

    QString createStateName(const ReliabilityState& state, const QList<SchemaNodeData>& nodes) const;
};

#endif // STATEGENERATOR_H
