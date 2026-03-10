#pragma once
#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFontMetricsF>

class HoverLabel : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit HoverLabel(const QString& text, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

    void showAnimated(const QPointF& targetPos); // animate to a target position
    void fadeOut();
    void setText(const QString& t);              // update text and resize

    const QSizeF& getSize() const { return contentSize; }

    double scale = 1.0; // For positioning

private:
    QString text;
    QSizeF contentSize;
    QPropertyAnimation opacityAnim;
    QPropertyAnimation posAnim;

    void updateSize();
};
