#include "lineconnectiongraphics.h"
#include "nodegraphics.h"

LineConnectionGraphics::LineConnectionGraphics(NodeGraphics* first, NodeGraphics* second, QGraphicsItem* parent): QGraphicsObject(parent), modelFirst(first), modelSecond(second)
{
    setZValue(-1);


    connect(modelFirst, &NodeGraphics::positionChanged, this, &LineConnectionGraphics::updateLine);
    connect(modelSecond, &NodeGraphics::positionChanged, this, &LineConnectionGraphics::updateLine);
}

LineConnectionGraphics::~LineConnectionGraphics() {}

QRectF LineConnectionGraphics::boundingRect() const
{
    if (!modelFirst || !modelSecond)
        return QRectF();

    QPointF p1 = modelFirst->scenePos();
    QPointF p2 = modelSecond->scenePos();

    return QRectF(p1, p2).normalized().adjusted(-10, -10, 10, 10);
}

void LineConnectionGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *,QWidget *)
{
    if (!modelFirst || !modelSecond)
        return;

    QPointF p1 = modelFirst->pos();
    QPointF p2 = modelSecond->pos();

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(p1, p2);
}

void LineConnectionGraphics::updateLine()
{
    prepareGeometryChange();
    update();
}