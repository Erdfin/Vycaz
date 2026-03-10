#ifndef PORTCONFIGITEM_H
#define PORTCONFIGITEM_H

#include <QWidget>

#include "portdefinition.h"

class PortConfigItem : public QWidget
{
    Q_OBJECT

    public:
        explicit PortConfigItem(QWidget *parent = nullptr, PortDefinition def = PortDefinition{PortType::Input, PortSide::Left}, QString instanceId = "", int diameter = 16, bool normal = true);
        PortDefinition def;
        QString id;

    signals:
        void addPort(PortSide side);
        void removePort(PortConfigItem *self);

    private:
        int diameter;
        bool normal;

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // PORTCONFIGITEM_H
