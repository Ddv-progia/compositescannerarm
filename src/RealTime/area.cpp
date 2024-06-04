#include "area.h"
#include <QtGui/qpainter.h>
#include <iostream>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>


Area::
Area(int width, int height) 
	: m_width(width), m_height(height)
{
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4);
	m_curentPixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_doublePixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_curentPixmap->fill(Qt::black);
	m_doublePixmap->fill(Qt::black);
}

Area::
~Area() {

}

void Area::setScan(std::shared_ptr<Scan> scan) {
	m_scan = scan;
	
	start();
}

float maxPeak(const std::list<Peak> &peaks, const ::std::vector< float > &data) {
	try {
		double summ = 0;
		for (auto peak : peaks) {
			auto max = std::max_element(data.begin() + peak.beginIndex, data.begin() + peak.endIndex);
			if (max == data.end()) 
				return 0;
			summ += *max;
		}
		
		return summ / peaks.size();
	}
	catch (...) {
		return 0;
	}
}

void Area::
run() {
	QPainter painter(m_curentPixmap);
	//m_doublePixmap->fill(QColor(0, 0, 0, 0));
	PixelInfo pixel;
	QColor color;
	while (!m_srcCoords.empty()) {
		m_srcCoords.pull(pixel);
		int f_y = pixel.y * m_pixelpermm;
		int f_x = pixel.x * m_pixelpermm;
		if (m_scan->rtPeaks.at(pixel.y).at(pixel.x).empty())
			continue;
		float value = maxPeak(m_scan->rtPeaks.at(pixel.y).at(pixel.x), m_scan->sound.samples);

		if (max < value || min > value) {
			max = max > value ? max : value;
			min = min < value ? min : value;
			for (int y = 0; y < m_scan->rtPeaks.size(); ++y) {
				int f_y = y * m_pixelpermm;
				for (int x = 0; x < m_scan->rtPeaks.at(y).size(); ++x) {
					int f_x = x * m_pixelpermm;
					if (m_scan->rtPeaks.at(y).at(x).empty())
						continue;
					float value = maxPeak(m_scan->rtPeaks.at(y).at(x), m_scan->sound.samples);
					if (max < value || min < value) {
						max = max > value ? max : value;
						min = min < value ? min : value;
					}
					float converted = (value - min) / (max - min);
					color.setHsvF(converted, 1, 1, 1);
					painter.setPen({ color, 1 * m_pixelpermm + 1 });
					painter.drawPoint(f_x, f_y);
				}
			}
		}
		else {
			float converted = (value - min) / (max - min);
			color.setHsvF(converted, 1, 1, 1);
			painter.setPen({ color, 1 * m_pixelpermm + 1 });
			painter.drawPoint(f_x, f_y);
		}
	}
	//swapPixMap();
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
	m_srcCoords.push({x, y});
	//start();
}

void Area::clear() {
	m_curentPixmap->fill(Qt::black);
	m_doublePixmap->fill(Qt::black);
}

void Area::
paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}