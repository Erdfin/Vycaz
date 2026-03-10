#pragma once

#include <QHash>
#include <QSharedPointer>

#include "machinedefinition.h"
#include "recipedefinition.h"
#include "materialdefinition.h"

template <typename T>
void sortByName(QVector<T> &list)
{
    std::sort(list.begin(), list.end(),
              [](const T &a, const T &b) {
                  return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
              });
}

class MachineLibrary
{
public:
    bool loadMachinesFromFile(const QString& resourcePath);
    bool loadRecipesFromFile(const QString& resourcePath);
    bool loadMaterialsFromFile(const QString &directoryPath);

    const MachineDefinition* get(const QString& id) const;
    const QHash<QString, QHash<QString, MaterialDefinition>>& getMaterials() const { return materials; }

    const QVector<MachineDefinition>& sortedMachines() const { return machineList; }
    const QVector<MachineDefinition>& sortedSources() const { return sourceList; }
    const QVector<MachineDefinition>& sortedRouters() const { return routerList; }
    const QHash<QString, QVector<MaterialDefinition>>& sortedMaterials() const { return materialList; }

private:
    QHash<QString, MachineDefinition> all;
    QVector<MachineDefinition> machineList;
    QVector<MachineDefinition> sourceList;
    QVector<MachineDefinition> routerList;

    QHash<QString, QVector<MaterialDefinition>> materialList;

    // Recipe type - Recipe Name - Recipe
    QHash<QString, QHash<QString, MachineRecipe*>> recipes;

    // Category - Material name - Material
    QHash<QString, QHash<QString, MaterialDefinition>> materials;
};
