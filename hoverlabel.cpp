#include "hoverlabel.h"
#include <QPainter>

HoverLabel::HoverLabel(const QString& t, QGraphicsItem* parent)
    : QGraphicsObject(parent), text(t),
    opacityAnim(this, "opacity"),
    posAnim(this, "pos")
{
    setOpacity(0.0);
    setZValue(1000);

    opacityAnim.setDuration(120);
    opacityAnim.setStartValue(0.0);
    opacityAnim.setEndValue(0.9);

    posAnim.setDuration(120);
    posAnim.setEasingCurve(QEasingCurve::OutCubic);

    setFlag(ItemIgnoresTransformations, true);
    updateSize();
}

void HoverLabel::updateSize()
{
    QFont font;
    QFontMetricsF fm(font);

    // Provide a very wide rect to let text expand naturally
    QRectF textRect = fm.boundingRect(QRectF(0, 0, 1000, 1000),
                                      Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                                      text);

    contentSize = textRect.size() + QSizeF(16, 12); // padding
    prepareGeometryChange();
}


void HoverLabel::setText(const QString& t)
{
    text = t;
    updateSize();
    update();
}

QRectF HoverLabel::boundingRect() const
{
    return QRectF(QPointF(0,0), contentSize);
}

void HoverLabel::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing);
    p->setBrush(QColor(40, 40, 40, 200));
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(boundingRect(), 6, 6);

    p->setPen(Qt::white);
    p->drawText(boundingRect().adjusted(8,6,-8,-6), text);
}

void HoverLabel::showAnimated(const QPointF& targetPos)
{
    posAnim.setStartValue(pos());
    posAnim.setEndValue(targetPos);
    opacityAnim.start();
    posAnim.start();
}

void HoverLabel::fadeOut()
{
    if (!graphicsEffect()) {
        auto* effect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(effect);
    }

    auto* effect = static_cast<QGraphicsOpacityEffect*>(graphicsEffect());

    QPropertyAnimation* anim = new QPropertyAnimation(effect, "opacity", this);
    anim->setDuration(300); // milliseconds
    anim->setStartValue(effect->opacity());
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InOutQuad);

    // Connect using lambda instead of direct method pointer
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        this->hide();
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
