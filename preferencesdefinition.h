#pragma once

#include <QFont>
#include <QColor>

struct Prefs {
    QString name;
    QFont font;
    QColor fontColor;
    QColor highlightedColor;
    QColor nodeColor;
    QColor topicColor;
    QColor contBodyColor;
    QColor contHeadColor;
    QColor bcColor;
    QColor gridColor;
    QColor subGridColor;
    QColor inputColor;
    QColor outputColor;
    QColor connectionColor;
    QColor arrowColor;
    bool showGrid;
    bool snapToGrid;
    int gridSize;
    int majorGridFactor;
    qreal arrowSpacing;
};
