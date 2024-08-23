/*
 * Gui/Saveable.hh
 */

#pragma once

#include "Core/BackgroundTaskExecutor.hh"
#include <QtWidgets/QMdiSubWindow>

struct Saveable
{
  virtual void save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) = 0;
  virtual void saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) = 0;
};