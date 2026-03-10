#pragma once
#include <QToolButton>

class FactoryButton : public QToolButton
{
    Q_OBJECT

public:
    explicit FactoryButton(int factoryIndex, QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    int index;
    QPoint dragStart;
};
