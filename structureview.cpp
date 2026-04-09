#include "structureview.h"

StructureView::StructureView(QWidget* parent): QGraphicsView(parent), coord(nullptr)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setMouseTracking(true);
    setupCoordLabel();
}

StructureView::~StructureView()
{}

void StructureView::updateCursorCoordinatesLabel()
{
    QPointF pos = mapToScene(mapFromGlobal(QCursor::pos()));
    coord->setText(QString("X: %1  Y: %2").arg(pos.x(), 0 ,'f', 0).arg(pos.y(), 0 ,'f', 0));
    coord->adjustSize();
    updateCoordLabelPosition();
}

void StructureView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx,dy);
    if (!scene()) return;

    expandSceneIfNeeded();
    updateCursorCoordinatesLabel();

}

void StructureView::updateCoordLabelPosition(){coord->move(viewport()->width() - coord->width() - 10, viewport()->height() - coord->height() - 10);}

void StructureView::expandSceneIfNeeded()
{
    QRectF rect = scene()->sceneRect();
    QPointF center = mapToScene(viewport()->rect().center());

    double margin = 500.0/transform().m11();
    double expand = 5000.0/transform().m11();

    bool changed = false;
    if (center.x() > rect.right() - margin)
    {
        rect.setRight(rect.right() + expand);
        changed = true;
    }
    if (center.x() < rect.left() + margin)
    {
        rect.setLeft(rect.left() - expand);
        changed = true;
    }
    if (center.y() > rect.bottom() - margin)
    {
        rect.setBottom(rect.bottom() + expand);
        changed = true;
    }
    if (center.y() < rect.top() + margin)
    {
        rect.setTop(rect.top() - expand);
        changed = true;
    }

    if (changed) scene()->setSceneRect(rect);

}

void StructureView::setupCoordLabel()
{
    coord = new QLabel(viewport());
    coord->setText("X: 0  Y: 0");
    coord->adjustSize();
    updateCoordLabelPosition();
}

void StructureView::mouseMoveEvent(QMouseEvent* event)
{
    updateCursorCoordinatesLabel();
    QGraphicsView::mouseMoveEvent(event);
}

void StructureView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    updateCoordLabelPosition();
}

void StructureView::wheelEvent(QWheelEvent* event)
{

    if (!(event->modifiers() & Qt::ControlModifier)) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    constexpr double scaleFactor = 1.15;

    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}