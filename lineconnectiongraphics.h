#ifndef LINECONNECTIONGRAPHICS_H
#define LINECONNECTIONGRAPHICS_H

#include <QGraphicsObject>
#include <QPainter>

class NodeGraphics;

class LineConnectionGraphics : public QGraphicsObject
{
    Q_OBJECT
public:
    LineConnectionGraphics(NodeGraphics* first, NodeGraphics* second, QGraphicsItem* parent = nullptr);
    ~LineConnectionGraphics();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

public slots:
    void updateLine();

private:
    NodeGraphics* modelFirst{nullptr};
    NodeGraphics* modelSecond{nullptr};
};

#endif // LINECONNECTIONGRAPHICS_H