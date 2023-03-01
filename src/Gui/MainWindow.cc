/*
 * Gui/MainWindow.cc
 */

#include <memory>
#include <db_cxx.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMessageBox>
#include <UCL/Reflection/Binding/ElementNameTransformer.hh>
#include "Core/PersistentVariable.hh"
#include "Core/CommonXmlLoad.hh"
#include "Core/CommonXmlSave.hh"
#include "Core/ScanDataReflection.hh"
#include "Core/ScriptSettingsReflection.hh"

#include "Core/AssembleScanTask.hh"
#include "Core/ConfigurationLocator.hh"
#include "Core/Devices.hh"
#include "Core/LoadScanTask.hh"
#include "Core/ScanIO.hh"
#include "Core/QtScript/AudioDataCollector.hh"
#include "Core/QtScript/Coil.hh"
#include "Core/QtScript/Functions.hh"
#include "Gui/AssembleScanDialog.hh"
#include "Gui/AssignColorForColorBarDialog.hh"
#include "Gui/CoilManualControl.hh"
#include "Gui/EditorWindow.hh"
#include "Gui/MainWindow.hh"
#include "Gui/ManualControlDialog.hh"
#include "Gui/ProcessingParametersDialog.hh"
#include "Gui/ScanControlDialog.hh"
#include "Gui/ScanDisplayWindow.hh"
#include "Gui/AutoScanWindow.hh"
#include "RealTime/RTAudioCollector.h"
#include "RealTime/RTHead.h"

MainWindow::MainWindow(realtime::RTContext &rtCtxt, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory)
  : taskExecutor(taskExecutor),
  scanFactory(scanFactory),
    m_rtCtxt(rtCtxt),
  processingParameters(Configuration::getConfigurationPathname("etc/Processing-Parameters.xml").toStdString(), "Processing-Parameters"),
  scripts(Configuration::getConfigurationPathname("etc/Scripts.xml").toStdString(), "Scripts")
{
  ui.setupUi(this);
  ui.backgroundTasksBox->hide();

  updateTimer = new QTimer(this);
  updateTimer->start(1000);
  
  prepareScriptEnvironment();
  connectSignals();
  loadConfiguration();
}

MainWindow::~MainWindow()
{
  delete scriptExecutor;
}

void MainWindow::prepareScriptEnvironment()
{
  scriptEngine = new QScriptEngine(this);
  scriptProgressReporter = new script::ProgressReporter(this);
  auto audioDataCollector = new script::AudioDataCollector(devices::audioDataCollector, scriptEngine);

  scriptEngine->globalObject().setProperty("testLabel",
    scriptEngine->newQObject(new script::TestLabel(ui.label_2,scriptEngine),QScriptEngine::ScriptOwnership));

  scriptEngine->globalObject().setProperty("audioDataCollector", 
                                            scriptEngine->newQObject(audioDataCollector, QScriptEngine::ScriptOwnership));
  //scriptEngine->globalObject().setProperty("builtin_coil", 
                                           //scriptEngine->newQObject(new script::Coil(devices::coile, scriptEngine),
                                                                    //QScriptEngine::ScriptOwnership));
  scriptEngine->globalObject().setProperty("progressReporter", scriptEngine->newQObject(scriptProgressReporter));
  scriptEngine->globalObject().setProperty("sleep",
                                           scriptEngine->newFunction(script::sleep));

  scriptExecutorThread = new QThread;
  scriptExecutorThread->start();

  scriptExecutor = new ScriptExecutor(scriptEngine);
  scriptExecutor->moveToThread(scriptExecutorThread);

  connect(audioDataCollector, SIGNAL(lineFechted(const SourceScanLine&)), &scanFactory, SLOT(addRangeScanLine(const SourceScanLine&)), Qt::QueuedConnection);
}

void MainWindow::connectSignals()
{
  connect(ui.runScriptAction, SIGNAL(triggered()), this, SLOT(runScript()));
  connect(ui.stopAction, SIGNAL(triggered()), this, SLOT(stop()));
  connect(ui.initializeAction, SIGNAL(triggered()), this, SLOT(initialize()));
  connect(ui.autoScanAction, SIGNAL(triggered()), this, SLOT(runAutoScan()));
  connect(ui.showManualControlDialogAction, SIGNAL(triggered()), this, SLOT(showManualControlDialog()));
  connect(ui.quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
  connect(ui.assembleScanAction, SIGNAL(triggered()), this, SLOT(showAssembleScanDialog()));
  connect(ui.exportWaveAction, SIGNAL(triggered()), this, SLOT(exportWave()));
  connect(ui.currentProcessingParametersAction, SIGNAL(triggered()), this, SLOT(showCurrentParameterDialog()));
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateCoordinates()));

  connect(ui.newAction, SIGNAL(triggered()), this, SLOT(newScript()));
  connect(ui.openAction, SIGNAL(triggered()), this, SLOT(open()));
  connect(ui.openTechnologicalAction, SIGNAL(triggered()), this, SLOT(openTechnological()));
  connect(ui.saveAction, SIGNAL(triggered()), this, SLOT(save()));
  connect(ui.saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
  connect(ui.technologicalParametersAction, SIGNAL(triggered()), this, SLOT(showTechnologicalParametersDialog()));
  connect(ui.saveTechnologicalParametersAction, SIGNAL(triggered()), this, SLOT(saveTechnologicalParameters()));
  connect(ui.currentParametersToTechnologicalAction, SIGNAL(triggered()), this, SLOT(currentParametersToTechnological()));

  connect(ui.coilManualControlAction, SIGNAL(triggered()), this, SLOT(coilManualControl()));

  connect(ui.undoAction, SIGNAL(triggered()), this, SLOT(editUndo()));
  connect(ui.redoAction, SIGNAL(triggered()), this, SLOT(editRedo()));
  connect(ui.cutAction, SIGNAL(triggered()), this, SLOT(editCut()));
  connect(ui.copyAction, SIGNAL(triggered()), this, SLOT(editCopy()));
  connect(ui.pasteAction, SIGNAL(triggered()), this, SLOT(editPaste()));

  connect(ui.setZeroCoodinateButton, SIGNAL(clicked()), this, SLOT(setTechnologicalZero()));

  connect(&taskExecutor, SIGNAL(started(const QString&, int)), this, SLOT(taskStarted(const QString&, int)), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(stageStarted(const QString&, int)), this, SLOT(taskStageStarted(const QString&, int)), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(stageProgressed()), this, SLOT(taskStageProgressed()), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(finished()), this, SLOT(taskFinished()), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(terminated(const QString&)), this, SLOT(taskTerminated(const QString&)), Qt::QueuedConnection);

  //connect(scriptExecutor, SIGNAL(scriptStarted()), &scanFactory, SLOT(startNewScan()), Qt::QueuedConnection);

  connect(scriptExecutor, SIGNAL(errorMessage(const QString&)), this, SLOT(showScriptErrorMessage(const QString&)), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(lineChanged(int)), this, SLOT(highlightScriptLine(int)), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(scriptFinished()), this, SLOT(unhighlightScriptLine()), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(scriptFinished()), &scanFactory, SLOT(finishScan()), Qt::QueuedConnection);
  connect(this, SIGNAL(scriptStarted(const QString&, const QString&, bool)), scriptExecutor, SLOT(runScript(const QString&, const QString&, bool)), Qt::QueuedConnection);

  connect(&scanFactory, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), this, SLOT(showScan(const std::shared_ptr<Scan>&)));
}

void MainWindow::loadConfiguration()
{
  namespace urb = uts::reflection::binding;

  try {
    processingParameters.load();
  } catch (...) {
    QMessageBox::critical(this, "Сбой загрузки настроек", QString::fromUtf8(boost::current_exception_diagnostic_information().c_str()));
  }
  try{
    scripts.load();
  } catch (...) {
    QMessageBox::critical(this, "Сбой загрузки настроек скриптов", QString::fromUtf8(boost::current_exception_diagnostic_information().c_str()));
  }
}

QWidget* MainWindow::getCurrentMdiWidget()
{
  auto win = ui.mdiArea->currentSubWindow();
  return win ? win->widget() : 0;
}

void MainWindow::showScriptErrorMessage(const QString& msg)
{
  QMessageBox::critical(this, "Ошибка в программе", msg);
}

void MainWindow::highlightScriptLine(int lineNumber)
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->highlightLine(lineNumber);
  }
}

void MainWindow::unhighlightScriptLine()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->unhighlightLine();
  }
}

void MainWindow::updateCoordinates()
{
  try {
    /*if (devices::xAxisMotor) {
      ui.xLabel->setText(QString::number(devices::xAxisMotor->getMachineCoordinate()));
      ui.x0Label->setText(QString::number(devices::xAxisMotor->getTechnologicalCoordinate()));
    }

    if (devices::yAxisMotor) {
      ui.yLabel->setText(QString::number(devices::yAxisMotor->getMachineCoordinate()));
      ui.y0Label->setText(QString::number(devices::yAxisMotor->getTechnologicalCoordinate()));
    }*/
  } catch (...) {
  }
}

void MainWindow::setTechnologicalZero()
{
  //devices::xAxisMotor->setTechnologicalZero();
  //devices::yAxisMotor->setTechnologicalZero();
}

void MainWindow::taskStarted(const QString& name, int stageCount)
{
  ui.backgroundTasksBox->show();
  ui.taskLabel->setText(name);
  ui.taskProgressBar->setMaximum(stageCount);
  ui.taskProgressBar->setValue(0);
  taskProgressed = false;
}

void MainWindow::taskStageStarted(const QString& name, int maximumValue)
{
  ui.stageLabel->setText(name);
  ui.stageProgressBar->setMaximum(maximumValue);
  ui.stageProgressBar->setValue(0);
  if (taskProgressed)
    ui.taskProgressBar->setValue(ui.taskProgressBar->value() + 1);
  taskProgressed = true;
}

void MainWindow::taskStageProgressed()
{
  ui.stageProgressBar->setValue(ui.stageProgressBar->value() + 1);
}

void MainWindow::taskFinished()
{
  ui.backgroundTasksBox->hide();
}

void MainWindow::taskTerminated(const QString& errorMessage)
{
  ui.backgroundTasksBox->hide();
  QMessageBox::critical(this, "Ошибка", errorMessage);
}

void MainWindow::newScript()
{
  auto ew = new EditorWindow;
  ew->setAttribute(Qt::WA_DeleteOnClose, true);
  ui.mdiArea->addSubWindow(ew);
  ew->showMaximized();
}

void MainWindow::open()
{
  auto pathnames = QFileDialog::getOpenFileNames(this, "Открыть", lastOpenDir, "Все файлы сканера (*.js *.csp)");
  if (pathnames.isEmpty()) return;

  bool newPartCreated = false;
  for (auto& pathname : pathnames) {
    QString normalizedSuffix = QFileInfo(pathname).suffix().toLower();
    if (normalizedSuffix == "js") {
      auto ew = new EditorWindow(pathname, this);
      ew->setAttribute(Qt::WA_DeleteOnClose);
      ui.mdiArea->addSubWindow(ew);
      ew->showMaximized();
    } else if (normalizedSuffix == "csp") {
      try {
        taskExecutor.enqueue(new LoadScanTask(pathname, scanFactory, *processingParameters));
      } catch (DbException& exc) {
	      QMessageBox::critical(this, "Ошибка", exc.what());
      } catch (uts::Exception& exc) {
        auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
        if (msg) {
          QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
        } else {
          QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
        }
      }
    }
  }

  lastOpenDir = QFileInfo(pathnames.back()).dir().path();
}

void MainWindow::openTechnological()
{
  auto pathnames = QFileDialog::getOpenFileNames(this, "Открыть", lastOpenDir, "Сканированные детали (*.csp)");
  if (pathnames.isEmpty()) return;

  bool newPartCreated = false;
  for (auto& pathname : pathnames) {
    QString normalizedSuffix = QFileInfo(pathname).suffix().toLower();
    try {
      taskExecutor.enqueue(new LoadScanTask(pathname, scanFactory, *processingParameters, true));
    } catch (DbException& exc) {
      QMessageBox::critical(this, "Ошибка", exc.what());
    }
  }

  lastOpenDir = QFileInfo(pathnames.back()).dir().path();
}

void MainWindow::save()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Saveable*>(currentWidget)) {
    ew->save(taskExecutor);
  }
}

void MainWindow::saveAs()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Saveable*>(currentWidget)) {
    ew->saveAs(taskExecutor);
  }
}

void MainWindow::editUndo()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->undo();
  }
}

void MainWindow::editRedo()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->redo();
  }
}

void MainWindow::editCut()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->cut();
  }
}

void MainWindow::editCopy()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->copy();
  }
}

void MainWindow::editPaste()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    ew->paste();
  }
}

void MainWindow::runScript()
{
    try {
        /*realtime::RTAudioCollector* col = dynamic_cast<realtime::RTAudioCollector*>(m_rtCtxt.getRTDevice("AudioDataCollector").get());
        if (col)
            col->start(1000);*/
        realtime::RTHead* head = dynamic_cast<realtime::RTHead*>(m_rtCtxt.getRTDevice("APLHead").get());
        if (head)
            head->start(1000);
    }
    catch (...) {

    }
 
  /*auto currentWidget = getCurrentMdiWidget();
  if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
    QFile common(QString::fromStdString(scripts->common));
    if (common.open(QIODevice::ReadOnly)) {
      emit scriptStarted(QString::fromUtf8(common.readAll().data()), QString::fromStdString(scripts->common), true);
    }

    scanFactory.startNewScan(*processingParameters);
    ScanControlDialog scd;
    connect(scriptExecutor, SIGNAL(scriptFinished()), &scd, SLOT(accept()));
    connect(scriptProgressReporter, SIGNAL(taskStarted(int)), &scd, SLOT(newScanTask(int)), Qt::QueuedConnection);
    connect(scriptProgressReporter, SIGNAL(taskProgressed()), &scd, SLOT(scanTaskProgressed()), Qt::QueuedConnection);
    connect(scriptProgressReporter, SIGNAL(taskFinished()), &scd, SLOT(scanTaskFinished()), Qt::QueuedConnection);

    emit scriptStarted(ew->scriptCode(), "", false);

    if (scd.exec() == QMessageBox::Abort) {
      stop();
      unhighlightScriptLine();
    }
  }*/
}

void MainWindow::runAutoScan()
{
  QFile common(QString::fromStdString(scripts->common));
    if (common.open(QIODevice::ReadOnly)) {
      emit scriptStarted(QString::fromUtf8(common.readAll().data()), QString::fromStdString(scripts->common), true);
    }
    AutoScanWindow asw(QString::fromStdString(scripts->autoScan), 0, 0);//devices::xAxisMotor->getMachineCoordinate(),devices::yAxisMotor->getMachineCoordinate());

  bool b = this->connect(&asw,SIGNAL(setProperty(const QString&,const QScriptValue&)),scriptExecutor,SLOT(setProperty(const QString&,const QScriptValue&)));
  connect(&asw,SIGNAL(startScanScript(const QString&,const QString&,bool)),this,SIGNAL(scriptStarted(const QString&,const QString&,bool)));
  connect(&asw,SIGNAL(stop()),this,SLOT(stop()));
  asw.exec();
}


void MainWindow::showManualControlDialog()
{
  auto mcd = new ManualControlDialog;
  mcd->setAttribute(Qt::WA_DeleteOnClose, true);
  mcd->show();
}

void MainWindow::showAssembleScanDialog()
{
  AssembleScanDialog* asd = new AssembleScanDialog(this);
  asd->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(asd, SIGNAL(accepted()), this, SLOT(enqueueAssembleScanTask()));
  asd->show();
}

void MainWindow::showAssignColorsForColorBarDialog()
{
    AssignColorForColorBarDialog* colorDialog = new AssignColorForColorBarDialog(*processingParameters, this);
    // Прячем кнопки перемещения итемов, т.к. реализация не завершена.
    colorDialog->ui.moveTopButton->setVisible(false);
    colorDialog->ui.moveBottomButton->setVisible(false);
    colorDialog->ui.moveDownButton->setVisible(false);
    colorDialog->ui.moveUpButton->setVisible(false);
    
    colorDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(colorDialog, SIGNAL(accepted()), this, SLOT(assignColorsForColorBar()));
    colorDialog->show();
}

void MainWindow::assignColorsForColorBar()
{
    if (AssignColorForColorBarDialog* colorDialog = dynamic_cast<AssignColorForColorBarDialog*>(sender())) {
        processingParameters = colorDialog->getProcessingParameters();
    }
}

void MainWindow::enqueueAssembleScanTask()
{
  if (AssembleScanDialog* asd = dynamic_cast<AssembleScanDialog*>(sender())) {
    taskExecutor.enqueue(new AssembleScanTask(asd->getFiles(), scanFactory, *processingParameters));
  }
}

void MainWindow::stop()
{
    realtime::RTAudioCollector* col = dynamic_cast<realtime::RTAudioCollector*>(m_rtCtxt.getRTDevice("AudioDataCollector").get());
    if (col)
        col->stop();
    realtime::RTHead* head = dynamic_cast<realtime::RTHead*>(m_rtCtxt.getRTDevice("APLHead").get());
    if (head)
        head->stop();
  //scriptExecutor->stop();
  //devices::audioDataCollector->stop();
  //devices::coile->stop();
}

void MainWindow::initialize()
{
  if (!scripts->initialization.empty()) {
    QFile common(QString::fromStdString(scripts->common));
    if (common.open(QIODevice::ReadOnly)) {
      emit scriptStarted(QString::fromUtf8(common.readAll().data()), QString::fromStdString(scripts->common), true);
    }

    QFile init(QString::fromStdString(scripts->initialization));
    if (! init.open(QIODevice::ReadOnly)) {
      QMessageBox::critical(this, "Ошибка", "Не удалось считать программу инициализации");
      return;
    }

    emit scriptStarted(QString::fromUtf8(init.readAll().data()), QString::fromStdString(scripts->initialization), false);

    QMessageBox stopScriptBox(QMessageBox::NoIcon, "Выполняется инициализация", "Остановить инициализацию", QMessageBox::Abort);
    stopScriptBox.setDefaultButton(QMessageBox::Abort);
    connect(scriptExecutor, SIGNAL(scriptFinished()), &stopScriptBox, SLOT(accept()));
    if (stopScriptBox.exec() == QMessageBox::Abort) {
      stop();
    }
  }
}

void MainWindow::showScan(const std::shared_ptr<Scan>& scan)
{
  auto sdw = new ScanDisplayWindow(scan);
  sdw->setAttribute(Qt::WA_DeleteOnClose, true);
  ui.mdiArea->addSubWindow(sdw);
  sdw->showMaximized();
  connect(sdw,SIGNAL(refreshScan(std::shared_ptr<Scan>&)),&scanFactory,SLOT(recalculateScan(std::shared_ptr<Scan>&)));
}

void MainWindow::closeEvent(QCloseEvent* evt)
{
  foreach(auto sw, ui.mdiArea->subWindowList()) {
    if (! sw->close()) {
      evt->ignore();
      return;
    }
  }

  scriptExecutorThread->quit();
  scriptExecutorThread->wait();
  evt->accept();
}

void MainWindow::showTechnologicalParametersDialog()
{
  ProcessingParametersDialog ppd(*processingParameters, true);
  if (ppd.exec() == QDialog::Accepted) {
    processingParameters = ppd.getProcessingParameters();
  }
}

void MainWindow::saveTechnologicalParameters()
{
  namespace urb = uts::reflection::binding;

  try {
    processingParameters.save();
  }
  catch (...) {
    QMessageBox::critical(this, "Сбой сохранения технологичесих параметров", QString::fromUtf8(boost::current_exception_diagnostic_information().c_str()));
  }
}

void MainWindow::exportWave()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
    auto dirname = QFileDialog::getExistingDirectory(this, "Каталог экспорта");
    if (! dirname.isEmpty()) {
      sdw->exportWave(dirname);
    }
  }  
}

void MainWindow::coilManualControl()
{
  auto cmd = new CoilManualControlDialog;
  cmd->setAttribute(Qt::WA_DeleteOnClose, true);
  cmd->show();
}

void MainWindow::showCurrentParameterDialog()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
    ProcessingParametersDialog ppd(sdw->getProcessingParameters(), false);

    if (ppd.exec() == QDialog::Accepted) {
        sdw->applyParameters(ppd.getProcessingParameters(), scanFactory);
    }
  }
}

void MainWindow::currentParametersToTechnological()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
    if (QMessageBox::question(this, "Подтверждение", "Заменить технологические параметры текущими?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      processingParameters = sdw->getProcessingParameters();
    }
  }
}
