#include "lineconnection.h"

Node* LineConnection::getFirst() const {return first;}
void LineConnection::setFirst(Node* node) {first = node;}

Node* LineConnection::getSecond() const {return second;}
void LineConnection::setSecond(Node* node) {second = node;}

void LineConnection::setPathPoints(const QList<QPointF>& points)
{pathPoints = points;}

QList<QPointF> LineConnection::getPathPoints() const{return pathPoints;}

void LineConnection::updatePathPoints()
{
    if (!first || !second) return;

    QPointF start = first->getPosition();
    QPointF end = second->getPosition();
    pathPoints.clear();
    pathPoints.append(start);
    pathPoints.append(QPointF(end.x(), start.y()));
    pathPoints.append(end);
}