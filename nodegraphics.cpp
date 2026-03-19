#include "nodegraphics.h"
#include <qgraphicsscene.h>

NodeGraphics::NodeGraphics(Node* node, QGraphicsItem* parent):  QGraphicsObject(parent), modelNode(node)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}


NodeGraphics::~NodeGraphics()
{}


QRectF NodeGraphics::boundingRect () const
{
    return QRectF(-25, -15, 50, 30);
}

void NodeGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(boundingRect(), 15.0, 15.0);
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, "Node");
}

QVariant NodeGraphics::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange && scene())
    {
        int step = 50;
        int x = round(value.toPointF().x()/step)*step;
        int y = round(value.toPointF().y()/step)*step;
        return QPointF(x,y);
    }
    return QGraphicsItem::itemChange(change, value);
}

void NodeGraphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit nodeDoubleClicked(modelNode);
    QGraphicsItem::mouseDoubleClickEvent(event);
}