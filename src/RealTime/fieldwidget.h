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
    int m_width;
    int m_height;
    QGraphicsScene* m_scene;
    Cursor* m_cursor;
    Area* m_area;
    std::shared_ptr<Scan> m_scan = nullptr;
    std::unique_ptr<QTimer> m_updateTimer;
    std::unique_ptr<QTimer> m_findPeakTimer;

    uint m_curIndex = 0;
    size_t m_numArea = 0;

public:
    FieldWidget(int width = 500,int height = 500);
    ~FieldWidget();
    void setScan(std::shared_ptr<Scan> scan);
    void setNumArea(size_t numArea);

public slots:
    void timeout();
    void findPeak();
    void changeMode(int btn, bool value);
};

#endif // FIELDWIDGET_H
