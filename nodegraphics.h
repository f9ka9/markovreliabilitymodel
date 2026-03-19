#ifndef NODEGRAPHICS_H
#define NODEGRAPHICS_H

#include <QGraphicsObject>
#include <QPainter>

#include "node.h"

class NodeGraphics : public QGraphicsObject
{
    Q_OBJECT
public:
    NodeGraphics(Node* node,QGraphicsItem* parent = nullptr);
    ~NodeGraphics();
protected:
    QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void nodeDoubleClicked(Node* Node);

private:
    Node* modelNode;


};
#endif // NODEGRAPHICS_