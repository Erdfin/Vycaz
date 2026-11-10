#include "machineitem.h"
#include "portitem.h"
#include "connectionitem.h"
#include "microtexruntime.h"

#include "core/formula.h"
#include "latex.h"
#include "platform/qt/graphic_qt.h"

#include <QPainter>
#include <QFontInfo>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QTextDocument>
#include <QTextCursor>

static qreal snap(qreal val, qreal grid) { return qRound(val / grid) * grid; }

static tex::color toTexColor(const QColor& color)
{
    return (tex::color(color.alpha()) << 24)
         | (tex::color(color.red()) << 16)
         | (tex::color(color.green()) << 8)
         | tex::color(color.blue());
}

MachineItem::MachineItem(const Prefs *prefs, const MachineDefinition& def)
    : definition(def)
    , prefs(prefs)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

    if (!definition.iconPath.isEmpty())
        icon.load(definition.iconPath);

    size = QSize(snap(definition.width, prefs->gridSize), snap(definition.height, prefs->gridSize));

    createPorts();
    updateOrientation();
    rebuildLabelRender();
}

MachineItem::MachineItem(const Prefs *prefs, const MachineItem& temp)
    : definition(temp.definition)
    , prefs(prefs)
    , label(temp.getLabel())
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setAcceptHoverEvents(true);
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsScenePositionChanges);

    if (!definition.iconPath.isEmpty())
        icon.load(definition.iconPath);

    QSizeF temp_size = temp.getSize();
    size = QSize(snap(temp_size.width(), prefs->gridSize), snap(temp_size.height(), prefs->gridSize));

    for (const PortItem* pi : std::as_const(temp.portMap))
    {
        addPort(*pi->getDefinition(), "");
    }
    updateOrientation();
    rebuildLabelRender();
}

MachineItem::~MachineItem()
{
    delete labelRender;
}

QRectF MachineItem::boundingRect() const
{
    // Add a small margin so the resize handles are fully inside the bounding rect
    return QRectF(-HandleSize, -HandleSize,
                  size.width() + 2 * HandleSize,
                  size.height() + 2 * HandleSize);
}

void MachineItem::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem*,
                        QWidget*)
{
    if (!view) {
        if (!scene()->views().isEmpty())
            view = scene()->views().first();
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF body(0, 0, size.width(), size.height());

    // Body
    painter->setBrush(definition.id == "node" ? prefs->nodeColor : prefs->topicColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(body, 6, 6);

    // Icon or label-fallback
    if (!icon.isNull()) {
        QRectF iconRect(20, 20, size.width() - 40, size.height() - 40);
        painter->drawPixmap(iconRect, icon, icon.rect());
    } else if (!textEditor) {
        if (labelRenderDirty) {
            rebuildLabelRender();
        }

        if (labelIsLatex() && labelRender != nullptr) {
            // MicroTeX's Qt backend resets the painter transform at the end of
            // a render. QGraphicsView here uses DontSavePainterState, so keep
            // our own save/restore around the formula.
            painter->save();
            const int x = qRound((size.width() - labelRender->getWidth()) / 2.0);
            const int y = qRound((size.height() - labelRender->getHeight()) / 2.0);
            tex::Graphics2D_qt graphics(painter);
            labelRender->draw(graphics, x, y);
            painter->restore();
        } else {
            painter->setPen(QPen(prefs->fontColor));
            painter->setFont(prefs->font);
            QRectF labelRect(8, 8, size.width() - 16, size.height() - 16);
            painter->drawText(labelRect, Qt::AlignCenter | Qt::TextWordWrap, label);
        }
    }

    if (isSelected()) {
        painter->setPen(QPen(prefs->highlightedColor, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(body, 6, 6);
    }
}

// Returns which handle (edge / corner) contains the given local position.
MachineItem::ResizeHandle MachineItem::handleAt(const QPointF& p) const
{
    const qreal h = HandleSize;
    const qreal w = size.width();
    const qreal ht = size.height();

    const bool onLeft   = p.x() >= 0   && p.x() <= h;
    const bool onRight  = p.x() >= w - h && p.x() <= w;
    const bool onTop    = p.y() >= 0   && p.y() <= h;
    const bool onBottom = p.y() >= ht - h && p.y() <= ht;

    if (onTop    && onLeft)  return ResizeHandle::TopLeft;
    if (onTop    && onRight) return ResizeHandle::TopRight;
    if (onBottom && onLeft)  return ResizeHandle::BottomLeft;
    if (onBottom && onRight) return ResizeHandle::BottomRight;
    if (onLeft)              return ResizeHandle::Left;
    if (onRight)             return ResizeHandle::Right;
    if (onTop)               return ResizeHandle::Top;
    if (onBottom)            return ResizeHandle::Bottom;

    return ResizeHandle::None;
}

Qt::CursorShape MachineItem::cursorForHandle(ResizeHandle h) const
{
    switch (h) {
    case ResizeHandle::Left:        return Qt::SizeHorCursor;
    case ResizeHandle::Right:       return Qt::SizeHorCursor;
    case ResizeHandle::Top:         return Qt::SizeVerCursor;
    case ResizeHandle::Bottom:      return Qt::SizeVerCursor;
    case ResizeHandle::TopLeft:     return Qt::SizeFDiagCursor;
    case ResizeHandle::BottomRight: return Qt::SizeFDiagCursor;
    case ResizeHandle::TopRight:    return Qt::SizeBDiagCursor;
    case ResizeHandle::BottomLeft:  return Qt::SizeBDiagCursor;
    default:                        return Qt::ArrowCursor;
    }
}

void MachineItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    ResizeHandle h = handleAt(event->pos());
    setCursor(cursorForHandle(h));
    QGraphicsObject::hoverMoveEvent(event);
}

void MachineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    setCursor(Qt::ArrowCursor);
    QGraphicsObject::hoverLeaveEvent(event);
}

void MachineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        activeHandle = handleAt(event->pos());
        if (activeHandle != ResizeHandle::None) {
            // Begin resize: capture initial geometry in scene coords
            dragStartScene   = event->scenePos();
            dragStartRect    = QRectF(QPointF(0, 0), size);
            dragStartItemPos = scenePos();
            event->accept();
            return; // Don't pass to base – suppress move behaviour
        }
    }
    QGraphicsObject::mousePressEvent(event);
}

void MachineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (activeHandle == ResizeHandle::None) {
        QGraphicsObject::mouseMoveEvent(event);
        return;
    }

    const QPointF delta = event->scenePos() - dragStartScene;

    // Start from the original geometry
    qreal x = dragStartItemPos.x();
    qreal y = dragStartItemPos.y();
    qreal w = dragStartRect.width();
    qreal h = dragStartRect.height();

    // Adjust edges based on which handle is active
    switch (activeHandle) {
    case ResizeHandle::Left:
        x += delta.x();
        w -= delta.x();
        break;
    case ResizeHandle::Right:
        w += delta.x();
        break;
    case ResizeHandle::Top:
        y += delta.y();
        h -= delta.y();
        break;
    case ResizeHandle::Bottom:
        h += delta.y();
        break;
    case ResizeHandle::TopLeft:
        x += delta.x(); w -= delta.x();
        y += delta.y(); h -= delta.y();
        break;
    case ResizeHandle::TopRight:
        w += delta.x();
        y += delta.y(); h -= delta.y();
        break;
    case ResizeHandle::BottomLeft:
        x += delta.x(); w -= delta.x();
        h += delta.y();
        break;
    case ResizeHandle::BottomRight:
        w += delta.x();
        h += delta.y();
        break;
    default:
        break;
    }

    // Enforce minimum size
    if (w < MinSize) {
        // Clamp: don't let the left edge creep past the right
        if (activeHandle == ResizeHandle::Left    ||
            activeHandle == ResizeHandle::TopLeft  ||
            activeHandle == ResizeHandle::BottomLeft)
        {
            x = dragStartItemPos.x() + dragStartRect.width() - MinSize;
        }
        w = MinSize;
    }
    if (h < MinSize) {
        if (activeHandle == ResizeHandle::Top     ||
            activeHandle == ResizeHandle::TopLeft  ||
            activeHandle == ResizeHandle::TopRight)
        {
            y = dragStartItemPos.y() + dragStartRect.height() - MinSize;
        }
        h = MinSize;
    }

    if (prefs->snapToGrid) {
        x = snap(x, prefs->gridSize);
        y = snap(y, prefs->gridSize);
        w = snap(w, prefs->gridSize);
        h = snap(h, prefs->gridSize);
    }

    if (w < MinSize) w = MinSize;
    if (h < MinSize) h = MinSize;

    // Apply
    prepareGeometryChange();
    size = QSizeF(w, h);
    setPos(x, y);
    updateOrientation();

    event->accept();
}

void MachineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (activeHandle != ResizeHandle::None) {
        activeHandle = ResizeHandle::None;
        event->accept();
        return;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

void MachineItem::createPorts()
{
    for (const PortDefinition& pd : definition.inputs) { addPort(pd); }
    for (const PortDefinition& pd : definition.outputs) { addPort(pd); }
}

void MachineItem::addPort(PortDefinition def, QString id)
{
    auto *port = new PortItem(
        def.type,
        this,
        new PortDefinition(def),
        id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id
        );
    portsList[def.side].push_back(port);
    portMap[port->instanceId] = port;
}

void MachineItem::updateOrientation()
{
    // For each side, spacing is determined by the dimension along that side
    const qreal W = size.width();
    const qreal H = size.height();

    const int nLeft   = portsList[PortSide::Left].size();
    const int nRight  = portsList[PortSide::Right].size();
    const int nTop    = portsList[PortSide::Top].size();
    const int nBottom = portsList[PortSide::Bottom].size();

    // Each side divides its own dimension evenly among (n + 1) slots
    const qreal leftSpacing   = nLeft   > 0 ? H / (nLeft   + 1) : 0;
    const qreal rightSpacing  = nRight  > 0 ? H / (nRight  + 1) : 0;
    const qreal topSpacing    = nTop    > 0 ? W / (nTop    + 1) : 0;
    const qreal bottomSpacing = nBottom > 0 ? W / (nBottom + 1) : 0;

    for (int i = 0; i < nLeft; i++)
        portsList[PortSide::Left][i]->setPos(0, (i + 1) * leftSpacing);

    for (int i = 0; i < nRight; i++)
        portsList[PortSide::Right][i]->setPos(W, (i + 1) * rightSpacing);

    for (int i = 0; i < nTop; i++)
        portsList[PortSide::Top][i]->setPos((i + 1) * topSpacing, 0);

    for (int i = 0; i < nBottom; i++)
        portsList[PortSide::Bottom][i]->setPos((i + 1) * bottomSpacing, H);
}

void MachineItem::flipMachineH()
{
    orientation = (orientation + 2) % 4;
    qSwap(portsList[PortSide::Left], portsList[PortSide::Right]);
    for (PortItem* p : std::as_const(portsList[PortSide::Left]))  p->setSide(PortSide::Left);
    for (PortItem* p : std::as_const(portsList[PortSide::Right])) p->setSide(PortSide::Right);
    updateOrientation();
}

void MachineItem::flipMachineV()
{
    orientation = (orientation + 2) % 4;
    qSwap(portsList[PortSide::Top], portsList[PortSide::Bottom]);
    for (PortItem* p : std::as_const(portsList[PortSide::Top]))    p->setSide(PortSide::Top);
    for (PortItem* p : std::as_const(portsList[PortSide::Bottom])) p->setSide(PortSide::Bottom);
    updateOrientation();
}

void MachineItem::rotateMachine()
{
    orientation = (orientation + 1) % 4;
    qSwap(portsList[PortSide::Left], portsList[PortSide::Top]);
    qSwap(portsList[PortSide::Left], portsList[PortSide::Right]);
    qSwap(portsList[PortSide::Left], portsList[PortSide::Bottom]);
    for (PortItem* p : std::as_const(portsList[PortSide::Left]))   p->setSide(PortSide::Left);
    for (PortItem* p : std::as_const(portsList[PortSide::Right]))  p->setSide(PortSide::Right);
    for (PortItem* p : std::as_const(portsList[PortSide::Top]))    p->setSide(PortSide::Top);
    for (PortItem* p : std::as_const(portsList[PortSide::Bottom])) p->setSide(PortSide::Bottom);
    updateOrientation();
}

void MachineItem::setSize(QSizeF new_size)
{
    prepareGeometryChange();
    size = new_size;
    updateOrientation();
    labelRenderDirty = true;
}

bool MachineItem::labelIsLatex() const
{
    return label.size() >= 2
        && label.startsWith(QLatin1Char('$'))
        && label.endsWith(QLatin1Char('$'));
}

void MachineItem::setLabel(const QString& text)
{
    label = text;
    labelRenderDirty = true;
    update();
}

void MachineItem::rebuildLabelRender()
{
    labelRenderDirty = false;
    delete labelRender;
    labelRender = nullptr;

    if (!labelIsLatex() || size.width() <= 32.0 || !ensureMicroTeXInitialized()) {
        return;
    }

    const QString source = label.mid(1, label.size() - 2);
    if (source.trimmed().isEmpty()) {
        return;
    }

    // MicroTeX sizes are in pixels-per-em, not Qt points. Matching points to
    // pixels makes formulas ~25% too small (72 vs 96 DPI). Use the font's
    // actual pixel size so math lines up with the normal label text.
    qreal textSize = QFontInfo(prefs->font).pixelSize();
    if (textSize <= 0.0) {
        textSize = prefs->font.pointSizeF() * 4.0 / 3.0;
    }
    if (textSize <= 0.0) {
        textSize = 18.0;
    }
    textSize *= 1.35;
    textSize = qBound(10.0, textSize, 64.0);

    try {
        tex::Formula formula(source.toStdWString());
        tex::TeXRenderBuilder builder;
        labelRender = builder
            .setStyle(tex::TexStyle::display)
            .setTextSize(static_cast<float>(textSize))
            .setType(tex::TeXRenderBuilder::BOLD)
            .setWidth(tex::UnitType::pixel,
                      qMax(20, static_cast<int>(size.width() - 16.0)),
                      tex::Alignment::left)
            .setIsMaxWidth(true)
            .setLineSpace(tex::UnitType::pixel,
                          static_cast<float>(textSize * 0.35))
            .setForeground(toTexColor(prefs->fontColor))
            .build(formula);
    } catch (...) {
        labelRender = nullptr;
    }
}

void MachineItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* duplicate = menu.addAction("Duplicate");
    QAction* flipH     = menu.addAction("Flip Horizontally");
    QAction* flipV     = menu.addAction("Flip Vertically");
    QAction* rotate    = menu.addAction("Rotate");
    QAction* configure = menu.addAction("Configure");
    QAction* remove    = menu.addAction("Delete");

    QAction* selected = menu.exec(event->screenPos());
    if (!selected) return;

    if (selected == remove) {
        if (scene()) scene()->removeItem(this);
        this->deleteLater();
    } else if (selected == configure) {
        emit machineConfigureRequested(this);
    } else if (selected == flipH) {
        flipMachineH();
    } else if (selected == flipV) {
        flipMachineV();
    } else if (selected == rotate) {
        rotateMachine();
    } else if (selected == duplicate) {
        emit createDuplicate(this);
    }
}

void MachineItem::clearPorts() {
    const auto ports = portMap.values();
    for (PortItem* port : ports) delete port;

    portMap.clear();
    portsList.clear();
}

QPointF MachineItem::textEditorPos(QRectF tb)
{
    return QPointF(
        (size.width()  - tb.width())  / 2,
        (size.height() - tb.height()) / 2
    );
}

void MachineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (icon.isNull()) {
        // Create a temporary QGraphicsTextItem for inline editing
        if (!textEditor) {
            textEditor = new QGraphicsTextItem(this);
            textEditor->setPlainText(label);
            textEditor->setDefaultTextColor(prefs->fontColor);
            textEditor->setFont(prefs->font);
            textEditor->setTextInteractionFlags(Qt::TextEditorInteraction);
            textEditor->setZValue(zValue() + 1);

            QTextOption textOption;
            textOption.setAlignment(Qt::AlignCenter);
            textEditor->document()->setDefaultTextOption(textOption);
            textEditor->setTextWidth(size.width() - 16);

            QRectF tb = textEditor->boundingRect();
            textEditor->setPos(textEditorPos(tb));

            // Commit the edit when focus is lost
            connect(textEditor->document(), &QTextDocument::contentsChanged,
                    this, [this]() {
                        label = textEditor->toPlainText();
                        labelRenderDirty = true;
                        // Center it within the body as text it's being changed
                        QRectF tb = textEditor->boundingRect();
                        textEditor->setPos(textEditorPos(tb));
                    });

            // Use an event filter to detect focus-out and clean up
            textEditor->installEventFilter(this);

            update();
        }
        textEditor->setFocus();
        textEditor->setTextCursor(
            QTextCursor(textEditor->document()->find(label))
            );
        setSelected(true);
        event->accept();
        return;
    }
    QGraphicsObject::mouseDoubleClickEvent(event);
}

bool MachineItem::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == textEditor && e->type() == QEvent::FocusOut) {
        label = textEditor->toPlainText();
        labelRenderDirty = true;
        textEditor->deleteLater();
        textEditor = nullptr;
        update();
    }
    return QGraphicsObject::eventFilter(obj, e);
}

QVariant MachineItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange && scene() && prefs->snapToGrid) {
        const QPointF p = value.toPointF();
        return QPointF(snap(p.x(), prefs->gridSize), snap(p.y(), prefs->gridSize));
    }
    return QGraphicsObject::itemChange(change, value);
}
