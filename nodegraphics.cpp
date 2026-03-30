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

Node* NodeGraphics::getModelNode() const {return modelNode;}

void NodeGraphics::setModelNode(Node* node) {modelNode = node;}

QRectF NodeGraphics::boundingRect () const
{
    return QRectF(-25, -15, 50, 30);
}

void NodeGraphics::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if(isSelected())
        painter->setBrush(QColor("#ADD8E6"));
    else
        painter->setBrush(Qt::lightGray);
    painter->drawRoundedRect(boundingRect(), 15.0, 15.0);
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), Qt::AlignCenter, "Node");
}

void NodeGraphics::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    modelNode->setPreviousPosition(pos());
    QGraphicsItem::mousePressEvent(event);
}

void NodeGraphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit nodeDoubleClicked(modelNode);
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void NodeGraphics::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //есть другой вариант, отрубить коллизию линиям, но это надо обсуждать с куратором
    int step = 50;
    QPointF newPos = QPointF(round(pos().x()/step)*step, round(pos().y()/step)*step);
    for (QGraphicsItem* item : collidingItems())
    {
        if (item == this) continue;
        if(dynamic_cast<NodeGraphics*>(item))
        {
            setPos(modelNode->getPreviousPosition());
            modelNode->setPosition(modelNode->getPreviousPosition());
            return;
        }
    }
    setPos(newPos);
    modelNode->setPosition(newPos);
    update();

    QGraphicsItem::mouseReleaseEvent(event);
}

QVariant NodeGraphics::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange)
    {
        emit positionChanged();
    }

    return QGraphicsItem::itemChange(change, value);
}