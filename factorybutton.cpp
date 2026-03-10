#include "factorybutton.h"
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>

FactoryButton::FactoryButton(int factoryIndex, QWidget* parent)
    : QToolButton(parent), index(factoryIndex)
{
    setCheckable(false);
}

void FactoryButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        dragStart = event->pos();

    QToolButton::mousePressEvent(event);
}

void FactoryButton::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - dragStart).manhattanLength()
        < QApplication::startDragDistance())
        return;

    auto* drag = new QDrag(this);
    auto* mime = new QMimeData;

    mime->setData("application/x-factory-index",
                  QByteArray::number(index));

    drag->setMimeData(mime);
    drag->setPixmap(icon().pixmap(48, 48));
    drag->setHotSpot(QPoint(24, 24));

    drag->exec(Qt::CopyAction);

    setDown(false);
    clearFocus();
}
