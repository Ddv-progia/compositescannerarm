/*
 * Core/ScanCollector.hh
 */

#pragma once

#include <QtWidgets/QWidget>
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"


class ScanCollector : public QWidget
{
  Q_OBJECT
protected:
	  std::shared_ptr<ScanArm> m_scan;
public:
  ScanCollector();
  std::shared_ptr <SourceScanChunks> sourceScanChunks;
  
  Q_SLOT virtual void start() = 0;
  Q_SLOT virtual void pause() = 0;
  Q_SLOT virtual void stop() = 0;
  Q_SLOT virtual void share();
  std::shared_ptr<Scan> ScanArmToScan(std::shared_ptr<ScanArm> scanArm);

protected:
  Q_SIGNAL void ready(std::shared_ptr<ScanArm> scan);
  virtual ~ScanCollector() { };
};
