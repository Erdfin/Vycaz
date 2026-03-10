#pragma once

#include "preferencesdefinition.h"

#include <QGraphicsScene>
#include <tuple>

class PortItem;
class ConnectionItem;
class MachineItem;
class ContainerItem;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GraphicsScene(const Prefs *prefs, QObject* parent = nullptr);
    QList<MachineItem*> machineItemList() const;
    QList<ConnectionItem*> connectionItemList() const;
    QList<ContainerItem*> containersItemList() const;
    std::tuple<QList<MachineItem*>, QList<ConnectionItem*>, QList<ContainerItem*>> getItemsLists() const;
    ConnectionItem* tempConnection = nullptr;

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent*) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;

private:
    PortItem* startPort = nullptr;
    const Prefs *prefs;
};
