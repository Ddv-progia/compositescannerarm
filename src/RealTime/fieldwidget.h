#pragma once

#ifndef FIELDWIDGET_H
#define FIELDWIDGET_H


#include "Core/ScanData.hh"
#include <QtWidgets/qbuttongroup.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtWidgets/qgraphicsscene.h>

class Cursor;
class Area;
class FieldWidget : public QGraphicsView
{
    Q_OBJECT
    int m_width;
    int m_height;
    int m_maxCountOfPeak;
    QGraphicsScene* m_scene;
    std::shared_ptr<ScanArm> m_scanArm = nullptr;
    std::shared_ptr<SourceScanChunks> m_scanArmChunks = nullptr;
    std::unique_ptr<QTimer> m_updateTimer;
    std::unique_ptr<QTimer> m_findPeakTimer;
    Area* m_area;
    size_t m_curIndex = 0;
    //std::vector<float>::iterator m_currentSampleIndex;
    size_t m_curChunkIndex = 0;
    size_t m_curTrajectoryIndex = 0;
    size_t m_numArea = 0;
    float m_xCursorPosition = 0.0;
    float m_yCursorPosition = 0.0;

public:
    FieldWidget(int width,int height, int maxCountOfPeak = 2);
    ~FieldWidget();
    Cursor* m_cursor;
    void resizeRtPeaks(int width, int height);
    void setScan(std::shared_ptr<ScanArm> scan);
    void setFirstStepShift(size_t value);
    void setScanChunks(std::shared_ptr<SourceScanChunks> scanChunks);
    size_t getNumArea();
    void setNumArea(size_t numArea);
    std::shared_ptr<ScanArm> scan() { return m_scanArm; }
    QButtonGroup* group = new QButtonGroup();
    float getPositionX();
    float getPositionY();

public slots:
    void timeout();
    void findPeak();
    void changeMode(int btn, bool value);
    void stopTimers();
    void runTimers();
    void runCursorTimer();
    void drawArea();
    void setAreaAdditionalScale(double valueAreaAdditionalScale);
private:
    ::std::vector< float > *data;
    ::std::vector< ::Position > *dataCoord;
    uint backStep=0;
    uint foreStep=0;
    uint pause=0;
    uint m_firstStepShift = 0;
    double comparator=0.00001;
    double koeffOfSamplesRate = 1.0;
    unsigned int soundSampleRate = 1;
    bool getCoordinateOfPeak(size_t indexInSound, ::Peak& peak);

};

#endif // FIELDWIDGET_H
