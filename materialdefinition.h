#pragma once

#include <QString>

#include <QAnyStringView>
#include <array>
#include <optional>

enum class MaterialType
{
    Stackable,
    NonStackable,
    Fluid
};

constexpr std::array<std::pair<MaterialType, QAnyStringView>, 3> materialTypeMap{{
    {MaterialType::Stackable, "stackable"},
    {MaterialType::NonStackable, "non stackable"},
    {MaterialType::Fluid, "fluid"}
}};

constexpr std::optional<MaterialType> materialFromString(QAnyStringView s) {
    for (auto [material, name] : materialTypeMap) {
        if (name == s)
            return material;
    }
    return std::nullopt;
}

struct MaterialDefinition
{
    MaterialType type;
    QString name;
    QString id;
    QString iconPath;
    QString cat;
    // QList<QString> tags;
};
