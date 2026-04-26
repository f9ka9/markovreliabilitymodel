#ifndef GRIDSETTINGS_H
#define GRIDSETTINGS_H

#include <QPointF>
#include <QtGlobal>

namespace GridSettings
{
constexpr int step = 50;

inline QPointF snapToGrid(const QPointF& point)
{
    return QPointF(qRound(point.x() / step) * step,
                   qRound(point.y() / step) * step);
}
}

#endif // GRIDSETTINGS_H
