#include "sourceconfigdialog.h"
#include "ui_sourceconfigdialog.h"

#include "portitem.h"

#include <QToolButton>
#include <QButtonGroup>

SourceConfigDialog::SourceConfigDialog(MachineItem* source, const QHash<QString, QVector<MaterialDefinition>> &materials, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SourceConfigDialog)
    , source(source)
    , materials(materials)
{
    ui->setupUi(this);

    categories = QHash<QString, QGridLayout*> {
        {"Wood", ui->gridLayoutWood},
        {"Carpentry", ui->gridLayoutCarpentry},
        {"Rocks", ui->gridLayoutRocks},
        {"Ore", ui->gridLayoutOre},
        {"Food", ui->gridLayoutFood},
        {"Produce", ui->gridLayoutProduce},
        {"Fluid", ui->gridLayoutFluid}
    };

    int row = 0;
    int col = 0;

    QButtonGroup *btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);

    connect(btnGroup, &QButtonGroup::buttonToggled,
            this, [this](QAbstractButton* btn, bool checked)
    {
        if (!checked)
            return;

        if (!this->source->inputPorts.isEmpty()) {
            this->source->inputPorts.first()->material = this->materials[btn->property("cat").toString()][btn->property("mat").toInt()];
        } else if (!this->source->outputPorts.isEmpty()) {
            this->source->outputPorts.first()->material = this->materials[btn->property("cat").toString()][btn->property("mat").toInt()];
        }
    });

    for (auto [cat, matH] : materials.asKeyValueRange())
    {
        if (categories.contains(cat)) {
            for (int i = 0; i < matH.size(); i++)
            {
                QToolButton* btn = new QToolButton(this);

                btn->setProperty("cat", cat);
                btn->setProperty("mat", i);
                btn->setIcon(QIcon(matH[i].iconPath));
                btn->setIconSize(QSize(64, 64));
                btn->setFixedSize(80, 80);
                btn->setCheckable(true);
                btn->setToolTip(matH[i].name);

                btnGroup->addButton(btn);
                categories[cat]->addWidget(btn, row, col);

                col += 1;
                if (col > 3) {
                    col = 0;
                    row += 1;
                }
            }
            col = 0;
            row = 0;
        }
    }
}

SourceConfigDialog::~SourceConfigDialog()
{
    delete ui;
}
