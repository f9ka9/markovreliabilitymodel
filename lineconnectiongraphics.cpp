#include "lineconnectiongraphics.h"

#include <limits>

#include <QCursor>
#include <QPainterPathStroker>
#include <QtMath>

#include "nodegraphics.h"

LineConnectionGraphics::LineConnectionGraphics(LineConnection* conn,
                                               NodeGraphics* first,
                                               NodeGraphics* second,
                                               const QList<QPointF>& points,
                                               QGraphicsItem* parent)
    : QGraphicsObject(parent),
      connection(conn),
      firstNode(first),
      secondNode(second),
      bendPoints(points)
{
    setZValue(-0.5);
    setCursor(QCursor(Qt::ArrowCursor));
    if (!bendPoints.isEmpty() && firstNode && secondNode)
    {
        firstSegmentHorizontal = qFuzzyIsNull(firstNode->pos().y() - bendPoints.first().y());
        lastSegmentHorizontal = qFuzzyIsNull(secondNode->pos().y() - bendPoints.last().y());
    }
    updateGeometry();
}

LineConnectionGraphics::~LineConnectionGraphics() = default;

LineConnection* LineConnectionGraphics::getConnection() const {return connection;}

void LineConnectionGraphics::updateGeometry()
{
    prepareGeometryChange();

    const QList<QPointF> points = buildPathPoints();
    cachedPath = QPainterPath();
    if (points.isEmpty()) return;

    cachedPath.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        cachedPath.lineTo(points[i]);

    update();
}

QRectF LineConnectionGraphics::boundingRect() const
{
    if (cachedPath.isEmpty()) return QRectF();
    return cachedPath.boundingRect().adjusted(-14, -14, 14, 14);
}

QPainterPath LineConnectionGraphics::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(cachedPath);
}

void LineConnectionGraphics::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (!connection) return;
    const QList<QPointF> points = buildPathPoints();

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawPath(cachedPath);

    if (points.size() >= 2)
    {
        const QPointF tip = points.last();
        QPointF segmentStart = points[points.size() - 2];
        for (int i = points.size() - 2; i >= 0 && segmentStart == tip; --i)
            segmentStart = points[i];

        const QPointF direction = tip - segmentStart;
        const qreal length = qSqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (length > 0.0)
        {
            const QPointF unit(direction.x() / length, direction.y() / length);
            const QPointF normal(-unit.y(), unit.x());
            constexpr qreal arrowLength = 9.0;
            constexpr qreal arrowWidth = 5.0;

            const QPointF base = tip - unit * arrowLength;
            const QPointF wing1 = base + normal * arrowWidth;
            const QPointF wing2 = base - normal * arrowWidth;

            painter->drawLine(tip, wing1);
            painter->drawLine(tip, wing2);
        }
    }

    painter->restore();
}

QList<QPointF> LineConnectionGraphics::buildPathPoints() const
{
    if (!firstNode || !secondNode) return {};

    QList<QPointF> points;
    const QPointF firstCenter = firstNode->pos();
    const QPointF secondCenter = secondNode->pos();

    QList<QPointF> adjustedBends = bendPoints;
    if (!adjustedBends.isEmpty())
    {
        if (firstSegmentHorizontal) adjustedBends.first().setY(firstCenter.y());
        else adjustedBends.first().setX(firstCenter.x());

        if (lastSegmentHorizontal) adjustedBends.last().setY(secondCenter.y());
        else adjustedBends.last().setX(secondCenter.x());
    }

    const QPointF firstToward = adjustedBends.isEmpty() ? secondCenter : adjustedBends.first();
    const QPointF secondFrom = adjustedBends.isEmpty() ? firstCenter : adjustedBends.last();

    points.append(nodeBoundaryPoint(firstNode, firstToward));
    for (const QPointF& bend : adjustedBends)
        points.append(bend);
    points.append(nodeBoundaryPoint(secondNode, secondFrom));
    return points;
}

QPointF LineConnectionGraphics::nodeBoundaryPoint(NodeGraphics* node, const QPointF& toward) const
{
    if (!node) return {};

    const QPointF center = node->pos();
    const QPointF delta = toward - center;
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y()))
        return center;

    const QRectF rect = node->connectionRect();
    const qreal halfWidth = rect.width() / 2.0;
    const qreal halfHeight = rect.height() / 2.0;

    const qreal scaleX = qFuzzyIsNull(delta.x()) ? std::numeric_limits<qreal>::max() : halfWidth / qAbs(delta.x());
    const qreal scaleY = qFuzzyIsNull(delta.y()) ? std::numeric_limits<qreal>::max() : halfHeight / qAbs(delta.y());
    const qreal scale = qMin(scaleX, scaleY);

    return center + delta * scale;
}
