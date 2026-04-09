#ifndef STATEVIEW_H
#define STATEVIEW_H

#include <QGraphicsView>

class StateView : public QGraphicsView
{
public:
    StateView(QWidget* parent = nullptr);
    ~StateView();
};

#endif // STATEVIEW_H
