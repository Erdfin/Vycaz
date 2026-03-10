#pragma once

#include <QString>
#include <QVector>
#include <QSet>

#include "portdefinition.h"
#include "recipedefinition.h"

enum class MachineType {
    Machine,
    Source,
    Router
};

struct MachineDefinition
{
    QString id;
    QString name;
    QString iconPath;
    MachineType type;
    QList<QString> recipeType;
    int kineticStress; // kinectic stress impact multiplier
    int speedRequirement;
    qreal height;
    qreal width;

    QVector<PortDefinition> inputs;
    QVector<PortDefinition> outputs;

    QSet<QString> acceptedInputs;
    QSet<QString> possibleOutputs;

    /*
     * key: input
     * value: QHash:
     *          key: output
     *          value: chance
    */
    MachineRecipe recipes;
};
