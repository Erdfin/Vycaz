#pragma once

#include <QGraphicsPathItem>

class PortItem;

class ConnectionItem : public QGraphicsPathItem
{
public:
    ConnectionItem(PortItem* from, PortItem* to = nullptr);
    ~ConnectionItem();

    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    void setEndPos(const QPointF& pos);
    void setTargetPort(PortItem* p);
    void updatePath();
    QRectF boundingRect() const override;

    PortItem* src = nullptr;
    PortItem* dst = nullptr;

    void deleteConn();
    bool updateArrows = true;
private:
    QPointF tempEnd;
    QPointF dragDir = QPointF(1, 0);
    struct ArrowData { QPointF pos; qreal angle; };
    QVector<ArrowData> cachedArrows;
    qreal arrowSpacing;
    qreal length = 0;

protected:
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    QPainterPath shape() const override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};
