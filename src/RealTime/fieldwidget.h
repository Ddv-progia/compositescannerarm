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
    std::shared_ptr<ScanArm> m_scan = nullptr;
    std::unique_ptr<QTimer> m_updateTimer;
    std::unique_ptr<QTimer> m_findPeakTimer;
    Area* m_area;
    uint m_curIndex = 0;
    size_t m_numArea = 0;

public:
    FieldWidget(int width,int height);
    ~FieldWidget();
    void setScan(std::shared_ptr<ScanArm> scan);
    void setNumArea(size_t numArea);
    std::shared_ptr<ScanArm> scan() { return m_scan; }

public slots:
    void timeout();
    void findPeak();
    void changeMode(int btn, bool value);
    void stopTimers();
    void runTimers();
    void drawArea();
};

#endif // FIELDWIDGET_H
