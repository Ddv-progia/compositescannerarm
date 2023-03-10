#include "area.h"
#include <QtGui/qpainter.h>
#include <iostream>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>


Area::
Area(int width, int height) {
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4) * 2;
	m_curentPixmap = new QImage(width * m_pixelpermm, height * m_pixelpermm, QImage::Format_ARGB32);
	m_doublePixmap = new QImage(width * m_pixelpermm, height * m_pixelpermm, QImage::Format_ARGB32);
	m_curentPixmap->fill(Qt::black);
	m_doublePixmap->fill(Qt::black);
}

Area::
~Area() {

}

void Area::
run() {
    QPainter painter(m_curentPixmap);
    //m_doublePixmap->fill(QColor(0, 0, 0, 0));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    //pen->setCapStyle(Qt::RoundCap);
	painter.setPen({ Qt::green,1 * m_pixelpermm });
	QPoint p;
    //swapPixMap();
	while (!m_srcCoords.empty()) {
		m_srcCoords.pull(p);
		int x = p.x() * m_pixelpermm;
		int y = p.y() * m_pixelpermm;
		if(x > 0 && x < m_curentPixmap->width()
			&& y > 0 && y < m_curentPixmap->height())
			painter.drawPoint(x, y);
	}
}

void Area::
swapPixMap() {
	QImage* ptr = m_curentPixmap;
	m_curentPixmap = m_doublePixmap;
	m_doublePixmap = ptr;
	ptr = nullptr;
}

QRectF Area::
boundingRect() const {
	return QRectF{ 0, 0, (qreal)m_curentPixmap->width(), (qreal)m_curentPixmap->height() };
}
void Area::drawPoint(int x, int y) {
	m_srcCoords.push({ x, y });
	start();
}

void Area::clear() {
	m_curentPixmap->fill(Qt::black);
	m_doublePixmap->fill(Qt::black);
}

void Area::
paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}