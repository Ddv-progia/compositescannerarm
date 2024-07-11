#include "fieldwidget.h"
#include "cursor.h"
#include "area.h"
#include <QtCore/qtimer.h>
#include <iostream>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qbuttongroup.h>
#include <QOpenGLWidget>

FieldWidget::
FieldWidget(int width, int height) 
	: m_width(width), m_height(height) {
	m_scene = new QGraphicsScene;
	m_cursor = new Cursor(m_width, m_height);
	m_area = new Area(m_width, m_height,Qt::lightGray);
	m_scene->addItem(m_area);
	m_scene->addItem(m_cursor);


	setScene(m_scene);

	m_updateTimer = std::make_unique<QTimer>();
	//m_updateTimer->start(100);
	connect(m_updateTimer.get(), &QTimer::timeout, this, &FieldWidget::timeout);

	m_findPeakTimer = std::make_unique<QTimer>();
	//m_findPeakTimer->start(100);
	connect(m_findPeakTimer.get(), &QTimer::timeout, this, &FieldWidget::findPeak);

	QVBoxLayout* layout = new QVBoxLayout();
	QButtonGroup* group = new QButtonGroup();
	auto rb1 = new QRadioButton("1mm");
	auto rb2 = new QRadioButton("5mm");
	auto rb3 = new QRadioButton("10mm");
	group->addButton(rb1,0);
	group->addButton(rb2,1);
	group->addButton(rb3,2);
	layout->addWidget(rb1);
	layout->addWidget(rb2);
	layout->addWidget(rb3);
	this->setLayout(layout);
	connect(group, SIGNAL(idToggled(int, bool)),this,SLOT( changeMode(int , bool )));
	//setViewport(new QOpenGLWidget);
}

void FieldWidget::changeMode(int btn, bool value) {
	if(value)
		m_numArea = btn * 2ll;
}

void FieldWidget::drawArea()
{
	m_area->drawAllPoints();
}

void FieldWidget::stopTimers()
{
	m_updateTimer->stop();
	m_findPeakTimer->stop();
}

void FieldWidget::runTimers()
{
	m_updateTimer->start(100);
	m_findPeakTimer->start(100);
}

FieldWidget::
~FieldWidget() {

}

void FieldWidget::
setScan(std::shared_ptr<ScanArm> scan) {
	m_scan = scan;
	m_scan->rtPeaks.resize(m_height);
	int coord = 0;
	for (auto& linePeak : m_scan->rtPeaks)
		linePeak.resize(m_width);
	m_area->clear();
	m_area->setScan(m_scan);
	m_curIndex = 0;
}

void FieldWidget::
setNumArea(size_t numArea) {
	m_numArea = numArea;
}

void FieldWidget::
timeout() {
	if ( !m_scan)
		return;
	if (!m_scan->trajectory.pos.empty()) {
		m_cursor->setPosition(
			m_scan->trajectory.pos.back().x,
			m_scan->trajectory.pos.back().y
		);
	}
	m_area->start();
}

void FieldWidget::
findPeak() {
	//std::srand((unsigned int)time(0));
	try {
		//if (!m_scan || m_scan->trajectory.pos.empty())
		if (!m_scan )
			return;

		const auto& parameters = m_scan->parameters;
		uint backStep = m_scan->sound.sampleRate * parameters.peakBackstep;
		uint foreStep = m_scan->sound.sampleRate * parameters.peakForestep;
		uint pause = m_scan->sound.sampleRate * parameters.peakPauseCount;
		auto& data = m_scan->sound.samples;
		auto& dataCoord = m_scan->trajectory.pos;
		int size = data.size();
		double comparator = m_scan->parameters.peakMagnitudeLimit;
		for (; m_curIndex < size; ++m_curIndex) {
			if (abs(data.at(m_curIndex)) > comparator) {
				std::cout << " * *******fieldWidjet Out " << abs(data.at(m_curIndex)) << ":" << comparator << std::endl;
				//int indCoord = m_curIndex / 1000; //TODO ???? magic number ???
				int indCoord = m_curIndex / (m_scan->sound.sampleRate/ m_scan->trajectory.sampleRate);
				int x;
				int y;
				int z;
				if (indCoord >= dataCoord.size()) {
					return; // ремарим в отсутствии данных и эмулируем случайную траекторию
				}           // ремарим в отсутствии данных и эмулируем случайную траекторию
				//	auto height = parameters.headAndScanCollectorParameters.height;
				//	auto width  = parameters.headAndScanCollectorParameters.width;
				//	//x =(double)(rand()) / RAND_MAX * (max - min) + min;
				//	x = (int)(rand()) / RAND_MAX * (width - 0) + 0;;
				//	y = (int)(rand()) / RAND_MAX * (height - 0) + 0;;
				//	z = (int)0;
				//}
				//else {
				//	x = (int)dataCoord.at(indCoord).x;
				//	y = (int)dataCoord.at(indCoord).y;
				//	z = (int)dataCoord.at(indCoord).z;
				//}
					x = (int)dataCoord.at(indCoord).x;
					y = (int)dataCoord.at(indCoord).y;
					z = (int)dataCoord.at(indCoord).z;
				uint indBegin = m_curIndex - backStep;
				uint indEnd = m_curIndex + foreStep;
				if (indEnd >= size) {
					m_curIndex = indBegin; 
					return;
				}
				//m_curIndex += pause;
				m_curIndex = indEnd + pause;

				m_scan->rtPeaks.at(y).at(x).push_back({ indBegin, indEnd });
				m_area->drawPoint(x, y);
				for (int indy = std::max((y - m_numArea), size_t(0)); indy <= std::min(size_t(y + m_numArea), m_scan->rtPeaks.size() - 1); ++indy) {
					for (int indx = std::max((x - m_numArea), size_t(0)); indx <= std::min(size_t(x + m_numArea), m_scan->rtPeaks.at(indy).size() - 1); ++indx) {
							m_scan->rtPeaks.at(indy).at(indx).push_back({ indBegin, indEnd });
							if (m_scan->rtPeaks.at(indy).at(indx).size() > 4)
								m_scan->rtPeaks.at(indy).at(indx).pop_front();
							m_area->drawPoint(indx, indy);
					}
				}
				
			}

		}
	}
	catch (...) {
		std::cout << "find peak out of range \n";
	}

}