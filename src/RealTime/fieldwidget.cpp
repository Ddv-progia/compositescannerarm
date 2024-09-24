#include <boost/format.hpp>
#include <UCL/Exception.hh>

#include "fieldwidget.h"
#include "cursor.h"
#include "area.h"
#include <QtCore/qtimer.h>
#include <iostream>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qlayout.h>
#include <QOpenGLWidget>

FieldWidget::
FieldWidget(int width, int height, int maxCountOfPeak)
	: m_width(width), m_height(height), m_maxCountOfPeak(maxCountOfPeak){
	m_scene = new QGraphicsScene;
	m_cursor = new Cursor(m_width, m_height);
	m_area = new Area(m_width, m_height,Qt::lightGray, maxCountOfPeak);
	m_scene->addItem(m_area);
	m_scene->addItem(m_cursor);


	setScene(m_scene);

	m_updateTimer = std::make_unique<QTimer>();
	m_updateTimer->setInterval(100);
	//m_updateTimer->start(100);
	connect(m_updateTimer.get(), &QTimer::timeout, this, &FieldWidget::timeout);

	m_findPeakTimer = std::make_unique<QTimer>();
	m_findPeakTimer->setInterval(200);
	//m_findPeakTimer->start(100);
	connect(m_findPeakTimer.get(), &QTimer::timeout, this, &FieldWidget::findPeak);

	QVBoxLayout* layout = new QVBoxLayout();

	auto rb1 = new QRadioButton("1mm");
	auto rb3 = new QRadioButton("3mm");
	auto rb5 = new QRadioButton("5mm");
	auto rb10 = new QRadioButton("10mm");
	group->addButton(rb1,1);
	group->addButton(rb3,3);
	group->addButton(rb5,5);
	group->addButton(rb10,10);
	layout->addWidget(rb1);
	layout->addWidget(rb3);
	layout->addWidget(rb5);
	layout->addWidget(rb10);
	this->setLayout(layout);
	
	connect(group, SIGNAL(idToggled(int, bool)),this,SLOT( changeMode(int , bool )));
	rb5->toggle();

	//setViewport(new QOpenGLWidget);
}

void FieldWidget::changeMode(int btn, bool value) {
	if (value) {
		//m_numArea = btn * 2ll;
		m_numArea = btn;
		m_area->setNumArea(m_numArea);
		//resizeRtPeaks(m_width / m_numArea, m_height / m_numArea);
		if (m_scanArm != nullptr) {
			m_scanArm->parameters.headAndScanCollectorParameters.currentNumArea = m_numArea;
		}
	}
}

void FieldWidget::drawArea()
{
	//m_area->drawPoint(0.0, 0.0);
	m_area->findMinMax();
	if (!m_scanArm->rtPeaks.at(0).at(0).empty())
		m_area->drawPoint(m_area->m_scanArm->rtPeaks.at(0).at(0).front().x, m_area->m_scanArm->rtPeaks.at(0).at(0).front().y );
	else m_area->drawPoint(0.0, 0.0);
	m_area->m_needRecalculateAfterMaxMinChange = true;
	m_area->drawAllPoints();
	m_area->start();
}

float FieldWidget::getPositionX()
{
	return m_xCursorPosition;
}

float FieldWidget::getPositionY()
{
	return m_yCursorPosition;
}



/// <summary>
/// ищет координаты пика по timestamp'у
/// заполняет peak найденными координатами
/// </summary>
/// <param name="indexInSound"></param>
/// <param name="peak"></param>
/// <returns>true, если координаты найдены</returns>
bool FieldWidget::getCoordinateOfPeak(size_t indexInSound, ::Peak& peak)
{
	auto bindex = peak.beginIndex;
	auto eindex = peak.endIndex;
	auto dist = eindex - bindex;

	bool notFoundCurChunkIndex = true;
	unsigned long long int timestampForIndexInSound = 0;
	//std::cout << "getCoordinateOfPeak while started !!! m_scanArmChunks->chunks.size() = " << m_scanArmChunks->chunks.size() << std::endl;
	//return false;
	while (m_curChunkIndex < m_scanArmChunks->chunks.size() && notFoundCurChunkIndex) {
		//std::cout << "m_curChunkIndex = " << m_curChunkIndex << std::endl;
		auto currChunk = m_scanArmChunks->chunks.at(m_curChunkIndex);
		if ((indexInSound >= currChunk.startpositionOfChunk) && (indexInSound < currChunk.endpositionOfChunk)) {
			timestampForIndexInSound = currChunk.timestamp-
				                       (unsigned long long int)((currChunk.endpositionOfChunk - indexInSound) * (1 / soundSampleRate));
			notFoundCurChunkIndex = false;     // нашли, выход из цикла
		}
		else {
			++m_curChunkIndex;
		}
	}
	if (notFoundCurChunkIndex) {
		m_curChunkIndex--;
		return false;
	}

	//size_t m_curIndex = 0;
	//size_t m_curChunkIndex = 0;
	//size_t m_curTrajectoryIndex = 0; // первый найденный индекс, по которому timestamp элемента в Trajectory больше,чем timestamp искомого пика

	//bool notFoundCurTrajectoryIndex = true;
	//size_t indCoord = 0;
	//for (; m_curTrajectoryIndex< dataCoord.size()) {
	
	auto currSizeOfTrajectory = m_scanArm->trajectory.pos.size();
	//std::cout << "m_curTrajectoryIndex while started !!!!!!" << m_curTrajectoryIndex << std::endl;
	//std::cout << "currSizeOfTrajectory  = " << currSizeOfTrajectory << std::endl;
	while (m_curTrajectoryIndex < currSizeOfTrajectory)  {
		//std::cout << "m_curTrajectoryIndex = " << m_curTrajectoryIndex << std::endl;
		auto curTrajectoryTimeStamp = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).timeStamp;
		if (timestampForIndexInSound  > curTrajectoryTimeStamp) {
			++m_curTrajectoryIndex;
		}
		else {
			if (timestampForIndexInSound == curTrajectoryTimeStamp) {
				peak.x = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).x;
				peak.y = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).y;
				peak.z = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).z;
				//notFoundCurTrajectoryIndex = false;
				return true;

			}
			else {
				if (m_curTrajectoryIndex > 0) {
					auto trajectoryIndexPred = m_curTrajectoryIndex - 1;
					auto dst = (curTrajectoryTimeStamp - m_scanArm->trajectory.pos.at(trajectoryIndexPred).timeStamp);
					if (dst) {
						auto kt = (curTrajectoryTimeStamp - timestampForIndexInSound) / dst;
						auto dx = (m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).x - m_scanArm->trajectory.pos.at(trajectoryIndexPred).x) * kt;
						auto dy = (m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).y - m_scanArm->trajectory.pos.at(trajectoryIndexPred).y) * kt;
						auto dz = (m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).z - m_scanArm->trajectory.pos.at(trajectoryIndexPred).z) * kt;

						peak.x = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).x - dx;
						peak.y = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).y - dy;
						peak.z = m_scanArm->trajectory.pos.at(m_curTrajectoryIndex).z - dz;
						//notFoundCurTrajectoryIndex = false;
						return true;
					}
					else ++m_curTrajectoryIndex;
				}
				else ++m_curTrajectoryIndex;
			}
		}
	}
	if (m_curTrajectoryIndex >= currSizeOfTrajectory) {
		m_curTrajectoryIndex = currSizeOfTrajectory - 1;
		if (m_curTrajectoryIndex < 0) m_curTrajectoryIndex = 0;
	}
	return false;
}

void FieldWidget::stopTimers()
{
	m_updateTimer->stop();
	m_findPeakTimer->stop();
}

void FieldWidget::runCursorTimer()
{
	m_updateTimer->start();
}

void FieldWidget::runTimers()
{
	////m_updateTimer->start(100);
	////m_findPeakTimer->start(200);
	//m_updateTimer->start();
	runCursorTimer();
	m_findPeakTimer->start();
}

FieldWidget::
~FieldWidget() {

}

//void FieldWidget::
//resizeRtPeaks(int width, int height) {
//	for (auto& linePeak : m_scanArm->rtPeaks){
//		for (auto& peaks : linePeak)
//			peaks.resize(0);
//		linePeak.resize(0);
//	}
//	m_scanArm->rtPeaks.resize(height);
//	for (auto& linePeak : m_scanArm->rtPeaks)
//		linePeak.resize(width);
//}


void FieldWidget::
setScan(std::shared_ptr<ScanArm> scanArmIn) {
	m_scanArm = scanArmIn;

	backStep = m_scanArm->sound.sampleRate * m_scanArm->parameters.peakBackstep;
	foreStep = m_scanArm->sound.sampleRate * m_scanArm->parameters.headAndScanCollectorParameters.peakForestepSound;
	pause    = m_scanArm->sound.sampleRate * m_scanArm->parameters.peakPauseCount;
	m_maxCountOfPeak = m_scanArm->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox;
	m_firstStepShift = m_scanArm->sound.sampleRate * m_scanArm->parameters.headAndScanCollectorParameters.firstStepShift;

	data = &m_scanArm->sound.samples;
	dataCoord = &m_scanArm->trajectory.pos;
	comparator = m_scanArm->parameters.peakMagnitudeLimit;
	koeffOfSamplesRate = m_scanArm->sound.sampleRate / m_scanArm->trajectory.sampleRate;
	soundSampleRate = m_scanArm->sound.sampleRate;
	////m_scanArm->rtPeaks.resize(m_height);
	////int coord = 0;
	////for (auto& linePeak : m_scanArm->rtPeaks)
	////	linePeak.resize(m_width);
	//resizeRtPeaks(m_width, m_height);
	m_area->clear();
	m_area->setScan(m_scanArm);
	if (auto btn = group->button(m_scanArm->parameters.headAndScanCollectorParameters.currentNumArea)) {
		btn->toggle();
	}
	m_curIndex = 0;
	m_curTrajectoryIndex = 0;
}

void FieldWidget::setAreaAdditionalScale(double valueAreaAdditionalScale)
{
	m_area->setAdditionalScale(valueAreaAdditionalScale);
}

void FieldWidget::
setScanChunks(std::shared_ptr<SourceScanChunks> scanChunks)
{
	m_scanArmChunks = scanChunks;
	m_curChunkIndex = 0;
}

size_t FieldWidget::
getNumArea( ) 
{
	return m_numArea;
}

void FieldWidget::
setNumArea(size_t numArea) {
	m_numArea = numArea;
	m_area->setNumArea(m_numArea);
}

void FieldWidget::
timeout() {
	if ( !m_scanArm)
		return;
	if (!m_scanArm->trajectory.pos.empty()) {
		 
		m_cursor->setPosition(
			m_scanArm->trajectory.pos.back().x,
			m_scanArm->trajectory.pos.back().y
		);
		m_xCursorPosition = m_scanArm->trajectory.pos.back().x;
		m_yCursorPosition = m_height - m_scanArm->trajectory.pos.back().y;

	}
	m_area->start();
}

void FieldWidget::
findPeak() {
	try {
		//if (!m_scanArm || m_scanArm->trajectory.pos.empty())
		if (!m_scanArm )
			return;
		
		int size = m_scanArm->sound.samples.size();
		if (foreStep == 0) {
				BOOST_THROW_EXCEPTION(uts::IncorrectArgumentException()
					<< uts::ErrInfo_Description((boost::format("Длительность пика слишком мала: %1% ") % foreStep ).str()));
			}
		//for (; m_currentSampleIndex != m_scanArm->sound.samples.end(); ++m_currentSampleIndex) {
		//	if (abs(*m_currentSampleIndex) > comparator) {
		if (m_numArea == 0)
		{
			std::cout << "(m_numArea == 0 ! in findpeak procedure \n";
			return;
		}
		for (; m_curIndex < size; ++m_curIndex) {
			if (abs(m_scanArm->sound.samples.at(m_curIndex)) > comparator) {
				::Peak peak;
				int x;
				int y;
				int z;

				bool rez = getCoordinateOfPeak(m_curIndex,peak);
				if (!rez) {
					//return;
					continue;
				}
				else {
					x = (int)m_numArea * std::floor(peak.x/ m_numArea);
					y = (int)m_numArea * std::floor(peak.y/ m_numArea);
					z = (int)peak.z;
					//int koeff = 2 * (int)m_numArea + 1;
					//x = (x / koeff)* koeff + (int)m_numArea;
					//y = (y / koeff)* koeff + (int)m_numArea;
					uint indBegin = m_curIndex - backStep;
					//indBegin = (uint)std::max((uint)0, static_cast<uint>(indBegin))
					if (indBegin < 0) {
						m_curIndex += (size_t)(m_firstStepShift);
						continue;
					}
					uint indEnd = m_curIndex + foreStep;
					if (indEnd >= size) {
						m_curIndex = indBegin;
						return;
					}
					//m_curIndex += pause;
					m_curIndex = indEnd ;

					if (m_scanArm->rtPeaks.at(y).at(x).size() >= m_maxCountOfPeak) {
						m_scanArm->rtPeaks.at(y).at(x).pop_front();
					}
					//if (m_scanArm->rtPeaks.at(y).at(x).size() < m_maxCountOfPeak) 
					{
						m_area->drawPoint(x, y);
						m_scanArm->rtPeaks.at(y).at(x).push_back({ indBegin, indEnd ,(double)x,(double)y,(double)z});
						////m_scanArm->rtPeaks.at(y).at(x).push_back({ indBegin, indEnd });
						//for (int indy = std::max((y - m_numArea), size_t(0)); indy <= std::min(size_t(y + m_numArea), m_scanArm->rtPeaks.size() - 1); ++indy) {
						//	for (int indx = std::max((x - m_numArea), size_t(0)); indx <= std::min(size_t(x + m_numArea), m_scanArm->rtPeaks.at(indy).size() - 1); ++indx) {
						//		m_scanArm->rtPeaks.at(indy).at(indx).push_back({ indBegin, indEnd });
						//		if (m_scanArm->rtPeaks.at(indy).at(indx).size() > 4)
						//			m_scanArm->rtPeaks.at(indy).at(indx).pop_front();
						//		m_area->drawPoint(indx, indy);
						//	}
						//}
					}
				}
			}

		}
		if (m_curIndex > size) {
			m_curIndex = size;
		}
	}
	catch (...) {
		std::cout << "find peak out of range \n";
	}

}