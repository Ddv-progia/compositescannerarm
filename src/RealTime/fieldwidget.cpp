#include "fieldwidget.h"
#include "cursor.h"
#include "area.h"
#include <QtCore/qtimer.h>
#include <iostream>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qbuttongroup.h>


FieldWidget::
FieldWidget(int width, int height) 
	: m_width(width), m_height(height) {
	m_scene = new QGraphicsScene;
	m_cursor = new Cursor(m_width, m_height);
	m_area = new Area(m_width, m_height);
	m_scene->addItem(m_area);
	m_scene->addItem(m_cursor);


	setScene(m_scene);

	m_updateTimer = std::make_unique<QTimer>();
	m_updateTimer->start(100);
	connect(m_updateTimer.get(), &QTimer::timeout, this, &FieldWidget::timeout);

	m_findPeakTimer = std::make_unique<QTimer>();
	m_findPeakTimer->start(100);
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
	connect(group, SIGNAL(buttonToggled(int, bool)),this,SLOT( changeMode(int , bool )));

}

void FieldWidget::changeMode(int btn, bool value) {
	if(value)
		m_numArea = btn * 2;
}

FieldWidget::
~FieldWidget() {

}

void FieldWidget::
setScan(std::shared_ptr<Scan> scan) {
	m_scan = scan;
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
	if (!m_scan->originalScan.trajectory.pos.empty()) {
		m_cursor->setPosition(
			m_scan->originalScan.trajectory.pos.back().x,
			m_scan->originalScan.trajectory.pos.back().y
		);
	}
	m_area->start();
}

void FieldWidget::
findPeak() {
	try {
		if (!m_scan || m_scan->originalScan.trajectory.pos.empty())
			return;
		auto& data = m_scan->originalScan.sound.samples;
		auto& dataCoord = m_scan->originalScan.trajectory.pos;

		double comparator = m_scan->parameters.peakMagnitudeLimit;
		for (; m_curIndex < data.size(); ++m_curIndex) {
			if (abs(data.at(m_curIndex)) > comparator) {
				  
				int x = (int)dataCoord.at(m_curIndex / 1000).x;
				int y = (int)dataCoord.at(m_curIndex / 1000).y;
				//int z = (int)dataCoord.at(m_curIndex / 1000).z;
				
				uint indBegin = m_curIndex;
				m_curIndex += 4000;

				m_scan->peaks.at(y).at(x).push_back({ indBegin, indBegin + 1000, });
				m_area->drawPoint(x, y);
				for (int indy = std::max((y - m_numArea), size_t(0)); indy < std::min(size_t(y + m_numArea), m_scan->peaks.size() - 1); ++indy) {
					for (int indx = std::max((x - m_numArea), size_t(0)); indx < std::min(size_t(x + m_numArea), m_scan->peaks.at(indy).size() - 1); ++indx) {
							m_scan->peaks.at(indy).at(indx).push_back({ indBegin, indBegin + 1000, });
							if (m_scan->peaks.at(indy).at(indx).size() > 4)
								m_scan->peaks.at(indy).at(indx).pop_front();
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