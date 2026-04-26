#ifndef NODEGRAPHICS_H
#define NODEGRAPHICS_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPointF>

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

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

signals:
    void nodeDoubleClicked(Node* node);
    void positionChanged();

private:
    Node* modelNode{nullptr};
    QPointF previousPosition;
};

#endif // NODEGRAPHICS_H
