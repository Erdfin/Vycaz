#pragma once

#include <QString>

enum class PortType {
    Input,
    Output
};

enum class PortSide {
    Left,
    Right,
    Top,
    Bottom
};

struct PortDefinition
{
    PortType type;
    PortSide side;
};
