/*
 * Core/InitializeMediaFoundationTask.hh
 */

#pragma once

#include "Core/ProgressReportingTask.hh"

class InitializeMediaFoundationTask : public ProgressReportingTask
{
public:
  virtual void operator()() override;
};
