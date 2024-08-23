#include "cursor.h"
#include <QtGui/qpainter.h>
#include <iostream>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

Cursor::
Cursor(int width, int height, Qt::GlobalColor color)
: m_width(width) , m_height(height), m_color(color){
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4 );
	m_width *= m_pixelpermm;
	m_height *= m_pixelpermm;
}

Cursor::
~Cursor() {

}

void Cursor::
setPosition(float x, float y) {
	m_x = x * m_pixelpermm;
	m_y = y * m_pixelpermm;
	update();
}

std::pair<float, float> Cursor::getPositionXY()
{
	return std::pair<float, float>(m_x, m_y);
}

QRectF Cursor::
boundingRect() const {
	return QRectF{ -m_pixelpermm,  -m_pixelpermm, (qreal)m_width + m_pixelpermm * 2, (qreal)m_height + m_pixelpermm * 2 };
}

void Cursor::
paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	painter->setPen({ m_color, 1 * m_pixelpermm });
	painter->drawPoint(m_x, m_y);
	painter->drawRect(boundingRect());
}