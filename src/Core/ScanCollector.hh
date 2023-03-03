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
	  std::shared_ptr<Scan> m_scan;
public:
  ScanCollector();
  
  Q_SLOT virtual void start() = 0;
  Q_SLOT virtual void pause() = 0;
  Q_SLOT virtual void stop() = 0;
  Q_SLOT virtual void share();

protected:
  Q_SIGNAL void ready(std::shared_ptr<Scan> scan);
  virtual ~ScanCollector() { };
};
