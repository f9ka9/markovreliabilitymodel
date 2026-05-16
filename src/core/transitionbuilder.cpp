#include "transitionbuilder.h"

int TransitionBuilder::stateCode(const ReliabilityState& state) const
{
    int code = 0;
    int stateStep = 1;
    for (ReliabilityElementState elementState : state.elementStates)
    {
        if (elementState == ReliabilityElementState::Failed)
            code += stateStep;
        stateStep *= 2;
    }
    return code;
}

QList<ReliabilityTransition> TransitionBuilder::build(const QList<ReliabilityState>& states) const
{
    QList<ReliabilityTransition> transitions;
    if (states.isEmpty()) return transitions;

    QHash<int, int> stateIdByCode;
    for (const ReliabilityState& state : states)
        stateIdByCode.insert(stateCode(state), state.id);

    for (const ReliabilityState& source : states)
    {
        if (source.systemState == ReliabilitySystemState::Failure)
            continue;

        const int sourceCode = stateCode(source);
        int stateStep = 1;
        for (int nodeIndex = 0; nodeIndex < source.elementStates.size(); ++nodeIndex)
        {
            const ReliabilityElementState sourceState = source.elementStates[nodeIndex];
            if (sourceState == ReliabilityElementState::Failed)
            {
                stateStep *= 2;
                continue;
            }

            const int targetStateId = stateIdByCode.value(sourceCode | stateStep, -1);
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
