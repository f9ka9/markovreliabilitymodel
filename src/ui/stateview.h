#ifndef STATEVIEW_H
#define STATEVIEW_H

#include <QGraphicsView>

class QWheelEvent;

class StateView : public QGraphicsView
{
public:
    StateView(QWidget* parent = nullptr);
    ~StateView();

protected:
    void wheelEvent(QWheelEvent* event) override;
};

#endif // STATEVIEW_H
