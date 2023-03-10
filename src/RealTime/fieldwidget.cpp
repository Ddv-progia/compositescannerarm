#include "fieldwidget.h"
#include "cursor.h"
#include "area.h"
#include <QtCore/qtimer.h>
#include <iostream>


FieldWidget::
FieldWidget() {
	m_scene = new QGraphicsScene;
	m_cursor = new Cursor(WIDTH, HEIGHT);
	m_area = new Area(WIDTH, HEIGHT);
	m_scene->addItem(m_area);
	m_scene->addItem(m_cursor);
	
	setScene(m_scene);

	m_updateTimer = std::make_unique<QTimer>();
	m_updateTimer->start(10);
	connect(m_updateTimer.get(), &QTimer::timeout, this, &FieldWidget::timeout);

	m_findPeakTimer = std::make_unique<QTimer>();
	m_findPeakTimer->start(1000);
	connect(m_findPeakTimer.get(), &QTimer::timeout, this, &FieldWidget::findPeak);

}

FieldWidget::
~FieldWidget() {

}

void FieldWidget::
setScan(std::shared_ptr<Scan> scan) {
	m_scan = scan;
	m_area->clear();
	m_scan->peaks.resize(HEIGHT);
	m_curIndex = 0;
	
	int coord = 0;
	for (auto& linePeak : m_scan->peaks) {
		linePeak.peaks.resize(WIDTH);
		linePeak.startCoordinate = 0;
		linePeak.finalCoordinate = WIDTH;
		linePeak.lineCoordinate = coord++;
		linePeak.finalLineCoordinate = coord;
		linePeak.timestampStart = coord;
		linePeak.finalLineCoordinate = coord + 1.0;
	}
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
					m_area->drawPoint(
						(int)dataCoord.at(m_curIndex / 1000).x,
						(int)dataCoord.at(m_curIndex / 1000).y
					);
					m_curIndex += 100;
			}

		}
	}
	catch (...) {
		std::cout << "data\n";
	}

}