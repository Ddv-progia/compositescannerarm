/*
 * Gui/Saveable.hh
 */

#pragma once

#include "Core/BackgroundTaskExecutor.hh"

struct Saveable
{
  virtual void save(BackgroundTaskExecutor& taskExecutor) = 0;
  virtual void saveAs(BackgroundTaskExecutor& taskExecutor) = 0;
};