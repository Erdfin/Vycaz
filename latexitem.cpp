#include "latexitem.h"

#include "latex.h"
#include "platform/qt/graphic_qt.h"
#include "preferencesdefinition.h"

#include <QAction>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <algorithm>
#include <exception>

namespace {

constexpr qreal kOuterMargin = 10.0;
constexpr qreal kPadding = 18.0;
constexpr qreal kMinWidth = 220.0;
constexpr qreal kMinHeight = 76.0;
constexpr qreal kRadius = 8.0;
constexpr int kMaxFormulaWidth = 620;

tex::color toTexColor(const QColor& color)
{
    return (tex::color(color.alpha()) << 24)
         | (tex::color(color.red()) << 16)
         | (tex::color(color.green()) << 8)
         | tex::color(color.blue());
}

// MicroTeX needs its 'res' tree on disk before the first parse. The CMake
// post-build step copies it next to the executable.
bool ensureLatexInitialized()
{
    static const bool initialized = [] {
        QStringList candidates;
        const QString appDir = QCoreApplication::applicationDirPath();
        candidates << appDir + QStringLiteral("/res")
                   << appDir + QStringLiteral("/../res")
                   << QDir::current().filePath(QStringLiteral("res"));

        for (const QString& candidate : candidates) {
            const QDir dir(candidate);
            if (!QFileInfo::exists(dir.filePath(QStringLiteral(".clatexmath-res_root")))) {
                continue;
            }
            try {
                tex::LaTeX::init(dir.absolutePath().toStdString());
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    }();
    return initialized;
}

} // namespace

LatexItem::LatexItem(QString latex, const Prefs* prefs, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , _latex(std::move(latex))
    , _cardColor(prefs ? prefs->contBodyColor : QColor("#2D2D37"))
    , _textColor(prefs ? prefs->fontColor : QColor("#DCE6FF"))
    , _accentColor(prefs ? prefs->highlightedColor : QColor("#64A0FF"))
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setCursor(Qt::SizeAllCursor);
    setZValue(5.0);

    rebuild();
}

LatexItem::~LatexItem()
{
    delete _render;
}

QRectF LatexItem::boundingRect() const
{
    return _bounds;
}

QPainterPath LatexItem::shape() const
{
    QPainterPath path;
    path.addRoundedRect(cardRect(), kRadius, kRadius);
    return path;
}

QRectF LatexItem::cardRect() const
{
    return QRectF(kOuterMargin,
                  kOuterMargin,
                  std::max<qreal>(1.0, _bounds.width() - 2.0 * kOuterMargin),
                  std::max<qreal>(1.0, _bounds.height() - 2.0 * kOuterMargin));
}

void LatexItem::paint(QPainter* painter,
                      const QStyleOptionGraphicsItem*,
                      QWidget*)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    const QRectF card = cardRect();

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 40));
    painter->drawRoundedRect(card.translated(0, 3), kRadius, kRadius);

    painter->setBrush(_cardColor);
    painter->setPen(QPen(isSelected() ? _accentColor : _cardColor.lighter(150),
                         isSelected() ? 2.0 : 1.0));
    painter->drawRoundedRect(card, kRadius, kRadius);
    painter->setClipRect(card.adjusted(2, 2, -2, -2));

    if (_render != nullptr) {
        // QGraphicsView in this project uses DontSavePainterState and the
        // painter already contains the view/item transform. MicroTeX's Qt
        // backend ends a render with QPainter::setTransform(QTransform()),
        // which would throw that transform away. Save/restore around draw()
        // so later painting (and the view) stay correct.
        painter->save();

        const qreal w = _render->getWidth();
        const qreal h = _render->getHeight();
        const int x = qRound(card.left() + (card.width() - w) / 2.0);
        const int y = qRound(card.top() + (card.height() - h) / 2.0);

        tex::Graphics2D_qt graphics(painter);
        _render->draw(graphics, x, y);

        painter->restore();
    } else {
        QFont font = painter->font();
        font.setItalic(true);
        painter->setFont(font);
        painter->setPen(QColor("#FF9A9A"));
        painter->drawText(card.adjusted(kPadding, kPadding, -kPadding, -kPadding),
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                          QStringLiteral("MicroTeX: %1").arg(_error));
    }

    painter->restore();
}

bool LatexItem::setLatex(const QString& latex)
{
    _latex = latex;
    rebuild();
    return _render != nullptr;
}

void LatexItem::rebuild()
{
    tex::TeXRender* next = nullptr;
    QString error;

    if (_latex.trimmed().isEmpty()) {
        error = QStringLiteral("empty LaTeX source");
    } else if (!ensureLatexInitialized()) {
        error = QStringLiteral(
            "MicroTeX resources not found; expected a 'res' directory next to the executable");
    } else {
        try {
            next = tex::LaTeX::parse(
                _latex.toStdWString(),
                kMaxFormulaWidth,
                26.0f,
                8.0f,
                toTexColor(_textColor));
            if (next == nullptr) {
                error = QStringLiteral("parser returned no render");
            }
        } catch (const std::exception& ex) {
            error = QString::fromUtf8(ex.what());
            if (error.isEmpty()) {
                error = QStringLiteral("unknown parse error");
            }
        } catch (...) {
            error = QStringLiteral("unknown parse error");
        }
    }

    const qreal renderWidth = next ? qreal(next->getWidth()) : 0.0;
    const qreal renderHeight = next ? qreal(next->getHeight()) : 0.0;
    const qreal cardWidth = std::max(kMinWidth, renderWidth + 2.0 * kPadding);
    const qreal cardHeight = std::max(kMinHeight, renderHeight + 2.0 * kPadding);

    prepareGeometryChange();
    delete _render;
    _render = next;
    _error = error;
    _bounds = QRectF(0.0, 0.0,
                     cardWidth + 2.0 * kOuterMargin,
                     cardHeight + 2.0 * kOuterMargin);

    setToolTip(error.isEmpty() ? _latex : QStringLiteral("MicroTeX: %1").arg(error));
    update();
}

void LatexItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    editFormula();
    event->accept();
}

void LatexItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;
    QAction* edit = menu.addAction(QStringLiteral("Edit LaTeX..."));
    QAction* front = menu.addAction(QStringLiteral("Bring to Front"));

    QAction* chosen = menu.exec(event->screenPos());
    if (chosen == edit) {
        editFormula();
    } else if (chosen == front) {
        setZValue(zValue() + 1.0);
    }
    event->accept();
}

void LatexItem::editFormula()
{
    QWidget* parent = nullptr;
    if (scene() && !scene()->views().isEmpty()) {
        parent = scene()->views().first();
    }

    bool accepted = false;
    const QString source = QInputDialog::getMultiLineText(
        parent,
        QStringLiteral("Edit LaTeX formula"),
        QStringLiteral("LaTeX source:"),
        _latex,
        &accepted);

    if (!accepted) {
        return;
    }

    if (!setLatex(source)) {
        QMessageBox::warning(parent,
                             QStringLiteral("MicroTeX parse error"),
                             _error);
    }
}

QVariant LatexItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged) {
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}
