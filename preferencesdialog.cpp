#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QFontDialog>
#include <QColorDialog>
#include <QPushButton>

PreferencesDialog::PreferencesDialog(Prefs defaults, Prefs prefs, QHash<QString, Prefs> *themes, Settings *settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
    , prefs(prefs)
    , newPrefs(prefs)
    , themes(themes)
    , settings(settings)
{
    ui->setupUi(this);

    setWindowTitle("Preferences");


    // ------------------------------------------------------------------------
    // General
    // ------------------------------------------------------------------------

    ui->gridSizeSpinBox->setValue(newPrefs.gridSize);
    ui->arrowSpacingSpinBox->setValue(newPrefs.arrowSpacing);

    ui->showGridCheckBox->setChecked(newPrefs.showGrid);
    ui->snapToGridCheckBox->setChecked(newPrefs.snapToGrid);
    ui->bundleThemeCheckBox->setChecked(settings->bundleTheme);
    ui->loadBundledThemeCheckBox->setChecked(settings->loadBundledTheme);

    connect(this->ui->showGridCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) {
        this->newPrefs.showGrid = checked;
        emit applyChanges(this->newPrefs);
    });

    connect(this->ui->snapToGridCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) {
        this->newPrefs.snapToGrid = checked;
        emit applyChanges(this->newPrefs);
    });

    connect(this->ui->bundleThemeCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) {
        this->settings->bundleTheme = checked;
        emit applyChanges(this->newPrefs);
    });

    connect(this->ui->loadBundledThemeCheckBox, &QCheckBox::toggled,
            this, [this] (bool checked) {
        this->settings->loadBundledTheme = checked;
        emit applyChanges(this->newPrefs);
    });

    connect(this->ui->gridSizeSpinBox, &QSpinBox::valueChanged,
            this, [this] (int newGridSize) {
        this->newPrefs.gridSize = newGridSize;
        emit applyChanges(this->newPrefs);
    });

    connect(ui->arrowSpacingSpinBox, &QSpinBox::valueChanged,
            this, [this] (int value) {
        this->newPrefs.arrowSpacing = value;
        emit applyChanges(this->newPrefs);
    });

    // ------------------------------------------------------------------------
    // Themes
    // ------------------------------------------------------------------------

    ui->textColorButton->m_color = &newPrefs.fontColor;
    ui->nodeColorButton->m_color = &newPrefs.nodeColor;
    ui->topicColorButton->m_color = &newPrefs.topicColor;
    ui->contBColorButton->m_color = &newPrefs.contBodyColor;
    ui->contHColorButton->m_color = &newPrefs.contHeadColor;
    ui->bcColorButton->m_color = &newPrefs.bcColor;
    ui->gridColorButton->m_color = &newPrefs.gridColor;
    ui->subGridColorButton->m_color = &newPrefs.subGridColor;
    ui->inputColorButton->m_color = &newPrefs.inputColor;
    ui->outputColorButton->m_color = &newPrefs.outputColor;
    ui->connectionColorButton->m_color = &newPrefs.connectionColor;
    ui->arrowColorButton->m_color = &newPrefs.arrowColor;

    ui->fontButton->setText(QString("%1 %2")
                                      .arg(this->newPrefs.font.family())
                                      .arg(this->newPrefs.font.pointSize())
                                  );

    auto updateCBox = [this] () {
        this->ui->comboBox->clear();
        const auto themeNames = this->themes->keys();
        for (const QString &theme : themeNames) {
            this->ui->comboBox->addItem(theme);
        }
        this->ui->comboBox->setCurrentText(newPrefs.name);
    };

    updateCBox();

    connect(ui->comboBox, &QComboBox::currentTextChanged,
            this, [this] (QString newTheme) {
        if (!newTheme.isEmpty()) {
            newPrefs = this->themes->value(newTheme);
            emit applyChanges(this->newPrefs);
        }
    });

    connect(ui->saveThemeButton, &QPushButton::clicked,
            this, [this, updateCBox] () {
        emit saveTheme(this->newPrefs);
        updateCBox();
    });

    connect(ui->fontButton, &QPushButton::clicked,
            this, [this] () {
        setCursor(Qt::BusyCursor);
        bool ok;
        QFont font = QFontDialog::getFont(&ok, this->newPrefs.font, this);
        setCursor(Qt::ArrowCursor);
        if (ok) this->newPrefs.font = font;
        this->ui->fontButton->setText(QString("%1 %2")
                                       .arg(this->newPrefs.font.family())
                                       .arg(this->newPrefs.font.pointSize())
                                   );
    });

    connect(ui->textColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.fontColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->bcColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.bcColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->gridColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.gridColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->subGridColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.subGridColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->inputColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.inputColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->outputColorButton, &ColorLabel::clicked,
            this, [this] () {
        if (colorSelector(&this->newPrefs.outputColor)) this->ui->comboBox->setCurrentIndex(-1);
    });

    connect(ui->connectionColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.connectionColor)) this->ui->comboBox->setCurrentIndex(-1);
            });

    connect(ui->arrowColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.arrowColor)) this->ui->comboBox->setCurrentIndex(-1);
            });

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, [this] () {
        this->prefs = this->newPrefs;
        emit applyChanges(this->prefs);
        if (!this->themes->contains(this->ui->comboBox->currentText())) {
            emit saveTheme(this->newPrefs);
        }
    });

    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, [this] () {
        this->newPrefs = this->prefs;
        emit applyChanges(this->newPrefs);
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, [this] () {
        emit applyChanges(this->newPrefs);
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, [this] () {
        this->newPrefs = this->prefs;
        emit applyChanges(this->newPrefs);
    });

    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, [this, defaults] () {
        this->newPrefs = defaults;
        this->prefs = defaults;
        emit applyChanges(this->prefs);
    });

    // ------------------------------------------------------------------------
    // MQTT
    // ------------------------------------------------------------------------

    connect(ui->nodeColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.nodeColor)) this->ui->comboBox->setCurrentIndex(-1);
            });

    connect(ui->topicColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.topicColor)) this->ui->comboBox->setCurrentIndex(-1);
            });

    connect(ui->contBColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.contBodyColor)) this->ui->comboBox->setCurrentIndex(-1);
            });

    connect(ui->contHColorButton, &ColorLabel::clicked,
            this, [this] () {
                if (colorSelector(&this->newPrefs.contHeadColor)) this->ui->comboBox->setCurrentIndex(-1);
            });
}

bool PreferencesDialog::colorSelector(QColor *color) {
    setCursor(Qt::BusyCursor);
    QColor newColor = QColorDialog::getColor(*color, this, "Select Color");
    setCursor(Qt::ArrowCursor);
    if (newColor.isValid()) {
        *color = newColor;
        this->ui->comboBox->setCurrentText("");
        return true;
    }

    return false;
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}
