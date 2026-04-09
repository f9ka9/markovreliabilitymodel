#ifndef STRUCTUREVIEW_H
#define STRUCTUREVIEW_H

#include <QGraphicsView>
#include <QLabel>
#include <QMouseEvent>
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

protected:
    void scrollContentsBy(int dx, int dy) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QLabel* coord{nullptr};
};


#endif // STRUCTUREVIEW_H
