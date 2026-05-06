#include "statescene.h"

#include <QBrush>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QLineF>
#include <QPen>
#include <QtMath>

StateScene::StateScene(QObject* parent):QGraphicsScene(parent)
{
    setSceneRect(-1000, -1000, 2000, 2000);
}

void StateScene::showCalculationResult(const CalculationResult& result)
{
    clear();
    if (result.states.isEmpty()) return;

    const QList<ReliabilityState> visibleStates = reachableStates(result);
    const QSet<int> visibleStateIds = reachableStateIds(result);
    const QHash<int, QPointF> positionsByStateId = calculateStatePositions(visibleStates);
    const bool compactMode = visibleStates.size() > 600;
    const qreal nodeRadius = compactMode ? 5.0 : 30.0;

    for (int i = 0; i < visibleStates.size(); ++i)
    {
        const ReliabilityState& state = visibleStates[i];
        const QPointF position = positionsByStateId.value(state.id);

        QGraphicsEllipseItem* ellipse = addEllipse(position.x() - nodeRadius, position.y() - nodeRadius, nodeRadius * 2.0, nodeRadius * 2.0, compactMode ? QPen(Qt::black, 0.4) : QPen(Qt::black, 2), QBrush(stateColor(state.systemState)));
        ellipse->setZValue(1.0);

        if (compactMode)
            continue;

        QGraphicsTextItem* label = addText(QString("S%1").arg(state.id));
        label->setDefaultTextColor(Qt::black);
        label->setFont(QFont("Arial", 10, QFont::Bold));
        label->setPos(position.x() - label->boundingRect().width() / 2.0, position.y() - label->boundingRect().height() / 2.0);
        label->setZValue(2.0);
    }

    int drawnTransitions = 0;
    constexpr int maxCompactTransitions = 8000;
    for (const ReliabilityTransition& transition : result.transitions)
    {
        if (!visibleStateIds.contains(transition.sourceStateId) || !visibleStateIds.contains(transition.targetStateId))
            continue;

        if (!positionsByStateId.contains(transition.sourceStateId) || !positionsByStateId.contains(transition.targetStateId))
            continue;

        const QPointF source = positionsByStateId.value(transition.sourceStateId);
        const QPointF target = positionsByStateId.value(transition.targetStateId);
        if (compactMode)
        {
            if (drawnTransitions >= maxCompactTransitions)
                continue;

            QGraphicsLineItem* line = addLine(QLineF(source, target), QPen(QColor(80, 80, 80, 80), 0.35));
            line->setZValue(0.0);
        }
        else
        {
            addTransitionArrow(source, target);
        }

        ++drawnTransitions;
    }
}

QList<ReliabilityState> StateScene::reachableStates(const CalculationResult& result) const
{
    const QSet<int> ids = reachableStateIds(result);
    QList<ReliabilityState> states;
    for (const ReliabilityState& state : result.states)
    {
        if (ids.contains(state.id))
            states.append(state);
    }
    return states;
}

QSet<int> StateScene::reachableStateIds(const CalculationResult& result) const
{
    QHash<int, QList<int>> adjacency;
    for (const ReliabilityTransition& transition : result.transitions)
        adjacency[transition.sourceStateId].append(transition.targetStateId);

    QSet<int> visited;
    QList<int> queue;
    visited.insert(0);
    queue.append(0);

    while (!queue.isEmpty())
    {
        const int current = queue.takeFirst();
        for (int next : adjacency.value(current))
        {
            if (visited.contains(next))
                continue;

            visited.insert(next);
            queue.append(next);
        }
    }

    return visited;
}

QHash<int, QPointF> StateScene::calculateStatePositions(const QList<ReliabilityState>& states) const
{
    QHash<int, QPointF> positions;
    if (states.isEmpty()) return positions;

    int maxFailures = 0;
    for (const ReliabilityState& state : states)
        maxFailures = qMax(maxFailures, state.failedNodeIds.size());

    QVector<QList<const ReliabilityState*>> levels(maxFailures + 1);
    for (const ReliabilityState& state : states)
        levels[state.failedNodeIds.size()].append(&state);

    const bool compactMode = states.size() > 600;
    const qreal horizontalSpacing = compactMode ? 140.0 : 190.0;
    const qreal verticalSpacing = compactMode ? 14.0 : 95.0;
    const qreal startX = -horizontalSpacing * maxFailures / 2.0;

    for (int level = 0; level < levels.size(); ++level)
    {
        const QList<const ReliabilityState*>& levelStates = levels[level];
        const qreal levelHeight = (levelStates.size() - 1) * verticalSpacing;
        const qreal x = startX + level * horizontalSpacing;

        for (int index = 0; index < levelStates.size(); ++index)
        {
            const qreal y = index * verticalSpacing - levelHeight / 2.0;
            positions.insert(levelStates[index]->id, QPointF(x, y));
        }
    }

    return positions;
}

QColor StateScene::stateColor(ReliabilitySystemState state) const
{
    switch (state)
    {
    case ReliabilitySystemState::Working:
        return QColor("#BFE8C6");
    case ReliabilitySystemState::PartialFailure:
        return QColor("#FFE2A8");
    case ReliabilitySystemState::Failure:
        return QColor("#F3A6A6");
    }

    return Qt::lightGray;
}

void StateScene::addTransitionArrow(const QPointF& source, const QPointF& target)
{
    QLineF line(source, target);
    if (line.length() <= 0.0) return;

    constexpr qreal nodeRadius = 30.0;
    if (line.length() <= nodeRadius * 2.0) return;

    const QPointF direction((target.x() - source.x()) / line.length(), (target.y() - source.y()) / line.length());
    const QPointF visibleStart = source + direction * nodeRadius;
    const QPointF arrowEnd = target - direction * nodeRadius;
    QLineF visibleLine(visibleStart, arrowEnd);

    QGraphicsLineItem* lineItem = addLine(visibleLine, QPen(Qt::darkGray, 1.5));
    lineItem->setZValue(0.0);

    const qreal angle = qAtan2(-visibleLine.dy(), visibleLine.dx());
    constexpr qreal arrowSize = 10.0;
    const QPointF arrowP1 = arrowEnd + QPointF(qSin(angle + M_PI / 3.0) * arrowSize, qCos(angle + M_PI / 3.0) * arrowSize);
    const QPointF arrowP2 = arrowEnd + QPointF(qSin(angle + M_PI - M_PI / 3.0) * arrowSize, qCos(angle + M_PI - M_PI / 3.0) * arrowSize);

    QPolygonF arrowHead;
    arrowHead << arrowEnd << arrowP1 << arrowP2;
    QGraphicsPolygonItem* arrowItem = addPolygon(arrowHead, QPen(Qt::darkGray), QBrush(Qt::darkGray));
    arrowItem->setZValue(0.5);
}
