#include "machinelibrary.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QVector>
#include <QMessageBox>

bool MachineLibrary::loadMachinesFromFile(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray arr = doc["machines"].toArray();

    all.clear();
    machineList.clear();
    sourceList.clear();

    for (const QJsonValue& v : std::as_const(arr))
    {
        QJsonObject o = v.toObject();

        MachineDefinition def;
        def.id = o["id"].toString();
        def.name = o["name"].toString();
        def.iconPath = o["icon"].toString();
        def.type = MachineType::Machine;
        def.kineticStress = o["kinetic stress"].toInt();
        def.speedRequirement = o["speed requirement"].toInt();

        def.height = o["height"].isDouble() ? o["height"].toInt() : 50;
        def.width = o["width"].isDouble() ? o["width"].toInt() : 160;

        QString recipeType = o["type"].toString();
        if (!recipeType.isEmpty())
        {
            if (recipeType.at(0) == QChar('|'))
            {
                def.recipeType.append(recipeType.split("|", Qt::SkipEmptyParts));
            } else
            {
                def.recipeType.append(recipeType);
            }
        }

        int input_count = o["inputs"].toInt();
        int output_count = o["outputs"].toInt();

        for (int _ = 0; _ < input_count ; _++) {
            PortDefinition pd;
            pd.type = PortType::Input;
            pd.side = PortSide::Left;
            def.inputs.push_back(pd);
        }

        for (int _ = 0; _ < output_count ; _++) {
            PortDefinition pd;
            pd.type = PortType::Output;
            pd.side = PortSide::Right;
            def.outputs.push_back(pd);
        }

        all.insert(def.id, def);
        machineList.append(def);
    }

    if (machineList.empty()) {
        qWarning() << "No machines loaded";
    } else {
        qInfo() << "Loaded machines: " << machineList.size();
        sortByName(machineList);
    }

    return true;
}

const MachineDefinition* MachineLibrary::get(const QString& id) const
{
    auto it = all.find(id);
    return it == all.end() ? nullptr : &it.value();
}
