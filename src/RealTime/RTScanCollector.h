#pragma once

#include "Core/ScanCollector.hh"
#include "RTContext.h"
#include "onewavewidget.h"

namespace realtime {
	class RTScanCollector : public ScanCollector
	{
		realtime::RTContext& m_rtCtxt;
		OneWaveWidget* m_oneWave;
		RTScanCollector() = delete;
	public:
		explicit RTScanCollector(RTContext& trCtxt);
		Q_SLOT virtual void start() override;
		Q_SLOT virtual void pause() override;
		Q_SLOT virtual void stop() override;
		Q_SLOT void headData(float x, float y, float z);
		Q_SLOT void audioData();

		virtual ~RTScanCollector();
	};
}