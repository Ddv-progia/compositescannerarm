/*
 * Gui/MainWindow.hh
 */

#pragma once

#include <QtCore/QThread>
#include <QtCore/QTimer>

#include "ui_MainWindow.h"
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanDataMetatypes.hh"
#include "Core/ScanFactory.hh"
#include "Core/PersistentVariable.hh"
#include "RealTime/RTContext.h"



class MainWindow : public QMainWindow
{
  Q_OBJECT
	Ui::MainWindow ui;
  BackgroundTaskExecutor& taskExecutor;
  ScanFactory& scanFactory;
  QTimer* updateTimer;

  PersistentVariable<ProcessingParameters> processingParameters;

  bool taskProgressed;

  QString lastOpenDir;

  realtime::RTContext &m_rtCtxt;
public:
  explicit MainWindow(realtime::RTContext &rtCtxt, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory);
  ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent* evt) override;

private:
  

  void connectSignals();
  void loadConfiguration();

  QWidget* getCurrentMdiWidget();

  Q_SLOT void setTechnologicalZero();

  Q_SLOT void taskStarted(const QString& name, int stageCount);
  Q_SLOT void taskStageStarted(const QString& name, int maximumValue);
  Q_SLOT void taskStageProgressed();
  Q_SLOT void taskFinished();
  Q_SLOT void taskTerminated(const QString& errorMessage);

  Q_SLOT void newScript();
  Q_SLOT void open();
  Q_SLOT void openTechnological();
  Q_SLOT void save();
  Q_SLOT void saveAs();

  Q_SLOT void editUndo();
  Q_SLOT void editRedo();
  Q_SLOT void editCut();
  Q_SLOT void editCopy();
  Q_SLOT void editPaste();

  Q_SLOT void start();
  Q_SLOT void showManualControlDialog();
  Q_SLOT void showAssembleScanDialog();
  Q_SLOT void showAssignColorsForColorBarDialog();
  Q_SLOT void enqueueAssembleScanTask();
  Q_SLOT void assignColorsForColorBar();
  Q_SLOT void stop();
  Q_SLOT void initialize();
  Q_SLOT void showTechnologicalParametersDialog();
  Q_SLOT void saveTechnologicalParameters();

  Q_SLOT void exportWave();
  Q_SLOT void showCurrentParameterDialog();
  Q_SLOT void currentParametersToTechnological();
  Q_SLOT void coilManualControl();

  Q_SLOT void showScan(const std::shared_ptr<Scan>& scan);
};
