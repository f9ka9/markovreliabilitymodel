#include "lineconnectiongraphics.h"

LineConnectionGraphics::LineConnectionGraphics(LineConnection* conn, QGraphicsItem* parent)
    : QGraphicsObject(parent), connection(conn)
{
    setZValue(-0.5);
    updateGeometry();
}

LineConnectionGraphics::~LineConnectionGraphics() {}

LineConnection* LineConnectionGraphics::getConnection() const {return connection;}
void LineConnectionGraphics::updateGeometry()
{
    if (!connection) return;

    connection->updatePathPoints();
    QList<QPointF> points = connection->getPathPoints();

    cachedPath = QPainterPath();
    if (points.isEmpty()) return;

    cachedPath.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        cachedPath.lineTo(points[i]);

    prepareGeometryChange();
}

QRectF LineConnectionGraphics::boundingRect() const
{
    if (cachedPath.isEmpty()) return QRectF();
    return cachedPath.boundingRect().adjusted(-2, -2, 2, 2);
}

void LineConnectionGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (!connection) return;
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawPath(cachedPath);
    painter->restore();
}