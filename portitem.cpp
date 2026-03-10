#include "portitem.h"
#include "connectionitem.h"
#include "machineitem.h"

#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QSvgGenerator>

PortItem::PortItem(PortType t, MachineItem* parent, const PortDefinition* def, QString instanceId)
    : QGraphicsEllipseItem(-Radius, -Radius, Radius * 2, Radius * 2, parent)
    , portType(t)
    , instanceId(instanceId)
{
    color = portType == PortType::Input ? &parent->prefs->inputColor : &parent->prefs->outputColor;

    setPen(QPen(Qt::NoPen));
    setZValue(10);

    setFlag(ItemSendsScenePositionChanges, true);
    setAcceptHoverEvents(true);

    parentMachine = parent;
    definition = def;
    side = def->side;
}

PortItem::~PortItem() {
    for (ConnectionItem *conn: std::as_const(connections)) {
        delete conn;
    }
}

void PortItem::setPortType(PortType newType) {
    portType = newType;
    color = portType == PortType::Input ? &parentMachine->prefs->inputColor : &parentMachine->prefs->outputColor;
}

QPointF PortItem::centerPos() const
{
    return mapToScene(QPointF(0, 0));
}

void PortItem::addConnection(ConnectionItem* c)
{
    if (!connections.contains(c))
        connections.push_back(c);
}

void PortItem::removeConnection(ConnectionItem* c)
{
    connections.removeAll(c);
}

QVariant PortItem::itemChange(GraphicsItemChange change,
                              const QVariant& value)
{
    if (change == ItemScenePositionHasChanged) {
        for (ConnectionItem* c : std::as_const(connections))
            c->updatePath();
    }

    return QGraphicsEllipseItem::itemChange(change, value);
}

QPainterPath PortItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QPointF(0, 0), HitRadius, HitRadius);
    return path;
}

int PortItem::connectionCount() const
{
    return connections.size();
}

PortSide stringToPortSide(QString side)
{
    if (side == "left")
    {
        return PortSide::Left;
    } else if (side == "right")
    {
        return PortSide::Right;
    } else if (side == "top")
    {
        return PortSide::Top;
    } else if (side == "bottom")
    {
        return PortSide::Bottom;
    } else
    {
        return PortSide::Left;
    }
}

void PortItem::setSide(PortSide newSide)
{
    if (side == newSide)
        return;

    side = newSide;

    for (ConnectionItem* c : std::as_const(connections))
        c->updatePath();
}

void PortItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setBrush(QBrush(*color));
    // Skip rendering when exporting to SVG
    if (dynamic_cast<QSvgGenerator*>(painter->device()))
        return;
    QGraphicsEllipseItem::paint(painter, option, widget);
}

void PortItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsEllipseItem::hoverMoveEvent(event);
}
