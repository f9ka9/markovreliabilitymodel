#ifndef NODEGRAPHICS_H
#define NODEGRAPHICS_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "node.h"

class NodeGraphics : public QGraphicsObject
{
    Q_OBJECT
public:
    NodeGraphics(Node* node,QGraphicsItem* parent = nullptr);
    ~NodeGraphics();

    Node* getModelNode() const;
    void setModelNode(Node* node);


protected:
    QRectF boundingRect()const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void nodeDoubleClicked(Node* Node);

private:
    Node* modelNode;

};
#endif // NODEGRAPHICS_