#pragma once

#include "portitem.h"

#include <QHash>
#include <QVector>

class ConnectionGraph
{
public:
    void connect(PortItem* from, PortItem* to);
    void disconnect(PortItem* from, PortItem* to);

    QVector<PortItem*> inputs(PortItem* m);
    QVector<PortItem*> outputs(PortItem* m);

private:
    QHash<PortItem*, QVector<PortItem*>> adj;
};
