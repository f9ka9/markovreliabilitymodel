#ifndef NODEGRAPHICS_H
#define NODEGRAPHICS_H

#include <QGraphicsObject>
#include <QCursor>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QHash>
#include <QPainter>
#include <QPointF>

#include "gridsettings.h"
#include "node.h"

class NodeGraphics : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit NodeGraphics(Node* node, QGraphicsItem* parent = nullptr);
    ~NodeGraphics();

    static QRectF defaultBoundingRect();

    Node* getModelNode() const;
    QRectF connectionRect() const;

signals:
    void nodeDoubleClicked(Node* node);
    void positionChanged();

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    Node* modelNode{nullptr};
    QPointF previousPosition;
    QHash<NodeGraphics*, QPointF> draggedNodesStartPositions;
};

#endif // NODEGRAPHICS_H
