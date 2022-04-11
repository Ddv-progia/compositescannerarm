/*
 * Core/LoadScanTask.hh
 */

#pragma once

#include <QtCore/QString>

#include "Core/ProgressReportingTask.hh"
#include "Core/ScanData.hh"
#include "Core/ScanFactory.hh"

class LoadScanTask : public ProgressReportingTask
{
public:
  LoadScanTask(const QString& pathname, ScanFactory& scanFactory, const ProcessingParameters& parameters, bool ignoreSavedParameters = false);
  virtual void operator()() override;

private:
  QString pathname;
  ScanFactory& scanFactory;
  ProcessingParameters parameters;
  bool ignoreSavedParameters;

  void loadLine(Db& db, std::uint32_t i);
};