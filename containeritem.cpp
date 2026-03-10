#include "containeritem.h"
#include "machinedefinition.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QTextDocument>
#include <QCursor>
#include <QMenu>
#include <QTextCursor>

static qreal snap(qreal val, qreal grid) { return qRound(val / grid) * grid; }

ContainerItem::ContainerItem(const Prefs *prefs)
    : MachineItem(prefs, MachineDefinition{
                             .id="container",
                             .name="Container",
                             .iconPath="",
                             .type=MachineType::Machine,
                             .recipeType=QList<QString>(),
                             .kineticStress=0,
                             .speedRequirement=0,
                             .height=100,
                             .width=200,
                             .inputs=QVector<PortDefinition>(),
                             .outputs=QVector<PortDefinition>(),
                             .acceptedInputs=QSet<QString>(),
                             .possibleOutputs=QSet<QString>(),
                             .recipes=MachineRecipe()})
    , prefs(prefs)
{
    setZValue(-2);
}

QPointF ContainerItem::textEditorPos(QRectF tb)
{
    return QPointF(
        (size.width()  - tb.width())  / 2,
        (labelHeight   - tb.height()) / 2
    );
}

void ContainerItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    // Body
    const QRectF body(0, 0, size.width(), size.height());
    painter->setBrush(prefs->contBodyColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(body, 6, 6);

    // Header band
    QRectF header(0, 0, size.width(), labelHeight);
    painter->setBrush(prefs->contHeadColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(header, 6, 6);
    painter->drawRect(QRectF(0, 6, header.width(), header.height()-6));

    // Draw label text (only when the inline editor is not active)
    if (!textEditor) {
        painter->setPen(QPen(prefs->fontColor));
        painter->setFont(prefs->font);
        painter->drawText(header, Qt::AlignCenter | Qt::TextWordWrap, label);
    }

    if (isSelected()) {
        painter->setPen(QPen(prefs->highlightedColor, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(body, 6, 6);
    }
}

void ContainerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* remove = menu.addAction("Delete");

    QAction* selected = menu.exec(event->screenPos());
    if (!selected)
        return;

    if (selected == remove)
        this->deleteLater();
}
