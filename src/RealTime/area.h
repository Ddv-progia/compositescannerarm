#ifndef AREA_H
#define AREA_H

#include "Core/ScanData.hh"
#include <QtWidgets/qgraphicsitem.h>
#include <QtCore/qthread.h>
#include <boost/thread/sync_queue.hpp>

class Area : public QThread, public QGraphicsItem
{
    struct PixelInfo{
        int x;
        int y;
    };
    float m_x = 0;
    float m_y = 0;
    QImage* m_curentPixmap, * m_doublePixmap;
    int m_width, m_height;
    Qt::GlobalColor m_backgroundColor = Qt::lightGray;
    double m_pixelpermm = 1.0;
    double m_additionalScale = 1.0;
    void virtual run();
    void swapPixMap();
    float max = 0.0;
    float min = 0.0;
    QPainter* m_painter;
    size_t m_numArea = 0;
    float m_maxCountOfPeak;
public:
    Area(int width , int height, Qt::GlobalColor backgrColor, float maxCountOfPeak);
    ~Area();
    boost::sync_queue < PixelInfo > m_srcCoords;
    std::shared_ptr<ScanArm> m_scanArm = nullptr;
    std::vector< std::vector<double>> peaksValue;
    void setScan(std::shared_ptr<ScanArm> scan);
    void setNumArea(size_t numAreaValue);
    void setAdditionalScale(double numAreaValue);
    QColor setColorForPoint(float value);
    void drawPoint(int x, int y);
    void clear();
    void drawAllPoints();
    void reDrawAllPoints();
    void findMinMax();
    float getValueFromPeaks(const ::std::list< ::Peak >* peaks, const ::std::vector< float > *samples);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
    bool m_needRecalculateAfterMaxMinChange = true;
};

#endif // AREA_H
