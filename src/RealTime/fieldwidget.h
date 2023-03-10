#ifndef FIELDWIDGET_H
#define FIELDWIDGET_H

#include "Core/ScanData.hh"
#include <QtWidgets/qgraphicsview.h>
#include <QtWidgets/qgraphicsscene.h>

class Cursor;
class Area;
class FieldWidget : public QGraphicsView
{
    Q_OBJECT
    const int WIDTH = 100;
    const int HEIGHT = 10;
    QGraphicsScene* m_scene;
    Cursor* m_cursor;
    Area* m_area;
    std::shared_ptr<Scan> m_scan = nullptr;
    std::unique_ptr<QTimer> m_updateTimer;
    std::unique_ptr<QTimer> m_findPeakTimer;

    int m_curIndex = 0;

public:
    FieldWidget();
    ~FieldWidget();
    void setScan(std::shared_ptr<Scan> scan);

public slots:
    void timeout();
    void findPeak();
};

#endif // FIELDWIDGET_H
