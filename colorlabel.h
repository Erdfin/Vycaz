#pragma once

#include <QPushButton>
#include <QPainter>

class ColorLabel : public QPushButton
{
    Q_OBJECT
public:
    ColorLabel(QWidget *parent = nullptr);

    QColor *m_color;

private:

protected:
    void paintEvent(QPaintEvent *e) override {
        setStyleSheet("text-align: left; padding-left: 92px;");
        QPushButton::paintEvent(e); // draw the normal button

        if (m_color) {
            QPainter p(this);
            QRect swatch(2, 2, 80, fontMetrics().height() - 4);
            p.fillRect(swatch, *m_color);
            p.setPen(palette().color(QPalette::Dark));
            p.drawRect(swatch);
        }
    }
};
