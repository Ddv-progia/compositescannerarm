/*
 * Core/AssembleScanTask.hh
 */

#pragma once

#include <vector>
#include <QtCore/QString>

#include "Core/ProgressReportingTask.hh"
#include "Core/ScanFactory.hh"

class AssembleScanTask : public ProgressReportingTask
{
  std::vector<QString> sourceFilenames;
  ScanFactory& scanFactory;
  ProcessingParameters const& params;

public:
  AssembleScanTask(const std::vector<QString>& sourceFilenames,
                   ScanFactory& scanFactory,
                   ProcessingParameters const& params);

  virtual void operator()() override;
};