#ifndef LINECONNECTIONGRAPHICS_H
#define LINECONNECTIONGRAPHICS_H

#include <QGraphicsObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "lineconnection.h"

class LineConnectionGraphics : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit LineConnectionGraphics(LineConnection* connection, QGraphicsItem* parent = nullptr);
    ~LineConnectionGraphics();

    LineConnection* getConnection() const;
    void updateGeometry();

protected:
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    LineConnection* connection;
    QPainterPath cachedPath;
};

#endif // LINECONNECTIONGRAPHICS_H