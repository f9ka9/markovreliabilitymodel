#include "stategenerator.h"

QList<SchemaNodeData> StateGenerator::reliabilityNodes(const QList<SchemaNodeData>& nodes) const
{
    QList<SchemaNodeData> result;
    for (const SchemaNodeData& node : nodes)
    {
        if (node.nodeKind == NodeKind::Normal)
            result.append(node);
    }
    return result;
}


QList<ReliabilityState> StateGenerator::generate(const QList<SchemaNodeData>& nodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements) const
{
    QList<ReliabilityState> states;
    if (nodes.isEmpty()) return states;

    const QList<SchemaNodeData> stateNodes = reliabilityNodes(nodes);
    int stateCount = 1;
    for (int i = 0; i < stateNodes.size(); ++i)
        stateCount *= 2;
    states.reserve(stateCount);

    for (int stateCode = 0; stateCode < stateCount; ++stateCode)
        states.append(createState(states.size(), stateCode, stateNodes, nodes, connections, structureType, requiredElements));

    return states;
}

ReliabilityState StateGenerator::createState(int stateId, int stateCode, const QList<SchemaNodeData>& stateNodes, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements) const
{
    ReliabilityState state;
    state.id = stateId;

    int code = stateCode;
    for (int i = 0; i < stateNodes.size(); ++i)
    {
        state.nodeIds.append(stateNodes[i].id);
        const int digit = code % 2;
        code /= 2;

        if (digit == 0)
        {
            state.elementStates.append(ReliabilityElementState::Working);
        }
        else
        {
            state.elementStates.append(ReliabilityElementState::Failed);
            state.failedNodeIds.append(stateNodes[i].id);
        }
    }

    state.systemState = classifyState(state, allNodes, connections, structureType, requiredElements);
    state.name = createStateName(state, stateNodes);
    return state;
}

ReliabilitySystemState StateGenerator::classifyState(const ReliabilityState& state, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections, StructureType structureType, int requiredElements) const
{
    if (state.failedNodeIds.isEmpty())
    {
        if (connections.isEmpty()) return ReliabilitySystemState::Working;

        return structureWorksByConnections(state, allNodes, connections) ? ReliabilitySystemState::Working : ReliabilitySystemState::Failure;
    }

    const bool systemWorks = connections.isEmpty() ? structureWorksByType(state, state.elementStates.size(), structureType, requiredElements) : structureWorksByConnections(state, allNodes, connections);

    return systemWorks ? ReliabilitySystemState::PartialFailure : ReliabilitySystemState::Failure;
}

bool StateGenerator::structureWorksByType(const ReliabilityState& state, int totalCount, StructureType structureType, int requiredElements) const
{
    int workingCount = 0;
    for (int i = 0; i < state.elementStates.size(); ++i)
    {
        if (nodeWorks(state, i))
            ++workingCount;
    }

    const int failedCount = totalCount - workingCount;
    bool systemWorks = false;

    switch (structureType)
    {
    case StructureType::Series:
        systemWorks = failedCount == 0;
        break;
    case StructureType::Parallel:
        systemWorks = workingCount > 0;
        break;
    case StructureType::KOutOfN:
        systemWorks = workingCount >= requiredElements;
        break;
    case StructureType::Element:
        systemWorks = workingCount > 0;
        break;
    }

    return systemWorks;
}

bool StateGenerator::structureWorksByConnections(const ReliabilityState& state, const QList<SchemaNodeData>& allNodes, const QList<SchemaConnectionData>& connections) const
{
    QSet<int> nodeIds;
    for (const SchemaNodeData& node : allNodes)
        nodeIds.insert(node.id);

    QHash<int, QList<int>> adjacency;
    QSet<int> hasIncoming;
    QSet<int> hasOutgoing;
    for (const SchemaConnectionData& connection : connections)
    {
        if (!nodeIds.contains(connection.sourceNodeId)|| !nodeIds.contains(connection.targetNodeId)) continue;

        adjacency[connection.sourceNodeId].append(connection.targetNodeId);
        hasOutgoing.insert(connection.sourceNodeId);
        hasIncoming.insert(connection.targetNodeId);
    }

    QList<int> sources;
    QList<int> sinks;
    for (const SchemaNodeData& node : allNodes)
    {
        if (node.nodeKind == NodeKind::Input)
            sources.append(node.id);
        if (node.nodeKind == NodeKind::Output)
            sinks.append(node.id);
    }

    if (sources.isEmpty() || sinks.isEmpty())
    {
        sources.clear();
        sinks.clear();
        for (const SchemaNodeData& node : allNodes)
        {
            if (!hasIncoming.contains(node.id))
                sources.append(node.id);
            if (!hasOutgoing.contains(node.id))
                sinks.append(node.id);
        }
    }

    if (sources.isEmpty() || sinks.isEmpty())
        return structureWorksByType(state, state.elementStates.size(), StructureType::Series, 1);

    QSet<int> sinkSet;
    for (int sink : sinks)
        sinkSet.insert(sink);
    QList<int> queue;
    QSet<int> visited;

    for (int sourceId : sources)
    {
        if (!nodeWorksById(state, sourceId))
            continue;

        queue.append(sourceId);
        visited.insert(sourceId);
    }

    while (!queue.isEmpty())
    {
        const int current = queue.takeFirst();
        if (sinkSet.contains(current))
            return true;

        for (int next : adjacency.value(current))
        {
            if (!nodeIds.contains(next) || !nodeWorksById(state, next) || visited.contains(next))
                continue;

            visited.insert(next);
            queue.append(next);
        }
    }

    return false;
}

bool StateGenerator::nodeWorksById(const ReliabilityState& state, int nodeId) const
{
    const int stateIndex = state.nodeIds.indexOf(nodeId);
    if (stateIndex < 0)
        return true;

    return nodeWorks(state, stateIndex);
}

bool StateGenerator::nodeWorks(const ReliabilityState& state, int index) const
{
    if (index < 0 || index >= state.elementStates.size())
        return false;

    return state.elementStates[index] != ReliabilityElementState::Failed;
}

QString StateGenerator::createStateName(const ReliabilityState& state, const QList<SchemaNodeData>& nodes) const
{
    if (state.failedNodeIds.isEmpty())
        return "S0: все работают";

    QStringList failedNames;
    for (int i = 0; i < state.elementStates.size() && i < nodes.size(); ++i)
    {
        if (state.elementStates[i] == ReliabilityElementState::Working)
            continue;

        const QString name = nodes[i].name.isEmpty() ? QString("узел #%1").arg(nodes[i].id) : nodes[i].name;

        failedNames.append(name);
    }

    QStringList parts;
    if (!failedNames.isEmpty())
        parts.append("failed " + failedNames.join(", "));

    return QString("S%1: %2").arg(state.id).arg(parts.join("; "));
}
