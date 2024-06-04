/*
 * Gui/Loadable.hh
 */

#pragma once

#include "Core/BackgroundTaskExecutor.hh"

struct Loadable
{
  virtual void load(BackgroundTaskExecutor& taskExecutor) = 0;
};