#ifndef AREA_H
#define AREA_H

#include "Core/ScanData.hh"
#include <QtWidgets/qgraphicsitem.h>
#include <QtCore/qthread.h>
#include <boost/thread/sync_queue.hpp>

class Area : public QThread, public QGraphicsItem
{
    float m_x = 0;
    float m_y = 0;
    QImage* m_curentPixmap, * m_doublePixmap;
    boost::sync_queue < QPoint > m_srcCoords;
    double m_pixelpermm = 1;
    void virtual run();
    void swapPixMap();
public:
    Area(int width , int height);
    ~Area();
    void drawPoint(int x, int y);
    void clear();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;
};

#endif // AREA_H
