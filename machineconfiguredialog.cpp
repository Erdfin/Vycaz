#include "machineconfiguredialog.h"
#include "ui_machineconfiguredialog.h"

#include "portconfigitem.h"
#include "portitem.h"

MachineConfigureDialog::MachineConfigureDialog(MachineItem* machine, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MachineConfigureDialog)
    , machine(machine)
{
    ui->setupUi(this);

    setWindowTitle("Configuration");

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &MachineConfigureDialog::onAccepted);

    PortConfigItem* leftAdd = new PortConfigItem(this, PortDefinition{PortType::Input, PortSide::Left}, "", 16, false);
    PortConfigItem* rightAdd = new PortConfigItem(this, PortDefinition{PortType::Input, PortSide::Right}, "", 16, false);
    PortConfigItem* topAdd = new PortConfigItem(this, PortDefinition{PortType::Input, PortSide::Top}, "", 16, false);
    PortConfigItem* bottomAdd = new PortConfigItem(this, PortDefinition{PortType::Input, PortSide::Bottom}, "", 16, false);

    ui->leftPorts->addWidget(leftAdd);
    ui->rightPorts->addWidget(rightAdd);
    ui->topPorts->addWidget(topAdd);
    ui->bottomPorts->addWidget(bottomAdd);

    connect(leftAdd, &PortConfigItem::addPort, this, &MachineConfigureDialog::onAddPort);
    connect(rightAdd, &PortConfigItem::addPort, this, &MachineConfigureDialog::onAddPort);
    connect(topAdd, &PortConfigItem::addPort, this, &MachineConfigureDialog::onAddPort);
    connect(bottomAdd, &PortConfigItem::addPort, this, &MachineConfigureDialog::onAddPort);

    for (auto *port : std::as_const(machine->portsList[PortSide::Left])) {
        auto* newPort = new PortConfigItem(this, PortDefinition{port->getPortType(), port->side}, port->instanceId);
        ports[newPort->id] = newPort;
        ui->leftPorts->addWidget(newPort);
        connect(newPort, &PortConfigItem::removePort, this, &MachineConfigureDialog::onRemovePort);
    }
    for (auto *port : std::as_const(machine->portsList[PortSide::Right])) {
        auto* newPort = new PortConfigItem(this, PortDefinition{port->getPortType(), port->side}, port->instanceId);
        ports[newPort->id] = newPort;
        ui->rightPorts->addWidget(newPort);
        connect(newPort, &PortConfigItem::removePort, this, &MachineConfigureDialog::onRemovePort);
    }
    for (auto *port : std::as_const(machine->portsList[PortSide::Top])) {
        auto* newPort = new PortConfigItem(this, PortDefinition{port->getPortType(), port->side}, port->instanceId);
        ports[newPort->id] = newPort;
        ui->topPorts->addWidget(newPort);
        connect(newPort, &PortConfigItem::removePort, this, &MachineConfigureDialog::onRemovePort);
    }
    for (auto *port : std::as_const(machine->portsList[PortSide::Bottom])) {
        auto* newPort = new PortConfigItem(this, PortDefinition{port->getPortType(), port->side}, port->instanceId);
        ports[newPort->id] = newPort;
        ui->bottomPorts->addWidget(newPort);
        connect(newPort, &PortConfigItem::removePort, this, &MachineConfigureDialog::onRemovePort);
    }
}

MachineConfigureDialog::~MachineConfigureDialog()
{
    delete ui;
}

void MachineConfigureDialog::onAddPort(PortSide side)
{
    PortConfigItem* port = new PortConfigItem(this, PortDefinition{PortType::Input, side});
    ports[port->id] = port;
    switch (side) {
        case PortSide::Left:
            ui->leftPorts->addWidget(port);
            break;

        case PortSide::Right:
            ui->rightPorts->addWidget(port);
            break;

        case PortSide::Top:
            ui->topPorts->addWidget(port);
            break;

        case PortSide::Bottom:
            ui->bottomPorts->addWidget(port);
            break;
    }

}

void MachineConfigureDialog::onRemovePort(PortConfigItem *port)
{
    this->ports.remove(port->id);
    delete port;
}

void MachineConfigureDialog::onAccepted()
{
    const QStringList portMapKeys = machine->portMap.keys();
    for (const QString& key : portMapKeys) {
        if (!ports.contains(key)) {
            delete machine->portMap.value(key);
            machine->portMap.remove(key);
        }
    }
    machine->portsList.clear();

    QList<QLayout*> layouts = {
        ui->leftPorts,
        ui->rightPorts,
        ui->topPorts,
        ui->bottomPorts
    };

    for (QLayout* layout : layouts) {
        for (int i = 1; i < layout->count(); i++) {
            QLayoutItem *item = layout->itemAt(i);
            if (auto port = qobject_cast<PortConfigItem*>(item->widget())) {
                if (!machine->portMap.contains(port->id)) {
                    machine->addPort(port->def);
                } else {
                    machine->portMap.value(port->id)->setPortType(port->def.type);
                    machine->portsList[port->def.side].push_back(machine->portMap.value(port->id));
                }
            }
        }
    }

    machine->updateOrientation();
}
