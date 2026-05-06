#ifndef CYCLOGRAM_H
#define CYCLOGRAM_H

#include <QList>
#include <QString>

#include "node.h"

struct CyclogramStage
{
    QString name;
    OperationMode mode{OperationMode::Functioning};
    double duration{1.0};
};

using Cyclogram = QList<CyclogramStage>;

#endif // CYCLOGRAM_H
