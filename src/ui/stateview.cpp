#include "stateview.h"

#include <QScrollBar>
#include <QWheelEvent>

StateView::StateView(QWidget* parent): QGraphicsView(parent)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::ArrowCursor);
    viewport()->setCursor(Qt::ArrowCursor);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setMouseTracking(true);
}

StateView::~StateView()
{}

void StateView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    if (!scene()) return;

    expandSceneIfNeeded();
}

void StateView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        panning = true;
        lastPanPoint = event->pos();
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void StateView::mouseMoveEvent(QMouseEvent* event)
{
    if (panning)
    {
        const QPoint delta = event->pos() - lastPanPoint;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        lastPanPoint = event->pos();
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void StateView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && panning)
    {
        panning = false;
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void StateView::wheelEvent(QWheelEvent* event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        QGraphicsView::wheelEvent(event);
        return;
    }

    constexpr double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0)
        scale(scaleFactor, scaleFactor);
    else
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
}

void StateView::expandSceneIfNeeded()
{
    QRectF rect = scene()->sceneRect();
    const QPointF center = mapToScene(viewport()->rect().center());

    const double margin = 500.0 / transform().m11();
    const double expand = 5000.0 / transform().m11();

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
