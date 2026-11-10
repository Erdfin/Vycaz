#pragma once

#include <QColor>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QString>

struct Prefs;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneMouseEvent;

namespace tex {
class TeXRender;
}

/**
 * A movable QGraphicsItem that renders its text with MicroTeX.
 *
 * Drop one into any QGraphicsScene/QGraphicsView. Double-click (or use the
 * context menu) to edit the LaTeX source. The item keeps its own TeXRender and
 * resizes itself when the formula changes.
 */
class LatexItem final : public QGraphicsItem
{
public:
    enum { Type = QGraphicsItem::UserType + 5 };

    explicit LatexItem(QString latex,
                       const Prefs* prefs = nullptr,
                       QGraphicsItem* parent = nullptr);
    ~LatexItem() override;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    int type() const override { return Type; }

    QString latex() const { return _latex; }
    QString errorString() const { return _error; }
    bool isValid() const { return _render != nullptr; }

    bool setLatex(const QString& latex);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void rebuild();
    void editFormula();
    QRectF cardRect() const;

    tex::TeXRender* _render = nullptr;
    QString _latex;
    QString _error;
    QRectF _bounds;
    QColor _cardColor;
    QColor _textColor;
    QColor _accentColor;
};
