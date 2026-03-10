#pragma once

#include "machineitem.h"
#include "portdefinition.h"
#include "portconfigitem.h"

#include <QDialog>
#include <QValidator>

namespace Ui {
class MachineConfigureDialog;
}

class MachineConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MachineConfigureDialog(MachineItem* machine, QWidget *parent = nullptr);
    ~MachineConfigureDialog();

private slots:
    void onAccepted();
    void onAddPort(PortSide side);
    void onRemovePort(PortConfigItem* self);

private:
    Ui::MachineConfigureDialog *ui;
    MachineItem* machine;
    void addPort(PortSide side, PortDefinition def, int ix);
    QHash<QString, PortConfigItem*> ports;
};
