#include "transitionbuilder.h"

QList<ReliabilityTransition> TransitionBuilder::build(const QList<ReliabilityState>& states) const
{
    QList<ReliabilityTransition> transitions;
    if (states.isEmpty()) return transitions;

    for (const ReliabilityState& source : states)
    {
        if (source.systemState == ReliabilitySystemState::Failure)
            continue;

        int stateStep = 1;
        for (int nodeIndex = 0; nodeIndex < source.elementStates.size(); ++nodeIndex)
        {
            const ReliabilityElementState sourceState = source.elementStates[nodeIndex];
            if (sourceState == ReliabilityElementState::Failed)
            {
                stateStep *= 2;
                continue;
            }

            const int targetStateId = source.id + stateStep;
            if (targetStateId < 0 || targetStateId >= states.size())
            {
                stateStep *= 2;
                continue;
            }

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

            stateStep *= 2;
        }
    }

    return transitions;
}
