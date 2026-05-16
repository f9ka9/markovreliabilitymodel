#include "transitionbuilder.h"

QString TransitionBuilder::stateKey(const ReliabilityState& state) const
{
    QString key;
    key.reserve(state.elementStates.size());
    for (ReliabilityElementState elementState : state.elementStates)
    {
        key.append(elementState == ReliabilityElementState::Failed ? QLatin1Char('1') : QLatin1Char('0'));
    }
    return key;
}

QString TransitionBuilder::withFailedNode(QString key, int nodeIndex) const
{
    if (nodeIndex >= 0 && nodeIndex < key.size())
        key[nodeIndex] = QLatin1Char('1');
    return key;
}

QList<ReliabilityTransition> TransitionBuilder::build(const QList<ReliabilityState>& states) const
{
    QList<ReliabilityTransition> transitions;
    if (states.isEmpty()) return transitions;

    QHash<QString, int> stateIdByKey;
    for (const ReliabilityState& state : states)
        stateIdByKey.insert(stateKey(state), state.id);

    for (const ReliabilityState& source : states)
    {
        if (source.systemState == ReliabilitySystemState::Failure)
            continue;

        const QString sourceKey = stateKey(source);
        for (int nodeIndex = 0; nodeIndex < source.elementStates.size(); ++nodeIndex)
        {
            const ReliabilityElementState sourceState = source.elementStates[nodeIndex];
            if (sourceState == ReliabilityElementState::Failed)
                continue;

            const int targetStateId = stateIdByKey.value(withFailedNode(sourceKey, nodeIndex), -1);
            if (targetStateId < 0 || targetStateId >= states.size())
                continue;

            ReliabilityTransition transition;
            transition.sourceStateId = source.id;
            transition.targetStateId = targetStateId;
            transition.changedNodeIndex = nodeIndex;
            transition.failedNodeIndex = nodeIndex;
            transition.fromState = source.elementStates[nodeIndex];
            transition.toState = states[targetStateId].elementStates[nodeIndex];

            transition.changedNodeId = source.nodeIds.value(nodeIndex, 0);

            transition.failedNodeId = transition.changedNodeId;
            transitions.append(transition);
        }
    }

    return transitions;
}
