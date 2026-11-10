#include "graphicsview.h"
#include <QDrag>
#include <QMimeData>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>


GraphicsView::GraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
    setDragMode(NoDrag);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::TextAntialiasing);


    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void GraphicsView::startFactoryDrag(const QString& machineId)
{
    QDrag* drag = new QDrag(this);
    QMimeData* mime = new QMimeData;

    mime->setText(machineId);
    drag->setMimeData(mime);

    drag->exec(Qt::CopyAction);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
        event->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void GraphicsView::dropEvent(QDropEvent* event)
{
    const QString machineId = event->mimeData()->text();
    QPointF scenePos = mapToScene(event->position().toPoint());

    emit factoryDropped(machineId, scenePos);

    event->acceptProposedAction();
}

// --------------------------------------------------
// Zoom
// --------------------------------------------------
void GraphicsView::wheelEvent(QWheelEvent* event)
{
    const double factor = 1.15;
    double scaleFactor = (event->angleDelta().y() > 0) ? factor : 1.0 / factor;

    double newScale = currentScale * scaleFactor;
    if (newScale < minScale || newScale > maxScale)
        return;

    scale(scaleFactor, scaleFactor);
    currentScale = newScale;
}

// --------------------------------------------------
// Pan
// --------------------------------------------------
void GraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        panning = true;
        lastMousePos = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        // If we clicked on any item, let the scene handle it (port connections,
        // item moves, etc.). Only use rubber-band when clicking empty space.
        QGraphicsItem* item = itemAt(event->pos());
        if (item)
            setDragMode(NoDrag);
        else
            setDragMode(RubberBandDrag);
    }

    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (panning) {
        QPoint delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        panning = false;
        setCursor(Qt::ArrowCursor);
        viewport()->unsetCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);

    // Always restore rubber-band mode after any left-button release
    if (event->button() == Qt::LeftButton)
        setDragMode(RubberBandDrag);
}
