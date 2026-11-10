#include "connectionitem.h"
#include "portitem.h"
#include "machineitem.h"
#include "graphicscene.h"

#include <QPen>
#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QDebug>
#include <QSet>

ConnectionItem::ConnectionItem(PortItem* from, PortItem* to)
    : src(from), dst(to)
{
    setZValue(-1); // keep behind machines

    setAcceptedMouseButtons(Qt::RightButton | Qt::LeftButton);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    if (src != nullptr) {
        src->addConnection(this);
        arrowSpacing = src->getParentMachine()->prefs->arrowSpacing;
    }

    if (dst != nullptr)
        dst->addConnection(this);
}

ConnectionItem::~ConnectionItem()
{
    if (src) src->removeConnection(this);
    if (dst) dst->removeConnection(this);
    if (scene()) scene()->removeItem(this);
}

void ConnectionItem::setEndPos(const QPointF& pos)
{
    tempEnd = pos;
    updatePath();
}

void ConnectionItem::setTargetPort(PortItem* p)
{
    if (dst)
        dst->removeConnection(this);

    dst = p;

    if (dst)
        dst->addConnection(this);

    updatePath();
}

QRectF ConnectionItem::boundingRect() const
{
    QRectF r = QGraphicsPathItem::boundingRect();
    qreal w = pen().widthF();
    return r.adjusted(-w, -w, w, w); // expand for pen
}

QPainterPath ConnectionItem::shape() const {
    // Empty shape, makes it "invisible" for rubber band select
    QPainterPathStroker stroker;
    stroker.setWidth(20.0);   // match pen width
    return stroker.createStroke(path());
}

static QPointF sideToDir(PortSide Side)
{
    switch (Side) {
    case PortSide::Right: return QPointF( 1,  0); // Right
    case PortSide::Bottom: return QPointF( 0,  1); // Down
    case PortSide::Left: return QPointF(-1,  0); // Left
    case PortSide::Top: return QPointF( 0, -1); // Up
    }
    return QPointF(1, 0);
}

static QPointF safeNormalize(const QPointF& v)
{
    qreal len = std::hypot(v.x(), v.y());
    return (len > 1e-4) ? (v / len) : QPointF(1, 0);
}

void ConnectionItem::updatePath()
{
    if (!src) return;

    bool srcMoved = true;
    if (sPos == nullptr) {
        sPos = new QPointF(src->centerPos());
    } else {
        if (*sPos == src->centerPos()) srcMoved = false;
    }

    bool dstMoved = true;
    if (dst != nullptr) {
        if (dPos == nullptr) {
            dPos = new QPointF(dst->centerPos());
        } else {
            if (*dPos == dst->centerPos()) dstMoved = false;
        }
    }

    if (!(srcMoved || dstMoved || updateArrows)) return;

    QPen pen(src->getParentMachine()->prefs->connectionColor);
    pen.setWidthF(20.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    setPen(pen);

    setBrush(Qt::NoBrush);

    QPointF p1 = src->centerPos();
    QPointF p3 = dst ? dst->centerPos() : tempEnd;

    const qreal offset = 20.0;

    // Source tangent
    QPointF srcDir = sideToDir(src->side);
    QPointF exitPoint = p1 + srcDir * offset;

    // Target tangent
    QPointF dstDir;
    QPointF entryPoint;

    if (dst) {
        dstDir = sideToDir(dst->side);
        entryPoint = p3 + dstDir * offset;
    } else {
        // Direction from exit point to mouse
        QPointF mouseDir = safeNormalize(p3 - exitPoint);

        // Blend from source tangent → mouse direction
        constexpr qreal smoothing = 0.15; // smaller = smoother
        dragDir = safeNormalize(
            dragDir * (1.0 - smoothing) + mouseDir * smoothing
            );

        dstDir = dragDir;
    }

    // Adaptive handle
    QPointF delta = p3 - p1;
    qreal distance = std::hypot(delta.x(), delta.y());

    qreal handle = std::max(3*offset, distance * 0.5);

    QPointF c1 = exitPoint + srcDir * handle;
    QPointF c2 = p3 - dstDir * handle;

    if (dst) {
        c2 = entryPoint + dstDir * handle;
    }

    connPath = QPainterPath(p1);
    connPath.lineTo(exitPoint);

    if (dst) {
        connPath.cubicTo(c1, c2, entryPoint);
        connPath.lineTo(p3);
    } else {
        connPath.cubicTo(c1, c2, p3);
    }

    qreal newArrowSpacing = src->getParentMachine()->prefs->arrowSpacing;

    if (connPath.length() <= newArrowSpacing) {
        connPath = QPainterPath(p1);
        connPath.quadTo(c1, p3);
    }

    setPath(connPath);

    cachedArrows.clear();
    length = connPath.length();
    if (length <= 0) return;

    if (length <= 2 * newArrowSpacing) {
        qreal t = connPath.percentAtLength(length / 2);
        cachedArrows.append({ connPath.pointAtPercent(t), connPath.angleAtPercent(t) });
    } else {
        const int arrowCount = static_cast<int>(length / newArrowSpacing);
        const qreal distSpacing = length / (arrowCount + 1);
        for (int i = 1; i <= arrowCount; ++i) {
            qreal t = connPath.percentAtLength(i * distSpacing);
            cachedArrows.append({ connPath.pointAtPercent(t), connPath.angleAtPercent(t) });
        }
    }

    updateArrows = false;
}

void ConnectionItem::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem* option,
                           QWidget* widget)
{
    qreal newArrowSpacing = src->getParentMachine()->prefs->arrowSpacing;
    if (newArrowSpacing != arrowSpacing) {
        arrowSpacing = newArrowSpacing;
        updateArrows = true;
    }

    updatePath();

    painter->setRenderHint(QPainter::Antialiasing);
    QGraphicsPathItem::paint(painter, option, widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(src->getParentMachine()->prefs->arrowColor);

    const qreal arrowSize = 16.0;
    for (const ArrowData& a : std::as_const(cachedArrows)) {
        painter->save();
        painter->translate(a.pos);
        painter->rotate(-a.angle);
        QPolygonF arrow;
        if (dst != nullptr && dst->getPortType() == PortType::Output ) {
            painter->translate(arrowSize, 0);
            arrow << QPointF(0, 0)
                  << QPointF(-arrowSize, arrowSize/2)
                  << QPointF(-arrowSize, arrowSize/4)
                  << QPointF(-arrowSize - 4, arrowSize/4)
                  << QPointF(-arrowSize - 4, arrowSize/2)
                  << QPointF(-arrowSize*2 - 4, 0)
                  << QPointF(-arrowSize - 4, -arrowSize/2)
                  << QPointF(-arrowSize - 4, -arrowSize/4)
                  << QPointF(-arrowSize, -arrowSize/4)
                  << QPointF(-arrowSize, -arrowSize/2);
        } else {
            painter->translate(arrowSize / 2, 0);
            arrow << QPointF(0, 0)
                  << QPointF(-arrowSize,  arrowSize / 2)
                  << QPointF(-arrowSize, -arrowSize / 2);
        }
        painter->drawPolygon(arrow);
        painter->restore();
    }
}

void ConnectionItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* remove = menu.addAction("Delete");

    QAction* selected = menu.exec(event->screenPos());
    if (!selected)
        return;

    if (selected == remove) {
        delete this;
    }
}

void ConnectionItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dst->removeConnection(this);
        dst = nullptr;
        if (auto* c = qobject_cast<GraphicsScene*>(src->scene())) {
            c->tempConnection = this;
        }
    }
}
