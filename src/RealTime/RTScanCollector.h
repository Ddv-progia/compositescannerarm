#pragma once

#include "Core/ScanCollector.hh"
#include "RTContext.h"

#include "fieldwidget.h"
#include "Gui/Saveable.hh"
#include "Gui/Loadable.hh"

namespace realtime {
	class SoundDisplay;
	class PeakDisplay;

	class RTScanCollector : public ScanCollector, public Saveable, public Loadable
	{
		struct Shift {
			float x{ 0.0 };
			float y{ 0.0 };
			float z{ 0.0 };
			bool is_correct = false;
		}m_shift;
		static bool m_isStarted;
		bool m_isThisStarted = false;
		const size_t SEC_PER_MINUTE = 60;
		//const size_t MAXIMUM_TIME = 60;
		//const size_t SOUDS_SAMPLE_RATE = 100000;
		//const size_t HEAD_SAMPLE_RATE = 100;
		//const int WIDTH = 100;
		//const int HEIGHT = 50;
		
		realtime::RTContext& m_rtCtxt;
		ProcessingParameters m_parameters;
		SoundDisplay *m_soundDisplay;
		PeakDisplay* m_peakDisplay;
		RTScanCollector() = delete;
	public:
		FieldWidget* m_field;
		explicit RTScanCollector(RTContext& trCtxt, ProcessingParameters &parameters);
		Q_SLOT virtual void start() override;
		Q_SLOT virtual void stop() override;
		Q_SLOT virtual void pause() override {};
		Q_SLOT void headData(float x, float y, float z, time_t timeStamp);
		Q_SLOT void audioData(size_t startpositionOfChunk, size_t sizeOfChunk, std::time_t timeStampNewData, std::time_t timeStampFromChunk);
		Q_SLOT virtual void save(BackgroundTaskExecutor& taskExecutor) override;
		Q_SLOT virtual void saveAs(BackgroundTaskExecutor& taskExecutor) override;
		Q_SLOT void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0)  override;


		//virtual void load(BackgroundTaskExecutor& taskExecutor) override;

		virtual ~RTScanCollector();
	};
}