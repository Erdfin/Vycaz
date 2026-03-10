#pragma once

#include "preferencesdefinition.h"
#include "machineitem.h"

#include <QGraphicsObject>
#include <QGraphicsTextItem>

class ContainerItem : public MachineItem
{
    Q_OBJECT

public:
    explicit ContainerItem(const Prefs *prefs);

    enum { Type = QGraphicsObject::UserType + 4 };
    int type() const override { return Type; }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    const Prefs *prefs;

    // Minimum allowed size during resize
    static constexpr qreal MinSize = 25.0;
    // Width of the grab handle band along each edge / corner
    static constexpr qreal HandleSize = 10.0;

private:
    qreal labelHeight = 30;

protected:
    QPointF textEditorPos(QRectF tb) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
};
