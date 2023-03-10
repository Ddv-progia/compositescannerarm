#pragma once

#include "Core/ScanCollector.hh"
#include "RTContext.h"
#include "onewavewidget.h"
#include "fieldwidget.h"

namespace realtime {
	class RTScanCollector : public ScanCollector
	{
		struct Shift {
			float x{ 0.0 };
			float y{ 0.0 };
			float z{ 0.0 };
			bool is_correct = false;
		}m_shift;
		realtime::RTContext& m_rtCtxt;
		ProcessingParameters m_parameters;
		OneWaveWidget *m_oneWave;
		FieldWidget *m_field;
		RTScanCollector() = delete;
	public:
		explicit RTScanCollector(RTContext& trCtxt, ProcessingParameters &parameters);
		Q_SLOT virtual void start() override;
		Q_SLOT virtual void pause() override;
		Q_SLOT virtual void stop() override;
		Q_SLOT void calcShift(float x, float y, float z);
		Q_SLOT void headData(float x, float y, float z);
		Q_SLOT void audioData();

		virtual ~RTScanCollector();
	};
}