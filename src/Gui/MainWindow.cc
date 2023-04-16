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

#include "Core/AssembleScanTask.hh"
#include "Core/ConfigurationLocator.hh"
#include "Core/Devices.hh"
#include "Core/LoadScanTask.hh"
#include "Core/ScanIO.hh"
#include "Gui/AssembleScanDialog.hh"
#include "Gui/AssignColorForColorBarDialog.hh"
#include "Gui/CoilManualControl.hh"
#include "Gui/EditorWindow.hh"
#include "Gui/MainWindow.hh"
#include "Gui/ManualControlDialog.hh"
#include "Gui/ProcessingParametersDialog.hh"
#include "Gui/ScanControlDialog.hh"
#include "Gui/ScanDisplayWindow.hh"
#include "RealTime/RTScanCollector.h"
#include "Core/ScanCollector.hh"

MainWindow::MainWindow(realtime::RTContext &rtCtxt, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory)
  : taskExecutor(taskExecutor), scanFactory(scanFactory), m_rtCtxt(rtCtxt),
  processingParameters(Configuration::getConfigurationPathname("etc/Processing-Parameters.xml").toStdString(), "Processing-Parameters")
{
  ui.setupUi(this);
  
  connectSignals();
  loadConfiguration();

  ui.centralwidget->layout()->setContentsMargins(0, 0, 0, 0);
  newScript();
  updateTimer = new QTimer(this);
  updateTimer->start(1000);

}

MainWindow::~MainWindow()
{}

void MainWindow::connectSignals()
{
  connect(ui.runScriptAction, SIGNAL(triggered()), this, SLOT(start()));
  connect(ui.stopAction, SIGNAL(triggered()), this, SLOT(stop()));
  connect(ui.initializeAction, SIGNAL(triggered()), this, SLOT(initialize()));
  connect(ui.showManualControlDialogAction, SIGNAL(triggered()), this, SLOT(showManualControlDialog()));
  connect(ui.quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
  connect(ui.assembleScanAction, SIGNAL(triggered()), this, SLOT(showAssembleScanDialog()));
  connect(ui.exportWaveAction, SIGNAL(triggered()), this, SLOT(exportWave()));
  connect(ui.currentProcessingParametersAction, SIGNAL(triggered()), this, SLOT(showCurrentParameterDialog()));

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

  //connect(ui.setZeroCoodinateButton, SIGNAL(clicked()), this, SLOT(setTechnologicalZero()));

  connect(&taskExecutor, SIGNAL(started(const QString&, int)), this, SLOT(taskStarted(const QString&, int)), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(stageStarted(const QString&, int)), this, SLOT(taskStageStarted(const QString&, int)), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(stageProgressed()), this, SLOT(taskStageProgressed()), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(finished()), this, SLOT(taskFinished()), Qt::QueuedConnection);
  connect(&taskExecutor, SIGNAL(terminated(const QString&)), this, SLOT(taskTerminated(const QString&)), Qt::QueuedConnection);

  //connect(scriptExecutor, SIGNAL(scriptFinished()), &scanFactory, SLOT(finishScan()), Qt::QueuedConnection);

  connect(&scanFactory, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), this, SLOT(showScan(const std::shared_ptr<Scan>&)));
}

void MainWindow::loadConfiguration()
{
  namespace urb = uts::reflection::binding;

  try {
  
    //processingParameters.load();
  } catch (...) {
    QMessageBox::critical(this, "Сбой загрузки настроек", QString::fromUtf8(boost::current_exception_diagnostic_information().c_str()));
  }
}

QWidget* MainWindow::getCurrentMdiWidget()
{
  auto win = ui.mdiArea->currentSubWindow();
  return win ? win->widget() : 0;
}

void MainWindow::setTechnologicalZero()
{
  //devices::xAxisMotor->setTechnologicalZero();
  //devices::yAxisMotor->setTechnologicalZero();
}

void MainWindow::taskStarted(const QString& name, int stageCount)
{
  //ui.backgroundTasksBox->show();
  //ui.taskLabel->setText(name);
  //ui.taskProgressBar->setMaximum(stageCount);
  //ui.taskProgressBar->setValue(0);
  taskProgressed = false;
}

void MainWindow::taskStageStarted(const QString& name, int maximumValue)
{
  //ui.stageLabel->setText(name);
  //ui.stageProgressBar->setMaximum(maximumValue);
  //ui.stageProgressBar->setValue(0);
  //if (taskProgressed)
    //ui.taskProgressBar->setValue(ui.taskProgressBar->value() + 1);
  //taskProgressed = true;
}

void MainWindow::taskStageProgressed()
{
  //ui.stageProgressBar->setValue(ui.stageProgressBar->value() + 1);
}

void MainWindow::taskFinished()
{
  //ui.backgroundTasksBox->hide();
}

void MainWindow::taskTerminated(const QString& errorMessage)
{
  //ui.backgroundTasksBox->hide();
  QMessageBox::critical(this, "Ошибка", errorMessage);
}

void MainWindow::newScript()
{
    auto ew = new realtime::RTScanCollector{ m_rtCtxt, *processingParameters };
    ew->setAttribute(Qt::WA_DeleteOnClose, true);
    ui.mdiArea->addSubWindow(ew);
    ew->showMaximized(); 
  /*auto ew = new EditorWindow;
  ew->setAttribute(Qt::WA_DeleteOnClose, true);
  ui.mdiArea->addSubWindow(ew);
  ew->showMaximized();*/
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
  /*auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Saveable*>(currentWidget)) {
    ew->save(taskExecutor);
  }*/
}

void MainWindow::saveAs()
{
  /*auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Saveable*>(currentWidget)) {
    ew->saveAs(taskExecutor);
  }*/
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

void MainWindow::start()
{
    try {
        auto collector = dynamic_cast<ScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (collector)
            collector->start();
    }
    catch (...) {

    }
}

void MainWindow::stop()
{
    try {
        auto collector = dynamic_cast<ScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (collector)
            collector->stop();
    }
    catch (...) {

    }
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



void MainWindow::initialize()
{
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
    //processingParameters.save();
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
