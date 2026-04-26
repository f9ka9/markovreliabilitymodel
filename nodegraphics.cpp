#include "nodegraphics.h"

#include <QCursor>
#include <QFontMetrics>
#include <QGraphicsScene>

#include "gridsettings.h"

NodeGraphics::NodeGraphics(Node* node, QGraphicsItem* parent): QGraphicsObject(parent), modelNode(node)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(QCursor(Qt::ArrowCursor));
}

NodeGraphics::~NodeGraphics() = default;

Node* NodeGraphics::getModelNode() const {return modelNode;}

QRectF NodeGraphics::connectionRect() const
{
    return boundingRect();
}

QRectF NodeGraphics::boundingRect() const
{
    return QRectF(-25, -15, 50, 30);
}

void NodeGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setBrush(isSelected() ? QColor("#ADD8E6") : Qt::lightGray);
    painter->drawRoundedRect(boundingRect(), 15.0, 15.0);
    painter->setPen(Qt::black);

    QString text = modelNode ? modelNode->getName() : QString("Node");
    if (text.isEmpty()) text = "Node";

    const QFontMetrics metrics(painter->font());
    text = metrics.elidedText(text, Qt::ElideRight, boundingRect().width() - 8);
    painter->drawText(boundingRect(), Qt::AlignCenter, text);
}

void NodeGraphics::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    previousPosition = pos();
    QGraphicsItem::mousePressEvent(event);
}

void NodeGraphics::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!modelNode) return;

    emit nodeDoubleClicked(modelNode);
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void NodeGraphics::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    const QPointF newPos = GridSettings::snapToGrid(pos());
    setPos(newPos);

    for (QGraphicsItem* item : collidingItems())
    {
        if (item == this) continue;
        if(dynamic_cast<NodeGraphics*>(item))
        {
            setPos(previousPosition);
            emit positionChanged();
            QGraphicsItem::mouseReleaseEvent(event);
            return;
        }
    }

    emit positionChanged();
    update();

    QGraphicsItem::mouseReleaseEvent(event);
}

QVariant NodeGraphics::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
        emit positionChanged();

    return QGraphicsItem::itemChange(change, value);
}
