#ifndef STATESCENE_H
#define STATESCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "nodegraphics.h"

class StateScene : public QGraphicsScene
{
public:
    StateScene(QObject* parent = nullptr);
};

#endif // STATESCENE_H