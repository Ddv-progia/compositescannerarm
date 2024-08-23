/*
 * Core/ScanCollector.hh
 */

#pragma once

#include <QtWidgets/QWidget>
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"
#include "Core/ScanFactory.hh"


class ScanCollector : public QWidget
{
  Q_OBJECT
//protected:

public:
	std::shared_ptr<ScanArm> m_scanArm;
	std::shared_ptr<Scan> m_scan;
	ScanCollector();
  std::shared_ptr <SourceScanChunks> sourceScanChunks;
  
  Q_SLOT virtual void start() = 0;
  Q_SLOT virtual void pause() = 0;
  Q_SLOT virtual void stop() = 0;
  Q_SLOT virtual void share();

protected:
  Q_SIGNAL void ready(std::shared_ptr<ScanArm> scanArm);
  virtual ~ScanCollector() { };
};
