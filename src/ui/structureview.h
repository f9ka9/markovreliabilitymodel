#ifndef STRUCTUREVIEW_H
#define STRUCTUREVIEW_H

#include <QGraphicsView>
#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QResizeEvent>


class StructureView : public QGraphicsView
{
public:
    StructureView(QWidget* parent = nullptr);
    ~StructureView();

    void setupCoordLabel();
    void updateCoordLabelPosition();
    void updateCursorCoordinatesLabel();
    void expandSceneIfNeeded();
    void setPanningEnabled(bool enabled);

protected:
    void scrollContentsBy(int dx, int dy) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QLabel* coord{nullptr};
    bool panningEnabled{true};
    bool panning{false};
    QPoint lastPanPoint;
};


#endif // STRUCTUREVIEW_H
