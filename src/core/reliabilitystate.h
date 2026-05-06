#ifndef RELIABILITYSTATE_H
#define RELIABILITYSTATE_H

#include <QList>
#include <QString>

enum class ReliabilityElementState { Working, Failed };

enum class ReliabilitySystemState { Working, PartialFailure, Failure };

struct ReliabilityState
{
    int id{0};
    QList<int> nodeIds;
    QList<ReliabilityElementState> elementStates;
    QList<int> failedNodeIds;
    ReliabilitySystemState systemState{ReliabilitySystemState::Working};
    QString name;
};

#endif // RELIABILITYSTATE_H
