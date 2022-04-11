/*
 * Core/SaveScanTask.hh
 */

#pragma once

#include "Core/LineEncoding.hh"
#include "Core/ProgressReportingTask.hh"
#include "Core/ScanData.hh"

class SaveScanTask : public ProgressReportingTask
{
public:
  SaveScanTask(const QString& pathname, const Scan& scan, LineEncoding encoding);
  virtual void operator() () override;

private:
  QString pathname;
  const Scan& scan;
  LineEncoding encoding;
};