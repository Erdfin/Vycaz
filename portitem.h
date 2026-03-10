#pragma once

#include <QGraphicsEllipseItem>
#include <QVector>

#include "portdefinition.h"

class ConnectionItem;
class MachineItem;

PortSide stringToPortSide(QString side);

class PortItem : public QGraphicsEllipseItem
{
public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    static constexpr qreal Radius = 10.0;
    static constexpr qreal HitRadius = 24.0;

    PortItem(PortType type, MachineItem* parentMachine, const PortDefinition *def, QString instanceId);
    ~PortItem();

    PortType getPortType() const { return portType; }
    void setPortType(PortType newType);
    QPointF centerPos() const;

    void addConnection(ConnectionItem* c);
    void removeConnection(ConnectionItem* c);

    int connectionCount() const;

    MachineItem* getParentMachine() const { return parentMachine; }

    const PortDefinition *getDefinition() const { return definition; }

    const QList<ConnectionItem*>& get_connections() const { return connections; }

    PortSide stringToPortSide(QString side);

    PortSide side;
    void setSide(PortSide newSide);

    QString instanceId;
protected:
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant& value) override;

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    MachineItem* parentMachine;
    const PortDefinition* definition;
    PortType portType;
    QVector<ConnectionItem*> connections;
    const QColor* color;
};
