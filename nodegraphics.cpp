#include "nodegraphics.h"

NodeGraphics::NodeGraphics(Node* node, QGraphicsItem* parent): QGraphicsObject(parent), modelNode(node)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCursor(QCursor(Qt::ArrowCursor));
}

NodeGraphics::~NodeGraphics() = default;

QRectF NodeGraphics::defaultBoundingRect()
{
    return QRectF(-25, -15, 50, 30);
}

Node* NodeGraphics::getModelNode() const {return modelNode;}

QRectF NodeGraphics::connectionRect() const
{
    return boundingRect();
}

QRectF NodeGraphics::boundingRect() const
{
    return defaultBoundingRect();
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
    draggedNodesStartPositions.clear();

    if (scene())
    {
        for (QGraphicsItem* item : scene()->selectedItems())
        {
            if (NodeGraphics* nodeGraphics = dynamic_cast<NodeGraphics*>(item))
                draggedNodesStartPositions.insert(nodeGraphics, nodeGraphics->pos());
        }
    }

    if (!draggedNodesStartPositions.contains(this))
        draggedNodesStartPositions.insert(this, previousPosition);

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
    QHash<NodeGraphics*, QPointF> snappedPositions;
    for (auto it = draggedNodesStartPositions.cbegin(); it != draggedNodesStartPositions.cend(); ++it)
    {
        NodeGraphics* nodeGraphics = it.key();
        if (!nodeGraphics) continue;

        const QPointF snappedPosition = GridSettings::snapToGrid(nodeGraphics->pos());
        snappedPositions.insert(nodeGraphics, snappedPosition);
        nodeGraphics->setPos(snappedPosition);
    }

    bool hasCollision = false;
    for (auto it = snappedPositions.cbegin(); it != snappedPositions.cend() && !hasCollision; ++it)
    {
        NodeGraphics* nodeGraphics = it.key();
        if (!nodeGraphics) continue;

        for (QGraphicsItem* item : nodeGraphics->collidingItems())
        {
            if (NodeGraphics* otherNode = dynamic_cast<NodeGraphics*>(item))
            {
                if (!snappedPositions.contains(otherNode))
                {
                    hasCollision = true;
                    break;
                }
            }
        }
    }

    if (hasCollision)
    {
        for (auto it = draggedNodesStartPositions.cbegin(); it != draggedNodesStartPositions.cend(); ++it)
        {
            if (NodeGraphics* nodeGraphics = it.key())
            {
                nodeGraphics->setPos(it.value());
                emit nodeGraphics->positionChanged();
                nodeGraphics->update();
            }
        }

        QGraphicsItem::mouseReleaseEvent(event);
        return;
    }

    for (NodeGraphics* nodeGraphics : snappedPositions.keys())
    {
        if (!nodeGraphics) continue;
        emit nodeGraphics->positionChanged();
        nodeGraphics->update();
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

QVariant NodeGraphics::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
        emit positionChanged();

    return QGraphicsItem::itemChange(change, value);
}
