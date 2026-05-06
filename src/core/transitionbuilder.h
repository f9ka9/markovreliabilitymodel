#ifndef TRANSITIONBUILDER_H
#define TRANSITIONBUILDER_H

#include <QList>

#include "reliabilitystate.h"

struct ReliabilityTransition
{
    int sourceStateId{0};
    int targetStateId{0};
    int changedNodeId{0};
    int changedNodeIndex{-1};
    ReliabilityElementState fromState{ReliabilityElementState::Working};
    ReliabilityElementState toState{ReliabilityElementState::Failed};

    int failedNodeId{0};
    int failedNodeIndex{-1};
};

class TransitionBuilder
{
public:
    QList<ReliabilityTransition> build(const QList<ReliabilityState>& states) const;
};

#endif // TRANSITIONBUILDER_H
