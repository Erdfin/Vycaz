#pragma once

#include <QHash>
#include <QString>
#include <QVector>

/*
 * inputs:
 *  "input": amount
 *
 * outpus:
 *  "output": [count, chance]
*/
struct MachineRecipe
{
    QString type;
    int processing_time;
    QHash<QString, int> inputs;
    QHash<QString, QVector<double>> outputs;
};
