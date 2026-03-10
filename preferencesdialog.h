#pragma once

#include "preferencesdefinition.h"

#include "settingsdefinition.h"

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(Prefs defaults, Prefs prefs, QHash<QString, Prefs> *themes, Settings *settings, QWidget *parent = nullptr);
    ~PreferencesDialog();

    Prefs prefs;
    Prefs newPrefs;

signals:
    void applyChanges(Prefs prefs);
    void saveTheme(Prefs theme);
    void loadTheme();

private:
    Ui::PreferencesDialog *ui;
    bool colorSelector(QColor *color);
    const QHash<QString, Prefs> *themes;
    Settings *settings;
};
