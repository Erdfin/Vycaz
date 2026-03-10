#include "portconfigitem.h"

#include <QPainter>
#include <QMouseEvent>
#include <QUuid>

PortConfigItem::PortConfigItem(QWidget *parent, PortDefinition def, QString instanceId, int diameter, bool normal)
    : QWidget(parent), diameter(diameter), def(def), normal(normal), id(instanceId)
{
    setMinimumSize(diameter, diameter);
    setMaximumSize(diameter, diameter);

    if (id.isEmpty()) id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void PortConfigItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(def.type == PortType::Input ? Qt::green : Qt::red);
    painter.setPen(Qt::NoPen);

    qreal dDia = qMin(width(), height()) - 4.0;
    QRectF circleRect(
        (width() - dDia) / 2.0,
        (height() - dDia) / 2.0,
        dDia,
        dDia
    );

    painter.drawEllipse(circleRect);

    if (!normal) {
        // ---- Draw plus sign ----
        painter.setPen(QPen(Qt::black, dDia * 0.08, Qt::SolidLine, Qt::RoundCap));

        qreal plusSize = dDia * 0.8;
        QPointF center = circleRect.center();

        // Horizontal line
        painter.drawLine(
            QPointF(center.x() - plusSize / 2.0, center.y()),
            QPointF(center.x() + plusSize / 2.0, center.y())
        );
        // Vertical line
        painter.drawLine(
            QPointF(center.x(), center.y() - plusSize / 2.0),
            QPointF(center.x(), center.y() + plusSize / 2.0)
        );
    }
}

void PortConfigItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (normal) {
        if (event->button() == Qt::LeftButton) {
            def.type = def.type == PortType::Input ? PortType::Output : PortType::Input;
            update();
        } else if(event->button() == Qt::RightButton) {
            emit removePort(this);
        }
    } else {
        emit addPort(def.side);
    }
}
