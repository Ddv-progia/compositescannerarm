#include "area.h"
#include <QtGui/qpainter.h>
#include <iostream>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>


Area::
Area(int width, int height, Qt::GlobalColor backgrColor, float maxCountOfPeak)
	: m_width(width), m_height(height), m_backgroundColor(backgrColor), m_maxCountOfPeak(maxCountOfPeak)
{
	max = m_maxCountOfPeak;
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4)*m_additionalScale;
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
	//auto pixmap = m_curentPixmap->scaled(width, height, Qt::AspectRatioMode::KeepAspectRatio);
	//m_painter = new QPainter(&pixmap);
	
}

Area::
~Area() {

}

void Area::setScan(std::shared_ptr<ScanArm> scan) {
	m_scanArm = scan;
	m_maxCountOfPeak = m_scanArm->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox;
	max = m_maxCountOfPeak;

	reDrawAllPoints();
	drawAllPoints();
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
		int f_y = pixel.y * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
		int f_x = pixel.x * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
		if (m_scanArm->rtPeaks.at(pixel.y).at(pixel.x).empty())
			continue;
		float value = getValueFromPeaks(&m_scanArm->rtPeaks.at(pixel.y).at(pixel.x), &m_scanArm->sound.samples);
		////float value = averagePeaksAmplitude(m_scanArm->rtPeaks.at(pixel.y).at(pixel.x), m_scanArm->sound.samples);
		////peaksValue.at(pixel.y).at(pixel.x) = value;
		peaksValue.at(pixel.y).at(pixel.x) = value;
		//if (max < value || min > value) {
		//	max = max > value ? max : value;
		//	min = min < value ? min : value;
		//	m_needRecalculateAfterMaxMinChange = true;
		//}
		float converted = (value - min) / (max - min);
		//color.setHsvF(converted, 1, 1, 1);
		color = setColorForPoint(converted);

		//painter.setPen({ color, 1 * m_pixelpermm + 1 });
		//painter.drawPoint(f_x, f_y);
		m_painter->setPen({ color, 1 * m_pixelpermm * m_numArea });
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

void Area::reDrawAllPoints()
{
	QColor color;

	for (int y = 0; y < peaksValue.size(); ++y) {
		int f_y = y * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
		if (peaksValue.at(y).empty())
			continue;
		for (int x = 0; x < peaksValue.at(y).size(); ++x) {
			int f_x = x * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
			float value = 0.0;
			value = peaksValue.at(y).at(x);
			float converted = (max == min) ? value:(value - min) / (max - min);
			//color.setHsvF(converted, 1, 1, 1);
			color = setColorForPoint(converted);

			//m_painter->setPen({ color, 1 * m_pixelpermm + 1 });
			m_painter->setPen({ color, 1 * m_pixelpermm * m_numArea });
			m_painter->drawPoint(f_x, f_y);
		}
	}
	m_painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}

void Area::drawAllPoints()
{
	QColor color;

	for (int y = 0; y < m_scanArm->rtPeaks.size(); ++y) {
		int f_y = y * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
		for (int x = 0; x < m_scanArm->rtPeaks.at(y).size(); ++x) {
			int f_x = x * m_pixelpermm + (m_numArea * m_pixelpermm) / 2;
			if (m_scanArm->rtPeaks.at(y).at(x).empty())
				continue;
			float value = 0.0;
			value = getValueFromPeaks(&m_scanArm->rtPeaks.at(y).at(x), &m_scanArm->sound.samples);
			peaksValue.at(y).at(x) = value;

			//float value = averagePeaksAmplitude(m_scanArm->rtPeaks.at(y).at(x), m_scanArm->sound.samples);
			//if (max < value || min < value) {
			//	max = max > value ? max : value;
			//	min = min < value ? min : value;
			//}
			
			float converted = (max == min) ? value:(value - min) / (max - min);
			//color.setHsvF(converted, 1, 1, 1);
			color = setColorForPoint(converted);

			m_painter->setPen({ color, 1 * m_pixelpermm * m_numArea });
			m_painter->drawPoint(f_x, f_y);
		}
	}
	m_painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}

void Area::setNumArea(size_t numAreaValue)
{
	m_numArea = numAreaValue;
}

void Area::setAdditionalScale(double numAreaValue)
{
	m_additionalScale = numAreaValue;
	m_pixelpermm = (QGuiApplication::primaryScreen()->physicalDotsPerInch() / 25.4) * m_additionalScale;
	m_curentPixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_doublePixmap = new QImage(m_width * m_pixelpermm, m_height * m_pixelpermm, QImage::Format_ARGB32);
	m_curentPixmap->fill(m_backgroundColor);
	m_doublePixmap->fill(m_backgroundColor);
	m_painter = new QPainter(m_curentPixmap);
	m_painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());

}

QColor Area::setColorForPoint(float value)
{
	QColor color;
	//////if (m_numArea < 2) color.setRgbF(0.5, value, value, 1);
	////if (m_numArea < 2) color.setHsvF(0.15, value, value, 1);
	////else if (m_numArea > 2) color.setHsvF(0.6, value, value, 1);
	////else color.setHsvF(0.3, value, value, 1);
	//if (m_numArea < 2) color.setHsvF(0.15, 1, value, 1);
	//else if (m_numArea > 2) color.setHsvF(0.6, 1, value, 1);
	//else color.setHsvF(0.3, 1, value, 1);
	switch (m_numArea) {
	case 1:color.setHsvF(0.15, 1, value, 1); break;
	case 3:color.setHsvF(0.3, 1, value, 1); break;
	case 5:color.setHsvF(0.45, 1, value, 1); break;
	case 10:color.setHsvF(0.6, 1, value, 1); break;
	default:color.setHsvF(0.95, 1, .5, 1);
	}
	return color;
}

void Area::
paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	painter->drawImage(boundingRect(), *m_curentPixmap, boundingRect());
}