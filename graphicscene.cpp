#include "graphicscene.h"
#include "portitem.h"
#include "connectionitem.h"
#include "machineitem.h"
#include "containeritem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QtMath>

GraphicsScene::GraphicsScene(const Prefs *prefs, QObject* parent)
    : QGraphicsScene(parent)
    , prefs(prefs)
{}

void GraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, prefs->bcColor);

    if (prefs->showGrid) {
        // ------------------------------------
        // Determine zoom level (LOD)
        // ------------------------------------
        const qreal scale = painter->transform().m11();

        int majorGridFactor = prefs->majorGridFactor;

        // Grid spacing jumps at thresholds
        qreal gridSize = prefs->gridSize;

        if (scale < 0.15)
            gridSize *= 8;
        else if (scale < 0.30)
            gridSize *= 4;
        else if (scale < 0.60)
            gridSize *= 2;
        // else: base grid

        // ------------------------------------
        // Align grid to world coordinates
        // ------------------------------------
        qreal left   = std::floor(rect.left()   / gridSize) * gridSize;
        qreal right  = std::ceil (rect.right()  / gridSize) * gridSize;
        qreal top    = std::floor(rect.top()    / gridSize) * gridSize;
        qreal bottom = std::ceil (rect.bottom() / gridSize) * gridSize;

        // ------------------------------------
        // Pens (cosmetic = stable thickness)
        // ------------------------------------
        QPen minorPen(prefs->subGridColor);
        minorPen.setWidthF(0.0); // cosmetic

        QPen majorPen(prefs->gridColor.lighter(115));
        majorPen.setWidthF(0.0); // also cosmetic

        // Number of vertical lines
        const int firstX = static_cast<int>(std::floor(left / gridSize));
        const int lastX  = static_cast<int>(std::ceil (right / gridSize));

        // Vertical grid lines
        for (int i = firstX; i <= lastX; ++i) {
            const qreal x = i * gridSize;

            painter->setPen(i % majorGridFactor ? minorPen : majorPen);
            painter->drawLine(QLineF(x, top, x, bottom));
        }

        // Number of horizontal lines
        const int firstY = static_cast<int>(std::floor(top / gridSize));
        const int lastY  = static_cast<int>(std::ceil (bottom / gridSize));

        // Horizontal grid lines
        for (int j = firstY; j <= lastY; ++j) {
            const qreal y = j * gridSize;

            painter->setPen(j % majorGridFactor ? minorPen : majorPen);
            painter->drawLine(QLineF(left, y, right, y));
        }
    }

}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    auto* item = itemAt(e->scenePos(), QTransform());
    auto* port = dynamic_cast<PortItem*>(item);

    if (port && port->getPortType() == PortType::Output) {
        startPort = port;
        tempConnection = new ConnectionItem(port);

        tempConnection->setEndPos(e->scenePos());

        addItem(tempConnection);
        return;
    }

    QGraphicsScene::mousePressEvent(e);
}


void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (tempConnection) {
        tempConnection->setEndPos(e->scenePos());
        return;
    }
    QGraphicsScene::mouseMoveEvent(e);
}


void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (!tempConnection) {
        QGraphicsScene::mouseReleaseEvent(e);
        return;
    }

    auto* item = itemAt(e->scenePos(), QTransform());
    auto* port = dynamic_cast<PortItem*>(item);

    bool connected = false;

    if (port) {
        tempConnection->setTargetPort(port);
        connected = true;
    }

    if (!connected) {
        removeItem(tempConnection);
        delete tempConnection;
    }

    tempConnection = nullptr;
    startPort = nullptr;
}

QList<MachineItem*> GraphicsScene::machineItemList() const
{
    QList<MachineItem*> result;
    const auto graphicItems = items(Qt::AscendingOrder);
    for (QGraphicsItem* item : graphicItems) {
        if (auto* m = qgraphicsitem_cast<MachineItem*>(item))
            result.push_back(m);
    }
    return result;
}

QList<ConnectionItem*> GraphicsScene::connectionItemList() const
{
    QList<ConnectionItem*> result;
    const auto graphicItems = items(Qt::AscendingOrder);
    for (QGraphicsItem* item : graphicItems) {
        if (auto* c = qgraphicsitem_cast<ConnectionItem*>(item))
            result.push_back(c);
    }
    return result;
}

QList<ContainerItem*> GraphicsScene::containersItemList() const
{
    QList<ContainerItem*> result;
    const auto graphicItems = items(Qt::AscendingOrder);
    for (QGraphicsItem* item : graphicItems) {
        if (auto* c = qgraphicsitem_cast<ContainerItem*>(item))
            result.push_back(c);
    }
    return result;
}

std::tuple<QList<MachineItem*>, QList<ConnectionItem*>, QList<ContainerItem*>> GraphicsScene::getItemsLists() const
{
    QList<MachineItem*> machines;
    QList<ConnectionItem*> connections;
    QList<ContainerItem*> containers;

    const auto graphicItems = items(Qt::AscendingOrder);
    for (QGraphicsItem* item : graphicItems) {
        if (auto* c = qgraphicsitem_cast<MachineItem*>(item)) {
            machines.push_back(c);
        } else if (auto* c = qgraphicsitem_cast<ConnectionItem*>(item)) {
            connections.push_back(c);
        } else if (auto* c = qgraphicsitem_cast<ContainerItem*>(item)) {
            containers.push_back(c);
        }
    }

    return std::tuple(machines, connections, containers);
}
