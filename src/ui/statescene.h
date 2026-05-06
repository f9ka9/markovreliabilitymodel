#ifndef STATESCENE_H
#define STATESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QPointF>
#include <QSet>
#include <QVector>

#include "calculationresult.h"

class StateScene : public QGraphicsScene
{
public:
    StateScene(QObject* parent = nullptr);

    void showCalculationResult(const CalculationResult& result);

private:
    QList<ReliabilityState> reachableStates(const CalculationResult& result) const;
    QSet<int> reachableStateIds(const CalculationResult& result) const;
    QHash<int, QPointF> calculateStatePositions(const QList<ReliabilityState>& states) const;
    QColor stateColor(ReliabilitySystemState state) const;
    void addTransitionArrow(const QPointF& source, const QPointF& target);
};

#endif // STATESCENE_H
