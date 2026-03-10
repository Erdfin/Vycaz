#pragma once

#include "machineitem.h"
#include "materialdefinition.h"

#include <QDialog>
#include <QHash>
#include <QGridLayout>

namespace Ui {
class SourceConfigDialog;
}

class SourceConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SourceConfigDialog(MachineItem* source, const QHash<QString, QVector<MaterialDefinition> > &materials, QWidget *parent = nullptr);
    ~SourceConfigDialog();

private:
    Ui::SourceConfigDialog *ui;
    MachineItem* source;
    const QHash<QString, QVector<MaterialDefinition>> &materials;
    QHash<QString, QGridLayout*> categories;
};
