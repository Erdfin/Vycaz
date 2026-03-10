#pragma once

#include "machinelibrary.h"
#include "machineitem.h"
#include "preferencesdefinition.h"
#include "settingsdefinition.h"

#include <QMainWindow>
#include <QTabWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QShortcut>
#include <QToolButton>

class GraphicsView;
class GraphicsScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    QGraphicsItem* spawnMachine(const QString& machineId, QPointF scenePos);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void createRibbon();
    QWidget* createMQTTTab();
    QWidget* createSourceTab();
    QWidget* createRouterTab();

    QToolButton* addButton(QString name, QString id);

    void showAboutDialog();
    void showPreferences();
    void showMachineConfigDialog(MachineItem* item);

    GraphicsScene* scene = nullptr;
    GraphicsView* view = nullptr;
    QTabWidget* ribbonTabs = nullptr;

    MachineLibrary machineLibrary;

    void saveSettings(const QString& filePath) const;
    void loadSettings(const QString& filePath);

    void saveProject(const QString& filePath);
    void loadProject(const QString& filePath);

    bool checkProjectVersion(const int fileVersion);

    void saveTheme(QString name, Prefs theme, const QString& filePath) const;
    void loadTheme(const QString& filePath);

    void loadThemes(const QString& filePath);

    Prefs loadThemeFromJson(QJsonObject* root) const;

    Prefs preferences;

    QString userThemesDir;

    QHash<QString, Prefs> themes;

    QPointF fixPosition(QPointF pos);

    QPointF getSceneMousePos();

    QHash<QString, QVector<QShortcut*>> shortcutMap;
    int tabIndex;

    Settings settings{
        .bundleTheme = true,
        .loadBundledTheme = true
    };
};
