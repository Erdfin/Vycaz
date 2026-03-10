#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(QWidget* parent = nullptr);

    void startFactoryDrag(const QString& machineId);

    double getScale() const {return currentScale;}

signals:
    void factoryDropped(const QString& machineId, QPointF scenePos);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    bool panning = false;
    QPoint lastMousePos;

    double currentScale = 1.0;
    const double minScale = 0.2;
    const double maxScale = 3.0;
};
