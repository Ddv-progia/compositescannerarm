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
    std::shared_ptr<Scan> m_scan = nullptr;
    boost::sync_queue < PixelInfo > m_srcCoords;
    int m_width, m_height;
    double m_pixelpermm = 1;
    void virtual run();
    void swapPixMap();
    float max = 0;
    float min = 0;
public:
    Area(int width , int height);
    ~Area();
    void setScan(std::shared_ptr<Scan> scan);
    void drawPoint(int x, int y);
    void clear();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};

#endif // AREA_H
