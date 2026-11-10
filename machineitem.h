#pragma once

#include "machinedefinition.h"

#include "preferencesdefinition.h"

#include <QGraphicsObject>
#include <QGraphicsView>
#include <QPixmap>
#include <QUuid>

class PortItem;

namespace tex {
class TeXRender;
}

class MachineItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit MachineItem(const Prefs *prefs, const MachineDefinition& def);
    explicit MachineItem(const Prefs *prefs, const MachineItem& temp);
    ~MachineItem() override;

    enum { Type = QGraphicsObject::UserType + 1 };
    int type() const override { return Type; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    void flipMachineH();
    void flipMachineV();
    void rotateMachine();
    int orientation = 0;

    void setSize(QSizeF new_size);

    const QString& getName() const { return definition.name; }
    QSizeF getSize() const { return size; }

    QGraphicsView *view = nullptr;
    const MachineDefinition& definition;
    QString instanceId;

    // Minimum allowed size during resize
    static constexpr qreal MinSize = 25.0;
    // Width of the grab handle band along each edge / corner
    static constexpr qreal HandleSize = 10.0;

    QHash<QString, PortItem*> portMap;
    QHash<PortSide, QVector<PortItem*>> portsList;
    void updateOrientation();

    void addPort(PortDefinition def, QString id = "");
    void clearPorts();

    void setLabel(const QString& text);
    const QString& getLabel() const { return label; }

    bool labelIsLatex() const;

    const Prefs *prefs;

private:
    void createPorts();
    void rebuildLabelRender();

    // Which resize handle (edge/corner) is being dragged
    enum class ResizeHandle {
        None,
        Left, Right, Top, Bottom,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    ResizeHandle handleAt(const QPointF& localPos) const;
    Qt::CursorShape cursorForHandle(ResizeHandle h) const;

    // Resize state
    ResizeHandle activeHandle  = ResizeHandle::None;
    QPointF      dragStartScene;   // scene position where drag began
    QRectF       dragStartRect;    // item rect (origin = item pos) at drag start
    QPointF      dragStartItemPos; // item's scene position at drag start

signals:
    void machineConfigureRequested(MachineItem* self);
    void createDuplicate(MachineItem* self);

protected:
    QGraphicsTextItem* textEditor = nullptr;
    QString label = "Label";
    QPixmap icon;
    QSizeF size;
    tex::TeXRender* labelRender = nullptr;
    bool labelRenderDirty = true;

    virtual QPointF textEditorPos(QRectF tb);

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
};
