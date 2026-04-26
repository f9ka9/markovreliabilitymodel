#ifndef LINECONNECTIONGRAPHICS_H
#define LINECONNECTIONGRAPHICS_H

#include <QGraphicsObject>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QStyleOptionGraphicsItem>

#include "lineconnection.h"

class NodeGraphics;

class LineConnectionGraphics : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit LineConnectionGraphics(LineConnection* connection,
                                    NodeGraphics* first,
                                    NodeGraphics* second,
                                    const QList<QPointF>& bendPoints,
                                    QGraphicsItem* parent = nullptr);
    ~LineConnectionGraphics();

    LineConnection* getConnection() const;
    void updateGeometry();

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    LineConnection* connection{nullptr};
    NodeGraphics* firstNode{nullptr};
    NodeGraphics* secondNode{nullptr};
    QList<QPointF> bendPoints;
    bool firstSegmentHorizontal{true};
    bool lastSegmentHorizontal{true};
    QPainterPath cachedPath;

    QList<QPointF> buildPathPoints() const;
    QPointF nodeBoundaryPoint(NodeGraphics* node, const QPointF& toward) const;
};

#endif // LINECONNECTIONGRAPHICS_H
