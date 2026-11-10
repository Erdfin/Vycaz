#include "mainwindow.h"

#include "graphicsview.h"
#include "graphicscene.h"
#include "connectionitem.h"
#include "portitem.h"
#include "machineconfiguredialog.h"
#include "containeritem.h"
#include "preferencesdefinition.h"
#include "preferencesdialog.h"

#include <QDockWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QTimer>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QSvgGenerator>
#include <tuple>
#include <QtOpenGLWidgets>
#include <QShortcut>
#include <QTabWidget>

static constexpr int CURRENT_FORMAT_VERSION = 3;
static constexpr int OLDEST_SUPPORTED_VERSION = 3;
static constexpr QLatin1StringView APP_VERSION{"0.0.2"};

static const Prefs &getDefaultPreferences()
{
    static Prefs prefs{
        .name = "Default",
        .font = []() {
            QFont font = QApplication::font();
            font.setPointSize(14);
            font.setBold(true);
            return font;
        }(),
        .fontColor = QColor(0xDC, 0xE6, 0xFF),
        .highlightedColor = QColor(0x64, 0xA0, 0xFF),
        .nodeColor = QColor(0xFF, 0x81, 0x03),
        .topicColor = QColor(0, 0x5F, 0x7B),
        .contBodyColor = QColor(0x2D, 0x2D, 0x37),
        .contHeadColor = QColor(0x46, 0x5A, 0x8C),
        .bcColor = QColor(0xFF, 0xFF, 0xFF),
        .gridColor = QColor(0x0, 0x0, 0x0),
        .subGridColor = QColor(0x96, 0x96, 0x96),
        .inputColor = QColor(Qt::green),
        .outputColor = QColor(Qt::red),
        .connectionColor = QColor(0xCB, 0xDC, 0xFF),
        .arrowColor = QColor(60, 60, 60),
        .showGrid = false,
        .snapToGrid = true,
        .gridSize = 20,
        .majorGridFactor = 5,
        .arrowSpacing = 120.0,
    };
    return prefs;
};

QJsonObject fontToJson(const QFont& font) {
    return {
        {"family",  font.family()},
        {"pointSize", font.pointSize()},
        {"weight", static_cast<int>(font.weight())},
        {"italic", font.italic()},
        {"underline", font.underline()}
    };
}

QFont fontFromJson(const QJsonObject& obj) {
    QFont font(
        obj["family"].toString(),
        obj["pointSize"].toInt(-1)  // -1 = Qt's "unset" sentinel
        );
    font.setWeight(static_cast<QFont::Weight>(obj["weight"].toInt(QFont::Normal)));
    font.setItalic(obj["italic"].toBool());
    font.setUnderline(obj["underline"].toBool());
    return font;
}

QJsonObject themeToJson(const QString& name, const Prefs& theme) {
    QJsonObject o;
    o["name"] = name;
    o["font"] = fontToJson(theme.font);
    o["font-color"] = theme.fontColor.name();
    o["highlighted-color"] = theme.highlightedColor.name();
    o["node-color"] = theme.nodeColor.name();
    o["topic-color"] = theme.topicColor.name();
    o["container-body-color"] = theme.contBodyColor.name();
    o["containter-head-color"] = theme.contHeadColor.name();
    o["background-color"] = theme.bcColor.name();
    o["grid-color"] = theme.gridColor.name();
    o["sub-grid-color"] = theme.subGridColor.name();
    o["input-color"] = theme.inputColor.name();
    o["output-color"] = theme.outputColor.name();
    o["connection-color"] = theme.connectionColor.name();
    o["arrow-color"] = theme.arrowColor.name();
    o["show-grid"] = theme.showGrid;
    o["snap-to-grid"] = theme.snapToGrid;
    o["grid-size"] = theme.gridSize;
    o["major-grid-factor"] = theme.majorGridFactor;
    o["arrow-spacing"] = theme.arrowSpacing;
    return o;
}

void MainWindow::saveSettings(const QString& filePath) const
{
    QJsonObject root;

    root["bundleTheme"] = settings.bundleTheme;
    root["loadBundledTheme"] = settings.loadBundledTheme;
    root["theme"] = preferences.name;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::saveTheme(QString name, Prefs theme, const QString& filePath) const
{
    QJsonObject newTheme = themeToJson(name, theme);

    // Read file, if it exists
    QJsonArray themesArray;
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly)) {
        QJsonDocument existing = QJsonDocument::fromJson(f.readAll());
        f.close();
        if (existing.isObject()) {
            QJsonObject existingRoot = existing.object();
            themesArray = existingRoot["themes"].toArray();
        }
    }

    // Overwrite if it already exists, append if it doesn't
    bool found = false;
    for (int i = 0; i < themesArray.size(); i++) {
        if (themesArray[i].toObject()["name"].toString() == name) {
            themesArray[i] = newTheme;
            found = true;
            break;
        }
    }
    if (!found)
        themesArray.append(newTheme);

    // Write back
    QJsonObject root;
    root["themes"] = themesArray;

    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::loadSettings(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    if (root["bundleTheme"].isBool()) settings.bundleTheme = root["bundleTheme"].toBool();
    if (root["loadBundledTheme"].isBool()) settings.loadBundledTheme = root["loadBundledTheme"].toBool();
    if (root["theme"].isString()) preferences.name = root["theme"].toString();

}

Prefs MainWindow::loadThemeFromJson(QJsonObject* root) const {
    Prefs loadedTheme;
    loadedTheme.font = fontFromJson(root->value("font").toObject());

    loadedTheme.fontColor = QColor(root->value("font-color").toString());
    loadedTheme.highlightedColor = QColor::fromString(root->value("highlighted-color").toString());
    loadedTheme.nodeColor = QColor::fromString(root->value("node-color").toString());
    loadedTheme.topicColor = QColor::fromString(root->value("topic-color").toString());
    loadedTheme.contBodyColor = QColor::fromString(root->value("container-body-color").toString());
    loadedTheme.contHeadColor = QColor::fromString(root->value("containter-head-color").toString());
    loadedTheme.bcColor = QColor::fromString(root->value("background-color").toString());
    loadedTheme.gridColor = QColor::fromString(root->value("grid-color").toString());
    loadedTheme.subGridColor = QColor::fromString(root->value("sub-grid-color").toString());
    loadedTheme.inputColor = QColor::fromString(root->value("input-color").toString());
    loadedTheme.outputColor = QColor::fromString(root->value("output-color").toString());
    loadedTheme.connectionColor = QColor::fromString(root->value("connection-color").toString());
    loadedTheme.arrowColor = QColor::fromString(root->value("arrow-color").toString());

    if (!loadedTheme.fontColor.isValid()) loadedTheme.fontColor = preferences.fontColor;
    if (!loadedTheme.highlightedColor.isValid()) loadedTheme.highlightedColor = preferences.highlightedColor;
    if (!loadedTheme.nodeColor.isValid()) loadedTheme.nodeColor = preferences.nodeColor;
    if (!loadedTheme.topicColor.isValid()) loadedTheme.topicColor = preferences.topicColor;
    if (!loadedTheme.contBodyColor.isValid()) loadedTheme.contBodyColor = preferences.contBodyColor;
    if (!loadedTheme.contHeadColor.isValid()) loadedTheme.contHeadColor = preferences.contHeadColor;
    if (!loadedTheme.bcColor.isValid()) loadedTheme.bcColor = preferences.bcColor;
    if (!loadedTheme.gridColor.isValid()) loadedTheme.gridColor = preferences.gridColor;
    if (!loadedTheme.subGridColor.isValid()) loadedTheme.subGridColor = preferences.subGridColor;
    if (!loadedTheme.inputColor.isValid()) loadedTheme.inputColor = preferences.inputColor;
    if (!loadedTheme.outputColor.isValid()) loadedTheme.outputColor = preferences.outputColor;
    if (!loadedTheme.connectionColor.isValid()) loadedTheme.connectionColor = preferences.connectionColor;
    if (!loadedTheme.arrowColor.isValid()) loadedTheme.arrowColor = preferences.arrowColor;

    loadedTheme.showGrid = root->value("show-grid").isBool() ? root->value("show-grid").toBool() : preferences.showGrid;
    loadedTheme.snapToGrid = root->value("snap-to-grid").isBool() ? root->value("snap-to-grid").toBool() : preferences.snapToGrid;
    loadedTheme.gridSize = root->value("grid-size").isDouble() ? root->value("grid-size").toInt() : preferences.gridSize;
    loadedTheme.majorGridFactor = root->value("major-grid-factor").isDouble() ? root->value("major-grid-factor").toInt() : preferences.majorGridFactor;
    loadedTheme.arrowSpacing = root->value("arrow-spacing").isDouble() ? root->value("arrow-spacing").toDouble() : preferences.arrowSpacing;

    return loadedTheme;
}

void MainWindow::loadTheme(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonObject root = doc.object();
    preferences = loadThemeFromJson(&root);
}

void MainWindow::loadThemes(const QString& filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    QJsonObject root = doc.object();

    const QJsonArray themesArray = root["themes"].toArray();
    for (const QJsonValue& t : themesArray) {
        QJsonObject o = t.toObject();
        if (!o["name"].isString()) return;

        QString name = o["name"].toString();
        Prefs loadedTheme = loadThemeFromJson(&o);
        loadedTheme.name = name;
        themes[name] = loadedTheme;
    }
}

void exportSceneToSvg(QGraphicsScene* scene, const QString& filePath, Prefs prefs)
{
    if (scene == nullptr) return;

    QRectF bounds;
    const auto graphicItems = scene->items();
    for (QGraphicsItem* item : graphicItems) {
        QRectF itemSceneBounds = item->mapToScene(item->boundingRect()).boundingRect();
        bounds = bounds.united(itemSceneBounds);
    }

    if (bounds.isEmpty()) return;

    const qreal padding = 30.0;
    bounds.adjust(-padding, -padding, padding, padding);

    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(bounds.size().toSize());
    generator.setViewBox(QRectF(0, 0, bounds.width(), bounds.height()));
    generator.setResolution(96);

    QPainter painter;
    painter.begin(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(prefs.font);

    QRectF target(0, 0, bounds.width(), bounds.height());
    scene->render(&painter, target, bounds);

    painter.end();
}

void MainWindow::saveProject(const QString& filePath)
{
    QJsonObject root;
    QJsonArray machines;
    QJsonArray connections;
    QJsonArray containers;
    QJsonObject viewObj;

    std::tuple<QList<MachineItem*>, QList<ConnectionItem*>, QList<ContainerItem*>> items = scene->getItemsLists();

    // ------------------------------------
    // 1) Save machines
    // ------------------------------------

    const auto machineItems = std::get<0>(items);
    for (MachineItem* machine : machineItems) {
        QJsonObject obj;
        obj["instanceId"] = machine->instanceId;
        obj["label"] = machine->getLabel();
        obj["type"] = machine->definition.id;
        obj["orientation"] = machine->orientation;
        obj["pos"] = QJsonArray{ machine->pos().x(), machine->pos().y() };

        QJsonObject cfg;
        cfg["size"] = QJsonArray{machine->getSize().width(), machine->getSize().height()};

        QJsonArray leftPorts;
        for (PortItem* port : std::as_const(machine->portsList[PortSide::Left])) {
            QJsonObject portEntry;
            portEntry["instanceId"] = port->instanceId;
            portEntry["type"] = static_cast<int>(port->getPortType());
            leftPorts.append(portEntry);
        }
        QJsonArray rightPorts;
        for (PortItem* port : std::as_const(machine->portsList[PortSide::Right])) {
            QJsonObject portEntry;
            portEntry["instanceId"] = port->instanceId;
            portEntry["type"] = static_cast<int>(port->getPortType());
            rightPorts.append(portEntry);
        }
        QJsonArray topPorts;
        for (PortItem* port : std::as_const(machine->portsList[PortSide::Top])) {
            QJsonObject portEntry;
            portEntry["instanceId"] = port->instanceId;
            portEntry["type"] = static_cast<int>(port->getPortType());
            topPorts.append(portEntry);
        }
        QJsonArray bottomPorts;
        for (PortItem* port : std::as_const(machine->portsList[PortSide::Bottom])) {
            QJsonObject portEntry;
            portEntry["instanceId"] = port->instanceId;
            portEntry["type"] = static_cast<int>(port->getPortType());
            bottomPorts.append(portEntry);
        }

        cfg["leftPorts"] = leftPorts;
        cfg["rightPorts"] = rightPorts;
        cfg["topPorts"] = topPorts;
        cfg["bottomPorts"] = bottomPorts;

        obj["config"] = cfg;

        machines.append(obj);
    }


    // ------------------------------------
    // 2) Save connections
    // ------------------------------------
    const auto connectionItems = std::get<1>(items);
    for (ConnectionItem* con : connectionItems) {
        QJsonObject obj;

        obj["from"] = QJsonObject{
            { "machine", con->src->getParentMachine()->instanceId },
            { "port", con->src->instanceId }
        };

        obj["to"] = QJsonObject{
            { "machine", con->dst->getParentMachine()->instanceId },
            { "port", con->dst->instanceId }
        };

        connections.append(obj);
    }

    // ------------------------------------
    // 3) Save containers
    // ------------------------------------

    const auto containerItems = std::get<2>(items);
    for (ContainerItem* cont : containerItems) {
        QJsonObject obj;

        obj["pos"] = QJsonArray{ cont->pos().x(), cont->pos().y() };
        obj["label"] = cont->getLabel();

        QJsonObject cfg;
        cfg["size"] = QJsonArray{cont->getSize().width(), cont->getSize().height()};
        obj["config"] = cfg;

        containers.append(obj);
    }

    // ------------------------------------
    // 4) Save view
    // ------------------------------------

    // Zoom
    QTransform transform = view->transform();
    viewObj["zoom"] = transform.m11();

    // View center in scene coords
    QPointF center = view->mapToScene(view->viewport()->rect().center());
    viewObj["center"] = QJsonArray{ center.x(), center.y() };


    root["view"] = viewObj;
    root["machines"] = machines;
    root["connections"] = connections;
    root["containers"] = containers;
    root["formatVersion"] = CURRENT_FORMAT_VERSION;
    root["appVersion"] = APP_VERSION;

    if (settings.bundleTheme) root["theme"] = themeToJson(preferences.name, preferences);

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    }
}

bool MainWindow::checkProjectVersion(const int fileVersion)
{
    if (fileVersion < OLDEST_SUPPORTED_VERSION) { // Hard reject (too old)
        QMessageBox::critical(
            this,
            "Cannot open file",
            "This project file is too old and is no longer supported."
            );
        return false;
    } else if (fileVersion < CURRENT_FORMAT_VERSION) { // Soft warning (older but supported)
        QMessageBox::warning(
            this,
            "Old project format",
            "This project was created with an older version.\n"
            "Loading will be attempted"
            );
    } else if (fileVersion > CURRENT_FORMAT_VERSION) { // Newer file (future version)
        QMessageBox::warning(
            this,
            "Newer project format",
            "This project was created with a newer version of the app.\n"
            "Loading may be incomplete or incorrect."
            );
    }
    return true;
}

void MainWindow::loadProject(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    const int fileVersion = root["formatVersion"].toInt(0);

    if (!checkProjectVersion(fileVersion)) return;

    // ------------------------------------
    // 0) Load bundled themes (if any)
    // ------------------------------------

    if (root.contains("theme") && settings.loadBundledTheme) {
        QJsonObject obj = root["theme"].toObject();
        preferences = loadThemeFromJson(&obj);
        preferences.name = obj["name"].toString();
        themes[preferences.name] = preferences;
        scene->update();
    }

    scene->clear();

    // Map instanceId -> MachineItem*
    QHash<QString, MachineItem*> machineMap;

    // ------------------------------------
    // 1) Load machines
    // ------------------------------------
    const QJsonArray machineArray = root["machines"].toArray();
    for (const QJsonValueConstRef& machineJson : machineArray) {
        QJsonObject obj = machineJson.toObject();

        QString instanceId = obj["instanceId"].toString();
        QString type       = obj["type"].toString();
        QString label      = obj["label"].toString();

        QJsonArray posArr = obj["pos"].toArray();
        QPointF pos(posArr[0].toDouble(), posArr[1].toDouble());

        auto* machine = qgraphicsitem_cast<MachineItem*>(spawnMachine(type, pos));
        if (machine == nullptr) continue;

        machine->instanceId  = instanceId;
        machine->orientation = obj["orientation"].toInt();
        machine->setLabel(label);

        // ---- Config ----
        QJsonObject cfg = obj["config"].toObject();

        QJsonArray sizeArr = cfg["size"].toArray();
        machine->setSize(QSizeF(sizeArr[0].toDouble(), sizeArr[1].toDouble()));

        QJsonArray ports;
        ports.append(cfg["leftPorts"]);
        ports.append(cfg["rightPorts"]);
        ports.append(cfg["topPorts"]);
        ports.append(cfg["bottomPorts"]);

        auto getSide = [](int side) {
            return (side >= 0 && side < 4) ? static_cast<PortSide>(side) : PortSide::Left;
        };

        auto getType = [](int type) {
            return type == 0 ? PortType::Input : PortType::Output;
        };

        machine->clearPorts();

        for (int i = 0; i < 4; i++) {
            PortSide side = getSide(i);

            const auto portArray = ports[i].toArray();
            for (const QJsonValueConstRef& portJson : portArray) {
                auto port = portJson.toObject();
                PortType type = getType(port["type"].toInt());
                QString id = port["instanceId"].toString();
                machine->addPort(PortDefinition{type, side}, id);
            }
        }
        machine->updateOrientation();

        machineMap.insert(instanceId, machine);
    }

    // ------------------------------------
    // 2) Load containers
    // ------------------------------------

    const QJsonArray containers = root["containers"].toArray();
    for (const QJsonValue& containerJson : containers) {
        QJsonObject obj = containerJson.toObject();

        QString label = obj["label"].toString();

        QJsonArray posArr = obj["pos"].toArray();
        QPointF pos(posArr[0].toDouble(), posArr[1].toDouble());

        ContainerItem* cont = qgraphicsitem_cast<ContainerItem*>(spawnMachine("container", pos));
        if (cont == nullptr) continue;

        cont->setLabel(label);

        // ---- Config ----
        QJsonObject cfg = obj["config"].toObject();

        QJsonArray sizeArr = cfg["size"].toArray();
        cont->setSize(QSizeF(sizeArr[0].toDouble(), sizeArr[1].toDouble()));
        cont->update();
    }

    // ------------------------------------
    // 3) Load connections
    // ------------------------------------
    const QJsonArray connections = root["connections"].toArray();
    for (const QJsonValue& connJson : connections) {
        QJsonObject obj = connJson.toObject();

        QJsonObject from = obj["from"].toObject();
        QJsonObject to   = obj["to"].toObject();

        QString fromMachineId = from["machine"].toString();
        QString toMachineId   = to["machine"].toString();

        QString fromPortId = from["port"].toString();
        QString toPortId   = to["port"].toString();

        MachineItem* mFrom = machineMap.value(fromMachineId, nullptr);
        MachineItem* mTo   = machineMap.value(toMachineId, nullptr);

        if (mFrom == nullptr || mTo == nullptr) continue;

        PortItem* src = mFrom->portMap.value(fromPortId, nullptr);
        PortItem* dst = mTo->portMap.value(toPortId, nullptr);

        if (src == nullptr || dst == nullptr) continue;

        auto* conn = new ConnectionItem(src, dst);
        conn->setTargetPort(dst);

        scene->addItem(conn);
    }

    // ------------------------------------
    // 4) Load view
    // ------------------------------------
    if (!root.contains("view"))
        return;

    QJsonObject viewJson = root["view"].toObject();

    const double zoom = viewJson["zoom"].toDouble(1.0);
    QJsonArray centerJson = viewJson["center"].toArray();
    QPointF center(centerJson[0].toDouble(), centerJson[1].toDouble());

    // Reset transform first
    view->resetTransform();

    // Apply zoom
    view->scale(zoom, zoom);

    // Center view
    view->centerOn(center);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Vycaz");

    preferences = getDefaultPreferences();
    themes["Default"] = getDefaultPreferences();

    scene = new GraphicsScene(&preferences, this);
    scene->setSceneRect(-1e9, -1e9, 2e9, 2e9);

    view = new GraphicsView(this);

    // Enable hardware acceleration
    auto *glWidget = new QOpenGLWidget(view);
    view->setViewport(glWidget);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    view->setScene(scene);
    setCentralWidget(view);

    // ------------------------
    // File menu
    // ------------------------

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* newAct  = fileMenu->addAction(tr("&New"));
    QAction* openAct = fileMenu->addAction(tr("&Open..."));
    QAction* saveAct = fileMenu->addAction(tr("&Save..."));
    QAction* expAct  = fileMenu->addAction(tr("&Export..."));
    fileMenu->addSeparator();
    QAction* exitAct = fileMenu->addAction(tr("&Exit"));

    connect(newAct, &QAction::triggered, this, [this]() {
        scene->clear();
    });

    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    connect(openAct, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this,
            tr("Open Project"),
            QString(),
            tr("Graph File (*.qgf)")
            );
        if (!path.isEmpty())
            loadProject(path);
    });

    connect(saveAct, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this,
            tr("Save Project"),
            QString(),
            tr("Graph File (*.qgf)")
            );
        if (!path.endsWith(".qgf", Qt::CaseInsensitive))
            path += ".qgf";
        if (!path.isEmpty())
            saveProject(path);
    });

    connect(expAct, &QAction::triggered, this, [this]() {
        QString path = QFileDialog::getSaveFileName(
            this,
            tr("Save Project"),
            QString(),
            tr("Vector Image (*.svg)")
            );
        if (!path.endsWith(".svg", Qt::CaseInsensitive))
            path += ".svg";
        if (!path.isEmpty())
            exportSceneToSvg(this->view->scene(), path, preferences);
    });

    newAct->setShortcut(QKeySequence::New);
    openAct->setShortcut(QKeySequence::Open);
    saveAct->setShortcut(QKeySequence::Save);
    exitAct->setShortcut(QKeySequence::Quit);

    // ------------------------
    // Edit menu
    // ------------------------
    QMenu* editMenu = menuBar()->addMenu("&Edit");
    QAction* prefAction = editMenu->addAction("Preferences");
    connect(prefAction, &QAction::triggered,
            this, &MainWindow::showPreferences);

    // ------------------------
    // Help menu
    // ------------------------
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    QAction* aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered,
            this, &MainWindow::showAboutDialog);

    if (!machineLibrary.loadMachinesFromFile(":/default/data/machines.json")) {
        qDebug() << "no machines.json file";
    }

    loadThemes(":/default/data/defaultthemes.json");

    QString appDir = QCoreApplication::applicationDirPath();
    userThemesDir = appDir + "/data/userthemes.json";

    loadThemes(userThemesDir);

    loadSettings("./data/settings.json");

    preferences = themes[preferences.name];

    connect(view, &GraphicsView::factoryDropped,
            this, &MainWindow::spawnMachine);

    createRibbon();
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(
        this,
        "About Vycaz",
        QString("<b>Vycaz</b><br>"
        "Version %1 <br><br>"
        "A graphical tool for manually drawing graphs<br><br>"
        "Built using the Qt 6.10.2 framework.").arg(APP_VERSION)
        );
}

void MainWindow::showPreferences() {
    loadThemes(userThemesDir);

    PreferencesDialog dlg(getDefaultPreferences(), preferences, &themes, &settings, this);

    connect(&dlg, &PreferencesDialog::applyChanges,
            this, [this] (Prefs newPrefs) {
        preferences = newPrefs;
        this->scene->update();
        saveSettings("./data/settings.json");
    });

    connect(&dlg, &PreferencesDialog::saveTheme,
            this, [this] (Prefs newTheme) {
        bool ok;
        QString name = QInputDialog::getText(
            this,
            tr("New Theme"),        // window title
            tr("Theme name:"),      // label
            QLineEdit::Normal,
            "",                     // default text
            &ok
        );

        if (ok && !name.isEmpty()) {
            if (themes.contains(name)) {
                bool overrideTheme = QMessageBox::question(
                    this,
                    "Warning",
                    "Theme already exists. Overwrite?"
                );
                if (!overrideTheme) return;
            }
            saveTheme(name, newTheme, userThemesDir);
            loadThemes(userThemesDir);
        }
    });

    dlg.exec();
}

void MainWindow::createRibbon()
{
    tabIndex = 0;
    QWidget* ribbonRoot = new QWidget(this);
    ribbonRoot->setFixedHeight(90);

    QVBoxLayout* rootLayout = new QVBoxLayout(ribbonRoot);
    rootLayout->setContentsMargins(6, 4, 6, 4);

    ribbonTabs = new QTabWidget(ribbonRoot);
    ribbonTabs->setMovable(false);

    auto *mqttTab = createMQTTTab();
    ribbonTabs->addTab(mqttTab, tr("MQTT"));
    rootLayout->addWidget(ribbonTabs);

    connect(ribbonTabs, &QTabWidget::currentChanged,
            this, [this] (int index) {
        for (auto* sc : this->shortcutMap[this->ribbonTabs->tabText(index)]) {
            sc->setEnabled(true);
        }
        for (auto* sc : this->shortcutMap[this->ribbonTabs->tabText(this->tabIndex)]) {
            sc->setEnabled(true);
        }
        this->tabIndex = index;
    });

    QDockWidget* dock = new QDockWidget(this);
    dock->setTitleBarWidget(new QWidget);
    dock->setWidget(ribbonRoot);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    addDockWidget(Qt::TopDockWidgetArea, dock);
}

QPointF MainWindow::getSceneMousePos() {
    return this->view->mapToScene(this->view->mapFromGlobal(QCursor::pos()));
}

QToolButton *MainWindow::addButton(QString name, QString id) {
    QToolButton* btnNode = new QToolButton;
    btnNode->setText(name);
    btnNode->setAutoRaise(true);
    btnNode->setCheckable(false);
    btnNode->setAutoExclusive(false);
    btnNode->setMinimumHeight(42);

    btnNode->setProperty("machineId", id);

    connect(btnNode, &QToolButton::pressed, this, [this, btnNode, id]() {
        // Start the drag
        view->startFactoryDrag(id);

        // Ensure button is visually reset after a small delay
        QTimer::singleShot(0, btnNode, [btnNode]() {
            btnNode->setDown(false); // Reset pressed state
            btnNode->repaint(); // Force repaint
        });
    });

    return btnNode;
}

QWidget* MainWindow::createMQTTTab()
{
    QWidget* tab = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(tab);
    layout->setSpacing(8);

    layout->addWidget(addButton("&Node", "node"));
    layout->addWidget(addButton("&Topic", "topic"));
    layout->addWidget(addButton("&Container", "container"));

    auto *scNode = new QShortcut(QKeySequence(Qt::Key_N), this);
    auto *scTopic = new QShortcut(QKeySequence(Qt::Key_T), this);
    auto *scCont = new QShortcut(QKeySequence(Qt::Key_C), this);

    scNode->setEnabled(true);
    scTopic->setEnabled(true);
    scCont->setEnabled(true);

    shortcutMap["MQTT"].push_back(scNode);
    shortcutMap["MQTT"].push_back(scTopic);
    shortcutMap["MQTT"].push_back(scCont);

    connect(scNode, &QShortcut::activated,
            this, [this] () {
        this->spawnMachine("node", getSceneMousePos());
    });

    connect(scTopic, &QShortcut::activated,
            this, [this] () {
        this->spawnMachine("topic", getSceneMousePos());
    });

    connect(scCont, &QShortcut::activated,
            this, [this] () {
        this->spawnMachine("cont", getSceneMousePos());
    });

    layout->setContentsMargins(4, 4, 4, 4);
    layout->addStretch();
    return tab;
}

QPointF MainWindow::fixPosition(QPointF pos) {
    QPointF fixedPos;

    qreal s = preferences.gridSize;

    fixedPos.setX(qRound(pos.x() / s) * s);
    fixedPos.setY(qRound(pos.y() / s) * s - s);

    return fixedPos;
}

QGraphicsItem* MainWindow::spawnMachine(const QString& machineId, QPointF scenePos)
{
    if (machineId == "node" || machineId == "topic") {
        const MachineDefinition* def = machineLibrary.get(machineId);

        auto* item = new MachineItem(&preferences, *def);

        connect(item, &MachineItem::machineConfigureRequested,
                this, &MainWindow::showMachineConfigDialog);

        connect(item, &MachineItem::createDuplicate,
                this, &MainWindow::spawnMachineItemDuplicate);

        item->instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        item->setPos(fixPosition(scenePos));
        scene->addItem(item);
        return item;
    } else {
        auto* item = new ContainerItem(&preferences);
        item->setPos(fixPosition(scenePos));
        scene->addItem(item);
        return item;
    }
}

MachineItem* MainWindow::spawnMachineItemDuplicate(MachineItem* temp) {
    auto* item = new MachineItem(&preferences, *temp);

    connect(item, &MachineItem::machineConfigureRequested,
            this, &MainWindow::showMachineConfigDialog);

    connect(item, &MachineItem::createDuplicate,
            this, &MainWindow::spawnMachineItemDuplicate);

    item->instanceId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item->setPos(fixPosition(getSceneMousePos()));
    scene->addItem(item);
    return item;
}

void MainWindow::showMachineConfigDialog(MachineItem *item) {
    if (item->definition.type == MachineType::Machine) {
        MachineConfigureDialog dlg(item);
        dlg.exec();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveSettings("./data/settings.json");
    qDebug() << preferences.name;
    event->accept();
}
