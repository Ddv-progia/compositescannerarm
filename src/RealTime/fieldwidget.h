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
    std::shared_ptr<ScanArm> m_scanArm = nullptr;
    std::shared_ptr<SourceScanChunks> m_scanArmChunks = nullptr;
    std::unique_ptr<QTimer> m_updateTimer;
    std::unique_ptr<QTimer> m_findPeakTimer;
    Area* m_area;
    size_t m_curIndex = 0;
    size_t m_curChunkIndex = 0;
    size_t m_curTrajectoryIndex = 0;
    size_t m_numArea = 0;

public:
    FieldWidget(int width,int height);
    ~FieldWidget();
    void setScan(std::shared_ptr<ScanArm> scan);
    void setScanChunks(std::shared_ptr<SourceScanChunks> scanChunks);
    void setNumArea(size_t numArea);
    std::shared_ptr<ScanArm> scan() { return m_scanArm; }

public slots:
    void timeout();
    void findPeak();
    void changeMode(int btn, bool value);
    void stopTimers();
    void runTimers();
    void drawArea();
private:
    ::std::vector< float > *data;
    ::std::vector< ::Position > *dataCoord;
    uint backStep=0;
    uint foreStep=0;
    uint pause=0;
    double comparator=0.00001;
    double koeffOfSamplesRate = 1.0;
    unsigned int soundSampleRate = 1;
    bool getCoordinateOfPeak(size_t indexInSound, ::Peak& peak);

};

#endif // FIELDWIDGET_H
