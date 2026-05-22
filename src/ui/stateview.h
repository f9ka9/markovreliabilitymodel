#ifndef STATEVIEW_H
#define STATEVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QPoint>

class QWheelEvent;

class StateView : public QGraphicsView
{
public:
    explicit StateView(QWidget* parent = nullptr);
    ~StateView();

protected:
    void scrollContentsBy(int dx, int dy) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void expandSceneIfNeeded();

    bool panning{false};
    QPoint lastPanPoint;
};

#endif // STATEVIEW_H
