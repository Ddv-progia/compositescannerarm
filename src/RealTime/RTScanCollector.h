#pragma once

#include "Core/ProgressReportingTask.hh"
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
		struct LastPosition {
			float x = 0.0;
			float y = 0.0;
			float z = 0.0;
			std::time_t m_timeStampLast = 0;
		}m_positionLast;
		time_t m_minimumTimeStampProlong = 10;
		static bool m_isStarted;
		bool m_isThisStarted = false;
		bool m_isThisCursorStarted = false;
		const size_t SEC_PER_MINUTE = 60;
		//const size_t MAXIMUM_TIME = 60;
		//const size_t SOUDS_SAMPLE_RATE = 100000;
		//const size_t HEAD_SAMPLE_RATE = 100;
		//const int WIDTH = 100;
		//const int HEIGHT = 50;
		
		realtime::RTContext& m_rtCtxt;
		ProcessingParameters m_parameters;
		ScanFactory& scanFactory;
		SoundDisplay *m_soundDisplay;
		PeakDisplay* m_peakDisplay;
		RTScanCollector() = delete;

	public:
		//std::shared_ptr<Scan> ScanArmToScan(std::shared_ptr<ScanArm> scanArm);
		void ScanArmToScan(std::shared_ptr<ScanArm> scanArm, std::shared_ptr<Scan>& scanIn);
		void SetShift(float x, float y, float z);
		void operator()() {};

		FieldWidget* m_field;
		explicit RTScanCollector(RTContext& trCtxt, ProcessingParameters &parameters, ScanFactory& scanFactory);
		explicit RTScanCollector(RTContext& trCtxt, ScanFactory& scanFactory, ScanArm scanArm);
		void resizeRtPeaks(int width, int height);
		Q_SLOT void setAreaAdditionalScale(double value);
		Q_SLOT virtual void start() override;
		Q_SLOT virtual void startCursor();
		Q_SLOT virtual void stop() override;
		Q_SLOT virtual void pause() override {};
		Q_SLOT void headData(float x, float y, float z, time_t timeStamp);
		Q_SLOT void audioData(size_t startpositionOfChunk, size_t sizeOfChunk, std::time_t timeStampNewData, std::time_t timeStampFromChunk);
		Q_SLOT virtual void save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
		Q_SLOT virtual void saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
		Q_SLOT void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0)  override;
		Q_SLOT void makeScanAndShow(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea = 0);
		Q_SLOT void makeScanAndShowCuttered(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea = 0);
		Q_SLOT void makeScanAndShowCutteredAbs(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea = 0);



		//virtual void load(BackgroundTaskExecutor& taskExecutor) override;

		virtual ~RTScanCollector();
	protected:
		//ProgressReportingTask
		Q_SIGNAL void started(const QString& name, int stageCount);
		Q_SIGNAL void stageStarted(const QString& name, int maximumValue);
		Q_SIGNAL void stageProgressed();
		Q_SIGNAL void finished();
		Q_SIGNAL void terminated(const QString& errorMessage);
		Q_SIGNAL void createdTask(ProgressReportingTask* newTask);
		//end ProgressReportingTask
	};
}