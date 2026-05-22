#ifndef LINECONNECTIONGRAPHICS_H
#define LINECONNECTIONGRAPHICS_H

#include <QGraphicsObject>
#include <QCursor>
#include <QList>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QPointF>
#include <QStyleOptionGraphicsItem>
#include <QtMath>
#include <limits>

#include "lineconnection.h"
#include "nodegraphics.h"

class LineConnectionGraphics : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit LineConnectionGraphics(LineConnection* connection, NodeGraphics* first, NodeGraphics* second, const QList<QPointF>& bendPoints, QGraphicsItem* parent = nullptr);
    ~LineConnectionGraphics();

    LineConnection* getConnection() const;
    QList<QPointF> getBendPoints() const;
    void updateGeometry();

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QList<QPointF> buildPathPoints() const;
    QPointF nodeBoundaryPoint(NodeGraphics* node, const QPointF& toward) const;

    LineConnection* connection{nullptr};
    NodeGraphics* firstNode{nullptr};
    NodeGraphics* secondNode{nullptr};

    QList<QPointF> bendPoints{};

    bool firstSegmentHorizontal{true};
    bool lastSegmentHorizontal{true};

    QPainterPath cachedPath{};
};

#endif // LINECONNECTIONGRAPHICS_H
