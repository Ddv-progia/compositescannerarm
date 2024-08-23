#include "area.h"
#include <QtGui/qpainter.h>
#include <iostream>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>


Area::
Area(int width, int height, Qt::GlobalColor backgrColor)
	: m_width(width), m_height(height), m_backgroundColor(backgrColor)
{
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4);
	m_curentPixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_doublePixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_curentPixmap->fill(m_backgroundColor);
	m_doublePixmap->fill(m_backgroundColor);
	for (int j = 0; j < m_height; j++) {
		std::vector<double> newvec;
		for (int i = 0; i < m_width; i++) {
			newvec.push_back(0.0);
		}
		peaksValue.push_back(newvec);
	}
	m_painter = new QPainter(m_curentPixmap);
}

Area::
~Area() {

}

void Area::setScan(std::shared_ptr<ScanArm> scan) {
	m_scanArm = scan;
	
	start();
}

float averagePeaksAmplitude(const std::list<Peak> &peaks, const ::std::vector< float > &data) {
	try {
		double summ = 0;
		for (auto peak : peaks) {
			auto max = std::max_element(data.begin() + peak.beginIndex, data.begin() + peak.endIndex);
			if (max == data.end()) //TODO заремарить либо выдать ошибку
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
	//QPainter painter(m_curentPixmap);
	//m_doublePixmap->fill(QColor(0, 0, 0, 0));
	PixelInfo pixel;
	QColor color;
	//bool m_needRecalculateAfterMaxMinChange = false;
	while (!m_srcCoords.empty()) {
		m_srcCoords.pull(pixel);
		int f_y = pixel.y * m_pixelpermm;
		int f_x = pixel.x * m_pixelpermm;
		if (m_scanArm->rtPeaks.at(pixel.y).at(pixel.x).empty())
			continue;
		float value = getValueFromPeaks(&m_scanArm->rtPeaks.at(pixel.y).at(pixel.x), &m_scanArm->sound.samples);
		////float value = averagePeaksAmplitude(m_scanArm->rtPeaks.at(pixel.y).at(pixel.x), m_scanArm->sound.samples);
		////peaksValue.at(pixel.y).at(pixel.x) = value;
		//float value = m_scanArm->rtPeaks.at(pixel.y).at(pixel.x).size();
		peaksValue.at(pixel.y).at(pixel.x) = value;
		if (max < value || min > value) {
			max = max > value ? max : value;
			min = min < value ? min : value;
			m_needRecalculateAfterMaxMinChange = true;
		}
		float converted = (value - min) / (max - min);
		color.setHsvF(converted, 1, 1, 1);
		//painter.setPen({ color, 1 * m_pixelpermm + 1 });
		//painter.drawPoint(f_x, f_y);
		m_painter->setPen({ color, 1 * m_pixelpermm + 1 });
		m_painter->drawPoint(f_x, f_y);
	}
	if (m_needRecalculateAfterMaxMinChange) {
		drawAllPoints();

		m_needRecalculateAfterMaxMinChange = false;
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
	m_curentPixmap->fill(m_backgroundColor);
	m_doublePixmap->fill(m_backgroundColor);
}

void Area::findMinMax()
{
	bool notFoundYet = true;
	for (int y = 0; y < m_scanArm->rtPeaks.size(); ++y) {
		int f_y = y * m_pixelpermm;
		for (int x = 0; x < m_scanArm->rtPeaks.at(y).size(); ++x) {
			int f_x = x * m_pixelpermm;
			if (m_scanArm->rtPeaks.at(y).at(x).empty())
				continue;
			float value = getValueFromPeaks(&m_scanArm->rtPeaks.at(y).at(x), &m_scanArm->sound.samples);
			if (notFoundYet) {
				max = value;
				min = 0;
				notFoundYet = false;
			}
			else {
				if (max < value)
					max = value;
				else if(min > value) min = value;
			}
		}
	}

}

float Area::getValueFromPeaks(const ::std::list< ::Peak >* peaks, const ::std::vector< float >* samples)
{
	//float value = averagePeaksAmplitude(*peaks, *samples);
	float value = peaks->size();
	return value;
}

void Area::drawAllPoints()
{
	QColor color;

	for (int y = 0; y < m_scanArm->rtPeaks.size(); ++y) {
		int f_y = y * m_pixelpermm;
		for (int x = 0; x < m_scanArm->rtPeaks.at(y).size(); ++x) {
			int f_x = x * m_pixelpermm;
			if (m_scanArm->rtPeaks.at(y).at(x).empty())
				continue;
			float value = 0.0;
			//if (peaksValue.at(y).at(x) != 0) {
			//	value = peaksValue.at(y).at(x);
			//}
			//else {
			//	//value = averagePeaksAmplitude(m_scanArm->rtPeaks.at(y).at(x), m_scanArm->sound.samples);
			//	//peaksValue.at(y).at(x) = value;
			//	value = m_scanArm->rtPeaks.at(y).at(x).size();
			//	peaksValue.at(y).at(x) = value;
			//}
			value = getValueFromPeaks(&m_scanArm->rtPeaks.at(y).at(x), &m_scanArm->sound.samples);
			peaksValue.at(y).at(x) = value;

			//float value = averagePeaksAmplitude(m_scanArm->rtPeaks.at(y).at(x), m_scanArm->sound.samples);
			//if (max < value || min < value) {
			//	max = max > value ? max : value;
			//	min = min < value ? min : value;
			//}
			
			float converted = (max == min) ? value:(value - min) / (max - min);
			color.setHsvF(converted, 0, 1, 1);
			////painter.setPen({ color, 1 * m_pixelpermm + 1 });
			////painter.drawPoint(f_x, f_y);
			//m_painter->setPen({ color, 1 * m_pixelpermm + 1 });
			m_painter->setPen(color);
			m_painter->drawPoint(f_x, f_y);
			m_painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
		}
	}
}

void Area::
paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}