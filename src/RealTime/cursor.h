#ifndef CURSOR_H
#define CURSOR_H

#include "Core/ScanData.hh"
#include <QtWidgets/qgraphicsitem.h>


class Cursor : public QGraphicsItem
{
    float m_x = 0;
    float m_y = 0;
    int m_width = 0;
    int m_height = 0;
    float m_pixelpermm = 1;
    Qt::GlobalColor m_color = Qt::white;
public:
    Cursor(int width , int height, Qt::GlobalColor color = Qt::white);
    ~Cursor();
    void setPosition(float x, float y);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;


};

#endif // CURSOR_H
