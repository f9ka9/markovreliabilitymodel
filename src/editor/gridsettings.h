#ifndef GRIDSETTINGS_H
#define GRIDSETTINGS_H

#include <QPointF>
#include <QtGlobal>

class GridSettings
{
public:
    static constexpr int step = 50;

    static QPointF snapToGrid(const QPointF& point)
    {
        return QPointF(qRound(point.x() / step) * step, qRound(point.y() / step) * step);
    }
};

#endif // GRIDSETTINGS_H
