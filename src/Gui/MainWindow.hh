/*
 * Gui/MainWindow.hh
 */

#pragma once

#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QJSEngine>
//#include <QtScript/QScriptEngine>
//#include <QAxScript>
//c:\Projects\vcpkg\installed\x64 - windows\include\QtScript
#include <QListWidget>
#include "ui_MainWindow.h"
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanDataMetatypes.hh"
#include "Core/ScanFactory.hh"
#include "Core/ScriptExecutor.hh"
#include "Core/ScriptSettings.hh"
#include "Core/QtScript/ProgressReporter.hh"
#include "Core/PersistentVariable.hh"
#include "Core/ImageProcessing.hh"
#include "RealTime/RTContext.h"


class RectSelectionDialog : public QDialog {
	Q_OBJECT
public:

	explicit RectSelectionDialog(QWidget* parent = nullptr);
	RectSelectionDialog(QWidget* parent, std::vector< ::RectForNormalization >& vectorOfRectForNormalizationIn);
	void setSpecNormalizationIJRect(std::vector< ::RectForNormalization >& vectorOfRectForNormalizationIn);
	RectForNormalization* getSelectedRect() const;
	std::vector<RectForNormalization>* specNormalizationIJRect;

private slots:
	void accept() override;
	void slotDelete();          // обработчик кнопки Ђудалитьї
	void slotClearAll();        // обработчик кнопки Ђочистить всеї

private:
	QListWidget* listWidget;
	RectForNormalization* selectedRect;
	QPushButton* btnDelete;      // Ђудалитьї Ц удал€ем выбранный элемент
	QPushButton * btnClearAll;    // Ђочистить всеї Ц очищаем весь список
};

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(realtime::RTContext &rtCtxt, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory);
  ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent* evt) override;
	Q_SIGNAL void scriptStarted(const QString& code, const QString& filename, bool intermediate);

private:
	Ui::MainWindow ui;
	QTimer* updateTimer;
	BackgroundTaskExecutor &taskExecutor;
	ScanFactory &scanFactory;
	QJSEngine* scriptEngine;
	ScriptExecutor* scriptExecutor;
	QThread* scriptExecutorThread;
	script::ProgressReporter* scriptProgressReporter;
	std::shared_ptr<std::vector<Defect>> defects;

	realtime::RTContext& m_rtCtxt;

	PersistentVariable<ProcessingParameters> processingParameters;
	PersistentVariable<ScriptSettings> scripts;

	bool taskProgressed;

	QString lastOpenDir;

  void prepareScriptEnvironment();
  void connectSignals();
  void loadConfiguration();

  QWidget* getCurrentMdiWidget();

  Q_SLOT void showScriptErrorMessage(const QString& msg);
  Q_SLOT void highlightScriptLine(int lineNumber);
  Q_SLOT void unhighlightScriptLine();

  Q_SLOT void updateCoordinates();
  Q_SLOT void updateCursorCoordinates();
  Q_SLOT void setTechnologicalZero();

  Q_SLOT void taskStarted(const QString& name, int stageCount);
  Q_SLOT void taskStageStarted(const QString& name, int maximumValue);
  Q_SLOT void taskStageProgressed();
  Q_SLOT void taskFinished();
  Q_SLOT void taskTerminated(const QString& errorMessage);

  Q_SLOT void newScript();
  Q_SLOT void newRtWindow();
  Q_SLOT void open();
  Q_SLOT void openTechnological();
  Q_SLOT void save();
  Q_SLOT void saveAs();
  Q_SLOT void load();

  Q_SLOT void editUndo();
  Q_SLOT void editRedo();
  Q_SLOT void editCut();
  Q_SLOT void editCopy();
  Q_SLOT void editPaste();

  Q_SLOT void startRtCoil();
  Q_SLOT void startRtCursor();
  Q_SLOT void startRt();
  Q_SLOT void runAutoScan();
  Q_SLOT void runScript();
  Q_SLOT void showManualControlDialog();
  Q_SLOT void showAssembleScanDialog();
  Q_SLOT void showAssignColorsForColorBarDialog();
  Q_SLOT void enqueueAssembleScanTask();
  Q_SLOT void assignColorsForColorBar();
  Q_SLOT void stop();
  Q_SLOT void stopRt();
  Q_SLOT void initialize();
  Q_SLOT void showTechnologicalParametersDialog();
  Q_SLOT void saveTechnologicalParameters();
  Q_SLOT void exportWave();
  Q_SLOT void makeBScanAction();
  Q_SLOT void doubleLines();
  Q_SLOT void showCurrentParameterDialog();
  Q_SLOT void currentParametersToTechnological();
  Q_SLOT void coilManualControl();

  Q_SLOT void showScan(const std::shared_ptr<Scan>& scan);
  Q_SLOT void showHideUnusedAction(bool needShow = false);
  
  Q_SLOT void moveAlongDefect(const std::vector<Defect*>& defectsIn, ::DefectSearchingParameters defectSearching);
  Q_SLOT void addZCoordinates();
  Q_SLOT void showSelectNormalizationAreaDialog();

};
