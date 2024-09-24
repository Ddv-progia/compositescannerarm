/*
 * Gui/Loadable.hh
 */

#pragma once

#include <QtWidgets/QMdiSubWindow>
#include "Core/PersistentVariable.hh"
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanFactory.hh"


struct Loadable
{

  //virtual void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0, ScanFactory& scanFactory, const ProcessingParameters& parameters) = 0;
  void loadLastOpenDir();
  void saveLastOpenDir();
  virtual void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) = 0;
  QString lastOpenDir = "";
  ScanFactory* scanFactory;
  //PersistentVariable<ProcessingParameters> processingParameters;
  ProcessingParameters processingParameters;
};