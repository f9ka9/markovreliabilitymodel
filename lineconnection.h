#ifndef LINECONNECTION_H
#define LINECONNECTION_H

#ifndef _LINECONNECTION_H
#define _LINECONNECTION_H

#include <QPointF>
#include <QList>
#include "node.h"

class Node;

class LineConnection
{
public:
    LineConnection() = default;
    ~LineConnection() = default;

    Node* getFirst() const;
    void setFirst(Node* node);

    Node* getSecond() const;
    void setSecond(Node* node);

    void setPathPoints(const QList<QPointF>& points);
    QList<QPointF> getPathPoints() const;
    void updatePathPoints();

private:
    Node* first{nullptr};
    Node* second{nullptr};
    QList<QPointF> pathPoints;
};

#endif // _LINECONNECTION_H

#endif // LINECONNECTION_H
