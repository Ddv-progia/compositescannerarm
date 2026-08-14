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
#include "Core/ScanCollector.hh"
#include "Core/QtScript/AudioDataCollector.hh"
#include "Core/QtScript/Coil.hh"
#include "Core/QtScript/Functions.hh"
#include "Core/QtScript/StepMotor.hh"
#include "Core/ScanCollector.hh"
#include "RealTime/RTScanCollector.h"
#include "RealTime/cursor.h"

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



RectSelectionDialog::RectSelectionDialog(QWidget* parent, std::vector< ::RectForNormalization >& vectorOfRectForNormalizationIn) : QDialog(parent)  , specNormalizationIJRect(&vectorOfRectForNormalizationIn)
{
    setWindowTitle(tr("Выбор области"));
    resize(400, 300);
    listWidget = new QListWidget(this);
    for (const auto& rect : *specNormalizationIJRect) {
        QString itemText = QString("(%1, %2, %3) to (%4, %5, %6)")
            .arg(rect.xLowLeft)
            .arg(rect.yLowLeft)
            .arg(rect.zLowLeft)
            .arg(rect.xTopRight)
            .arg(rect.yTopRight)
            .arg(rect.zTopRight);
        listWidget->addItem(itemText);
    }
    QWidget * rightPanel = new QWidget(this);
    QVBoxLayout * rightLayout = new QVBoxLayout(rightPanel);
    btnDelete = new QPushButton(tr("Удалить"), this);
    btnClearAll = new QPushButton(tr("Очистить все"), this);
    rightLayout->addWidget(btnDelete);
    rightLayout->addWidget(btnClearAll);
    QWidget* lowerPanel = new QWidget(this);
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QHBoxLayout* lowerLayout = new QHBoxLayout();
    lowerLayout->addStretch();
    lowerLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(listWidget,&QListWidget::itemDoubleClicked, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(btnDelete, &QPushButton::clicked, this, &RectSelectionDialog::slotDelete);
    connect(btnClearAll, &QPushButton::clicked, this, &RectSelectionDialog::slotClearAll);
    auto* upperLayout = new QHBoxLayout();
    upperLayout->addWidget(listWidget);
    upperLayout->addWidget(rightPanel);
    auto* layout = new QVBoxLayout(this);
    layout->addLayout(upperLayout);
    layout->addLayout(lowerLayout);
}

 void RectSelectionDialog::slotDelete() {
     // Проверяем, выбран ли элемент
     int curRow = listWidget->currentRow();
     if (curRow < 0) {
         QMessageBox::warning(this, tr("Ошибка"),
                              tr("Выберите элемент для удаления."));
         return;
     }

     if (!specNormalizationIJRect->empty()) {
         specNormalizationIJRect->erase(specNormalizationIJRect->begin() + curRow);
     }

     // Удаляем строку из списка виджетов
     QListWidgetItem *item = listWidget->takeItem(curRow);
     delete item;   // (Qt сам освободит память, но делаем явно)
 }

 void RectSelectionDialog::slotClearAll() {
     specNormalizationIJRect->clear();
     listWidget->clear();
 }

void RectSelectionDialog::setSpecNormalizationIJRect(std::vector< ::RectForNormalization >& vectorOfRectForNormalizationIn)
{
    specNormalizationIJRect = &vectorOfRectForNormalizationIn;
}

RectSelectionDialog::RectSelectionDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Выбор области"));
    resize(400, 300);
    listWidget = new QListWidget(this);
    // Populate the list with the rectangles from the global vector
    for (const auto& rect : *specNormalizationIJRect) {
        QString itemText = QString("(%1, %2, %3) to (%4, %5, %6)")
            .arg(rect.xLowLeft)
            .arg(rect.yLowLeft)
            .arg(rect.zLowLeft)
            .arg(rect.xTopRight)
            .arg(rect.yTopRight)
            .arg(rect.zTopRight);
        listWidget->addItem(itemText);
    }

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    layout->addWidget(buttonBox);
}

RectForNormalization* RectSelectionDialog::getSelectedRect() const
{
    return selectedRect;
}

void RectSelectionDialog::accept()
{
    int currentRow = listWidget->currentRow();
    if (currentRow >= 0 && currentRow < listWidget->count()) {
        selectedRect = &specNormalizationIJRect->at(currentRow);
    }
    else {
        // If nothing is selected, we clear the selectedRect to default
        selectedRect = new RectForNormalization();
    }
    QDialog::accept();
}

MainWindow::MainWindow(realtime::RTContext &rtCtxt, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory)
  : taskExecutor(taskExecutor), scanFactory(scanFactory), m_rtCtxt(rtCtxt),
  processingParameters(Configuration::getConfigurationPathname("etc\\Processing-Parameters.xml").toStdString(), "Processing-Parameters"),
  scripts(Configuration::getConfigurationPathname("etc\\Scripts.xml").toStdString(), "Scripts")
{
  ui.setupUi(this);
  ui.backgroundTasksBox->hide();
  showHideUnusedAction(true);
  updateTimer = new QTimer(this);

  prepareScriptEnvironment();
  connectSignals();
  loadConfiguration();

  //ui.centralwidget->layout()->setContentsMargins(2, 2, 2, 2);
  newScript();
  newRtWindow();
  updateTimer->start(1000); // Запускать, если катушка не 2022

}

MainWindow::~MainWindow()
{
    delete scriptExecutor;
}

template <typename T> void addType(QJSEngine* engine) {
    //auto constructor = engine->newFunction([](QScriptContext*, QScriptEngine* engine) {
    //    return engine->newQObject(new T());
    //    });
    //auto value = engine->newQMetaObject(&T::staticMetaObject, constructor);
    auto value = engine->newQMetaObject(&T::staticMetaObject);
    engine->globalObject().setProperty(T::staticMetaObject.className(), value);
}

void MainWindow::prepareScriptEnvironment()
{
    scriptEngine = new QJSEngine(this); 
    scriptEngine->installExtensions(QJSEngine::ConsoleExtension);
   
    scriptProgressReporter = new script::ProgressReporter(this);
    auto audioDataCollector = new script::AudioDataCollector(devices::audioDataCollector, scriptEngine);
    //audioDataCollector->start(96000, 1);
    //for (int i = 0; i < 300000000; i++) {};
    //audioDataCollector->stop(10,30,30);

    scriptEngine->globalObject().setProperty("testLabel",
        //scriptEngine->newQObject(new script::TestLabel(ui.label_2, scriptEngine), QJSEngine::ScriptOwnership));
        scriptEngine->newQObject(new script::TestLabel(ui.label_2, scriptEngine)));

    script::StepMotor* xAxisScriptMotor = new script::StepMotor(devices::xAxisMotor, scriptEngine);
    script::StepMotor* yAxisScriptMotor = new script::StepMotor(devices::yAxisMotor, scriptEngine);
    xAxisScriptMotor->addMotor(devices::yAxisMotor); 
    yAxisScriptMotor->addMotor(devices::xAxisMotor);
    scriptEngine->globalObject().setProperty("audioDataCollector",
        scriptEngine->newQObject(audioDataCollector));
    scriptEngine->globalObject().setProperty("builtin_xAxisMotor",
        //scriptEngine->newQObject(new script::StepMotor(devices::xAxisMotor, scriptEngine)));
        scriptEngine->newQObject(xAxisScriptMotor));
        scriptEngine->globalObject().setProperty("builtin_yAxisMotor",
        scriptEngine->newQObject(yAxisScriptMotor));
    scriptEngine->globalObject().setProperty("builtin_coil",
        scriptEngine->newQObject(new script::Coil(devices::coil, scriptEngine)));
    scriptEngine->globalObject().setProperty("progressReporter", scriptEngine->newQObject(scriptProgressReporter));
    //auto xyMotor = new script::StepMotor(devices::xAxisMotor, scriptEngine);
    //xyMotor->addMotor(devices::yAxisMotor);
    //scriptEngine->globalObject().setProperty("builtin_xyMotor",
    //    scriptEngine->newQObject(xyMotor));

    //scriptEngine->globalObject().setProperty("sleep", scriptEngine->newFunction(script::sleep));
    //scriptEngine->globalObject().setProperty("pause", scriptEngine->newFunction(script::pause));
    QJSValue funcObj = scriptEngine->newQObject(new script::FunctionalObject());
    scriptEngine->globalObject().setProperty("sleep", funcObj.property("sleep"));
    scriptEngine->globalObject().setProperty("pause", funcObj.property("pause"));
    scriptEngine->globalObject().setProperty("alert", funcObj.property("alert"));



    scriptExecutorThread = new QThread;
    scriptExecutorThread->start();

    //addType<QTimer>(scriptEngine); //new2024
    auto qTimer = new QTimer();
    qTimer->moveToThread(scriptExecutorThread);

    scriptEngine->globalObject().setProperty("qTimer", scriptEngine->newQObject(qTimer));
    scriptEngine->globalObject().setProperty("line", 0);

    scriptExecutor = new ScriptExecutor(scriptEngine);

    scriptExecutor->moveToThread(scriptExecutorThread);

    connect(audioDataCollector, SIGNAL(lineFechted(const SourceScanLine&)), &scanFactory, SLOT(addRangeScanLine(const SourceScanLine&)), Qt::QueuedConnection);
}

void MainWindow::connectSignals()
{
  connect(ui.runScriptAction, SIGNAL(triggered()), this, SLOT(runScript()));
  connect(ui.runHahdleScanAction, SIGNAL(triggered()), this, SLOT(startRt()));
  connect(ui.runCursorAction, SIGNAL(triggered()), this, SLOT(startRtCursor()));
  connect(ui.stopAction, SIGNAL(triggered()), this, SLOT(stop()));
  connect(ui.stopHandleScanAction, SIGNAL(triggered()), this, SLOT(stopRt()));
  connect(ui.initializeAction, SIGNAL(triggered()), this, SLOT(initialize()));
  connect(ui.autoScanAction, SIGNAL(triggered()), this, SLOT(runAutoScan()));
  connect(ui.showManualControlDialogAction, SIGNAL(triggered()), this, SLOT(showManualControlDialog()));
  connect(ui.quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
  connect(ui.assembleScanAction, SIGNAL(triggered()), this, SLOT(showAssembleScanDialog()));
  connect(ui.exportWaveAction, SIGNAL(triggered()), this, SLOT(exportWave()));
  connect(ui.makeBScanAction, SIGNAL(triggered()), this, SLOT(makeBScanAction()));
  connect(ui.addZcoordinatesAction, SIGNAL(triggered()), this, SLOT(addZCoordinates()));

  connect(ui.currentProcessingParametersAction, SIGNAL(triggered()), this, SLOT(showCurrentParameterDialog()));
  connect(ui.doubleLinesAction, SIGNAL(triggered()), this, SLOT(doubleLines()));
  connect(ui.pbSelectNormalizationAreaDialog, SIGNAL(clicked()), this, SLOT(showSelectNormalizationAreaDialog()));
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateCoordinates()));
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateCursorCoordinates()));

  connect(ui.newAction, SIGNAL(triggered()), this, SLOT(newScript()));
  connect(ui.newRtAction, SIGNAL(triggered()), this, SLOT(newRtWindow()));
  //connect(ui.openAction, SIGNAL(triggered()), this, SLOT(open()));
  connect(ui.openAction, SIGNAL(triggered()), this, SLOT(load()));
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
  //*******
  connect(scriptExecutor, SIGNAL(scriptStarted()), &scanFactory, SLOT(startNewScan()), Qt::QueuedConnection);
  //*******
  connect(scriptExecutor, SIGNAL(errorMessage(const QString&)), this, SLOT(showScriptErrorMessage(const QString&)), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(lineChanged(int)), this, SLOT(highlightScriptLine(int)), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(scriptFinished()), this, SLOT(unhighlightScriptLine()), Qt::QueuedConnection);
  connect(scriptExecutor, SIGNAL(scriptFinished()), &scanFactory, SLOT(finishScan()), Qt::QueuedConnection);
  connect(this, SIGNAL(scriptStarted(const QString&, const QString&, bool)), scriptExecutor, SLOT(runScript(const QString&, const QString&, bool)), Qt::QueuedConnection);

  connect(&scanFactory, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), this, SLOT(showScan(const std::shared_ptr<Scan>&)));

  //событие для проезда по контуру аномалии:
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
  }  catch (...) {
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

void MainWindow::updateCursorCoordinates()
{
    if ((ui.mdiArea->currentSubWindow() == nullptr) ||(ui.mdiArea->currentSubWindow()->widget()==nullptr)){
        return;
      }
    try {
        auto collector = dynamic_cast<realtime::RTScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (collector) {
            //auto xy = collector->m_field->m_cursor->getPositionXY();
 
            ui.xLabel->setText(QString::number(collector->m_field->getPositionX()));
            ui.x0Label->setText(QString::number(0.0));
            ui.yLabel->setText(QString::number(collector->m_field->getPositionY()));
            ui.y0Label->setText(QString::number(0.0));
        }

    } catch (...) {
    }
}

void MainWindow::updateCoordinates()
{
    try {
        if (devices::xAxisMotor) {
            ui.xLabel->setText(QString::number(devices::xAxisMotor->getMachineCoordinate()));
            ui.x0Label->setText(QString::number(devices::xAxisMotor->getTechnologicalCoordinate()));
        }

        if (devices::yAxisMotor) {
            ui.yLabel->setText(QString::number(devices::yAxisMotor->getMachineCoordinate()));
            ui.y0Label->setText(QString::number(devices::yAxisMotor->getTechnologicalCoordinate()));
        }
    } catch (...) {
    }
}

void MainWindow::setTechnologicalZero()
{
    if (devices::xAxisMotor)
    {
        devices::xAxisMotor->setTechnologicalZero();
    }
    else {
        QMessageBox::critical(this, "Ошибка", "xAxisMotor отсутствует");
    }
    if (devices::yAxisMotor)
    {
        devices::yAxisMotor->setTechnologicalZero();
    }
    else {
        QMessageBox::critical(this, "Ошибка", "yAxisMotor отсутствует");
    }

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

void MainWindow::newRtWindow()
{
    auto ew = new realtime::RTScanCollector{ m_rtCtxt, *processingParameters, scanFactory};
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
    ew->save(taskExecutor, ui.mdiArea);
  }
}

void MainWindow::saveAs()
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Saveable*>(currentWidget)) {
    ew->saveAs(taskExecutor,ui.mdiArea);
  }
}

void MainWindow::load()
{
    //QString  settingsFile = "SettingsForAutoScanWindow.ini";
    //QSettings* settings = new QSettings(settingsFile, QSettings::IniFormat);
    //if (lastOpenDir == "") {
    //    lastOpenDir = settings->value(QString::fromUtf8("lastOpenDir"), "").toString();
    //}

  auto currentWidget = getCurrentMdiWidget();
  if (auto ew = dynamic_cast<Loadable*>(currentWidget)) {
      //ew->lastOpenDir = lastOpenDir;
      ew->scanFactory = &scanFactory;
      ew->processingParameters = *processingParameters;
      ew->load(taskExecutor, ui.mdiArea);
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
    auto currentWidget = getCurrentMdiWidget();
    if (auto ew = qobject_cast<EditorWindow*>(currentWidget)) {
        QFile common(QString::fromStdString(scripts->common));
        if (common.open(QIODevice::ReadOnly)) {
            auto commonReadData = common.readAll().data();
            auto fromstdstr = QString::fromStdString(scripts->common);
            emit scriptStarted(QString::fromUtf8(commonReadData), fromstdstr, true);
        }

        scanFactory.startNewScan(*processingParameters);
        ScanControlDialog scd;
        connect(scriptExecutor, SIGNAL(scriptFinished()), &scd, SLOT(accept()));
        connect(scriptProgressReporter, SIGNAL(taskStarted(int)), &scd, SLOT(newScanTask(int)), Qt::QueuedConnection);
        connect(scriptProgressReporter, SIGNAL(taskProgressed()), &scd, SLOT(scanTaskProgressed()), Qt::QueuedConnection);
        connect(scriptProgressReporter, SIGNAL(taskFinished()), &scd, SLOT(scanTaskFinished()), Qt::QueuedConnection);
        //connect(scriptProgressReporter, SIGNAL(taskFinished()), &scanFactory, SLOT(finishScan()), Qt::QueuedConnection);
        //connect(scriptProgressReporter, SIGNAL(taskFinishedSoPlot()), &scanFactory, SLOT(finishScan()), Qt::QueuedConnection);

        emit scriptStarted(ew->scriptCode(), "", false);

        if (scd.exec() == QMessageBox::Abort) {
            stop();
            unhighlightScriptLine();
        }
    }
}

void MainWindow::runAutoScan()
{
    QFile common(QString::fromStdString(scripts->common));
    if (common.open(QIODevice::ReadOnly)) {
        emit scriptStarted(QString::fromUtf8(common.readAll().data()), QString::fromStdString(scripts->common), true);
    }
    AutoScanWindow asw(QString::fromStdString(scripts->autoScan), 0, 0);//devices::xAxisMotor->getMachineCoordinate(),devices::yAxisMotor->getMachineCoordinate());

    bool b = this->connect(&asw, SIGNAL(setProperty(const QString&, const QScriptValue&)), scriptExecutor, SLOT(setProperty(const QString&, const QScriptValue&)));
    connect(&asw, SIGNAL(startScanScript(const QString&, const QString&, bool)), this, SIGNAL(scriptStarted(const QString&, const QString&, bool)));
    connect(&asw, SIGNAL(stop()), this, SLOT(stop()));
    asw.exec();
}

void MainWindow::startRtCoil()
{
    try {
        //devices::coile->switchOnGenerator();
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        devices::coile->switchSingleWorkingMode(timing);
    }
    catch (...) {
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить генератор");
    }
}

void MainWindow::startRtCursor()
{

    try {
        auto collector = dynamic_cast<realtime::RTScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (!collector) {
            newRtWindow();
            collector = dynamic_cast<realtime::RTScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        }
        if (!collector) {}
        else {
            collector->startCursor();
        }
    }
    catch (...) {

    }
}

void MainWindow::startRt()
{
    try {
        //devices::coile->switchOnGenerator();
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        devices::coile->switchSingleWorkingMode(timing);
    }
    catch (...) {
        QMessageBox::critical(this, "Ошибка", "Не удалось запустить генератор");
    }

    try {
        auto collector = dynamic_cast<ScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (!collector) {
            newRtWindow();
            collector = dynamic_cast<ScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        }
        collector->start();
    }
    catch (...) {

    }
}

void MainWindow::stopRt()
{
    try {
        auto collector = dynamic_cast<ScanCollector*>(ui.mdiArea->currentSubWindow()->widget());
        if (collector)
            collector->stop();
    }
    catch (...) {

    }
    try {
        devices::coile->stop();
    }
    catch (...) {
        QMessageBox::critical(this, "Ошибка", "Не удалось остановить генератор");
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

void MainWindow::stop()
{
    scriptExecutor->stop();
    devices::audioDataCollector->stop();
    devices::coil->stop();
    devices::xAxisMotor->stop();
    devices::yAxisMotor->stop();
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
  auto sdw = new ScanDisplayWindow(scan, this);
  sdw->setAttribute(Qt::WA_DeleteOnClose, true);
  sdw->scanFactory = &scanFactory;

  ui.mdiArea->addSubWindow(sdw);
  sdw->showMaximized();
  connect(sdw,SIGNAL(refreshScan(std::shared_ptr<Scan>&)),&scanFactory,SLOT(recalculateScan(std::shared_ptr<Scan>&)));
  connect(sdw,SIGNAL(moveAlongSelectedDefect(const std::vector<Defect*>&, ::DefectSearchingParameters )),this,SLOT(moveAlongDefect(const std::vector<Defect*>&, ::DefectSearchingParameters)));
}

void MainWindow::showHideUnusedAction(bool needShow)
{
    ui.runScriptAction->setVisible(needShow);
    ui.stopAction->setVisible(needShow);
    ui.initializeAction->setVisible(needShow);
    ui.autoScanAction->setVisible(needShow);
    ui.showManualControlDialogAction->setVisible(needShow);
    //ui.quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));
    //ui.assembleScanAction, SIGNAL(triggered()), this, SLOT(showAssembleScanDialog()));
    //ui.exportWaveAction, SIGNAL(triggered()), this, SLOT(exportWave()));
    //ui.makeBScanAction, SIGNAL(triggered()), this, SLOT(makeBScanAction()));

    //ui.currentProcessingParametersAction, SIGNAL(triggered()), this, SLOT(showCurrentParameterDialog()));

    //ui.coilManualControlAction->setVisible(needShow);

    ui.undoAction ->setVisible(needShow);
    ui.redoAction ->setVisible(needShow);
    ui.cutAction  ->setVisible(needShow);
    ui.copyAction ->setVisible(needShow);
    ui.pasteAction->setVisible(needShow);
    ui.setZeroCoodinateButton->setVisible(needShow);

}

void MainWindow::moveAlongDefect(const std::vector<Defect*>& defectsIn,::DefectSearchingParameters defectSearching)
{
    script::StepMotor* xAxisScriptMotor = new script::StepMotor(devices::xAxisMotor, scriptEngine);
    xAxisScriptMotor->addMotor(devices::yAxisMotor);
    //QMessageBox::critical(this, "Ошибка", "Не удалось считать программу инициализации");
    auto dx = defectSearching.markerdxDblSpinBox;
    auto dy = defectSearching.markerdyDblSpinBox;
    auto vel = defectSearching.markerVelocityDblSpinBox;
    auto accel = defectSearching.markerAccelerationDblSpinBox;
    for (auto def : defectsIn) {
        auto point = def->contour.begin();
        xAxisScriptMotor->moveXYZ(vel, accel, accel, { point->x()+ dx,  point->y() + dy });
        QMessageBox putOnMarkerBox(QMessageBox::NoIcon, "Обводка контура.", "Закрепите маркер в рабочее положение!", { QMessageBox::Cancel, QMessageBox::Ok });
        putOnMarkerBox.setDefaultButton(QMessageBox::Ok);
        if (putOnMarkerBox.exec() == QMessageBox::Ok) {
            while(++point != def->contour.end()) {
                xAxisScriptMotor->moveXYZ(vel, accel, accel, { point->x() + dx,  point->y() + dy });
            };
        }
    }
    QMessageBox putOffMarkerBox(QMessageBox::NoIcon, "Обводка контура.", "Обводка контура закончена.\r\nУберите маркер.", { QMessageBox::Ok });
    putOffMarkerBox.exec();
    return Q_SLOT void();
}

Q_SLOT void MainWindow::addZCoordinates()
{
    auto currentWidget = getCurrentMdiWidget();
    if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
        auto par = sdw->getProcessingParameters();
        par.addZCoordinates = !par.addZCoordinates;
        sdw->applyParameters(par, scanFactory);
        return Q_SLOT void();
    }
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
void MainWindow::makeBScanAction()
{
    //try {
    //    auto currentWidget = getCurrentMdiWidget();
    //    if (auto ew = dynamic_cast<realtime::RTScanCollector*>(currentWidget)) {


    //        ew->processingParameters = &processingParameters;
    //        ew->makeScanAndShow(ew->m_scanArm, taskExecutor, scanFactory, ui.mdiArea);
    //        //ew->setWindowTitle(QString::fromUtf8( ew->m_scanArm->scanName));
    //    }
    //}
    //catch (...) {

    //}

    try {
        auto currentWidget = getCurrentMdiWidget();
        if (auto ew = dynamic_cast<realtime::RTScanCollector*>(currentWidget)) {


            ew->processingParameters = *processingParameters;
            ew->makeScanAndShow(ew->m_scanArm, taskExecutor, scanFactory, ui.mdiArea);
            //ew->setWindowTitle(QString::fromUtf8( ew->m_scanArm->scanName));
            //ew->makeScanAndShowCuttered(ew->m_scanArm, taskExecutor, scanFactory, ui.mdiArea);
            //ew->makeScanAndShowCutteredAbs(ew->m_scanArm, taskExecutor, scanFactory, ui.mdiArea);

        }
    }
    catch (...) {

    }

}

void MainWindow::coilManualControl()
{
  auto cmd = new CoilManualControlDialog;
  cmd->setAttribute(Qt::WA_DeleteOnClose, true);
  cmd->show();
}

/// <summary>
/// Дублируем строки по оси X и/или Y чтобы получить "отсканированное" поле нового размера
/// </summary>
void MainWindow::doubleLines() 
{
  auto currentWidget = getCurrentMdiWidget();
  if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
      QDialog dlg;
      dlg.setWindowTitle("Размеры заполняемого поля");

      // Радиокнопки
      QRadioButton* radio1NewSize = new QRadioButton("Новые размеры в мм");
      QRadioButton* radio2MultiplyersXY = new QRadioButton("Кратность размерам оригинала, раз");
      auto srnrts = sdw->getProcessingParameters().screenResizingNeededRatherThanScaleCount;
      if (srnrts) {
          radio1NewSize->setChecked(true);
      }
      else {
          radio2MultiplyersXY->setChecked(true);
      }

      QVBoxLayout* radioLayout = new QVBoxLayout;
      radioLayout->addWidget(radio2MultiplyersXY);
      radioLayout->addWidget(radio1NewSize);

      // Метки
      QLabel* labelX = new QLabel("x:");
      QLabel* labelY = new QLabel("y:");
      // Поля ввода
      QDoubleSpinBox* targetXSize = new QDoubleSpinBox();
      targetXSize->setRange(1.0, 20000.0);
      QDoubleSpinBox* targetYSize = new QDoubleSpinBox();
      targetYSize->setRange(1.0, 20000.0);
      // Горизонтальные layout'ы для строк
      QHBoxLayout* layoutX = new QHBoxLayout();
      layoutX->addWidget(labelX);
      layoutX->addWidget(targetXSize);
      QHBoxLayout* layoutY = new QHBoxLayout();
      layoutY->addWidget(labelY);
      layoutY->addWidget(targetYSize);
      radioLayout->addLayout(layoutX);
      radioLayout->addLayout(layoutY);

      QGroupBox* radioGroup = new QGroupBox("Выбор режима");
      radioGroup->setLayout(radioLayout);

      //*****
      QVBoxLayout* ScreenCountGroupLayout = new QVBoxLayout;

      QCheckBox* ScreenCountNeededCheckBox = new QCheckBox("Разбивать окно на виды");
      auto scn = sdw->getProcessingParameters().screenCountNeeded;
      ScreenCountNeededCheckBox->setChecked(scn);
      QCheckBox* CommonViewNeededCheckBox = new QCheckBox("Показать общий вид");
      auto cvn = sdw->getProcessingParameters().commonViewNeeded;
      CommonViewNeededCheckBox->setChecked(cvn);
      QLabel* labelScreenCountStart = new QLabel("Разбить вид на ");
      QLabel* labelScreenCountEnd = new QLabel(" частей");
      QSpinBox* screenCountBox = new QSpinBox();
      screenCountBox->setRange(1, 100);
      int scb = sdw->getProcessingParameters().screenCount;
      screenCountBox->setValue(scb);
      QHBoxLayout* layoutScreenCount = new QHBoxLayout();
      layoutScreenCount->addWidget(labelScreenCountStart);
      layoutScreenCount->addWidget(screenCountBox);
      layoutScreenCount->addWidget(labelScreenCountEnd);

      ScreenCountGroupLayout->addWidget(ScreenCountNeededCheckBox);
      ScreenCountGroupLayout->addWidget(CommonViewNeededCheckBox);
      ScreenCountGroupLayout->addLayout(layoutScreenCount);

      QGroupBox* ScreenCountGroup = new QGroupBox("Разбивка на экраны");
      ScreenCountGroup->setLayout(ScreenCountGroupLayout);

      //*****

      QDialogButtonBox* buttonBox = new QDialogButtonBox(
          QDialogButtonBox::Ok | QDialogButtonBox::Cancel
      );

      connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));

      QObject::connect(buttonBox, &QDialogButtonBox::rejected,
          &dlg, &QDialog::reject);

      // Основной layout
      QVBoxLayout* mainLayout = new QVBoxLayout();
      mainLayout->addWidget(radioGroup);
      //mainLayout->addLayout(layoutX);
      //mainLayout->addLayout(layoutY);
      mainLayout->addWidget(ScreenCountGroup);
      mainLayout->addWidget(buttonBox);

      dlg.setLayout(mainLayout);

      if (dlg.exec() == QDialog::Accepted) {
          double newXSize;
          double newYSize;
          newXSize = targetXSize->value();
          newYSize = targetYSize->value();
          auto parameters = sdw->getProcessingParameters();
          parameters.screenCount = screenCountBox->value();
          parameters.screenResizingNeededRatherThanScaleCount = radio1NewSize->isChecked();
          parameters.screenCountNeeded = ScreenCountNeededCheckBox->isChecked();
          parameters.commonViewNeeded  = CommonViewNeededCheckBox->isChecked();

          if (radio1NewSize->isChecked()) {
              sdw->multiSizeLines(parameters, scanFactory, newXSize, newYSize);
          }
          else {
              sdw->doubleSizeLines(parameters, scanFactory, newXSize, newYSize);
          }
          //sdw->multiSizeLines(sdw->getProcessingParameters(), scanFactory, newXSize, newYSize);
          sdw->ShowNView(parameters.screenCount);
      }
  }
}

void MainWindow::showSelectNormalizationAreaDialog()
{
    auto currentWidget = getCurrentMdiWidget();
    if (auto sdw = dynamic_cast<ScanDisplayWindow*>(currentWidget)) {
        ProcessingParameters par = sdw->getProcessingParameters();

        RectSelectionDialog dialog = RectSelectionDialog(this, par.specNormalizationIJRect);
        
        if (dialog.exec() == QDialog::Accepted) {
            RectForNormalization* selected = dialog.getSelectedRect();
            qDebug() << "Selected rect:"
                << selected->xLowLeft << selected->yLowLeft << selected->zLowLeft
                << selected->xTopRight << selected->yTopRight << selected->zTopRight;
            //// Optionally, show in a message box
            //QMessageBox::information(this, tr("Выбрано"),
            //    QString("Выбранный прямоугольник: (%1, %2, %3) - (%4, %5, %6)")
            //    .arg(selected.xLowLeft)
            //    .arg(selected.yLowLeft)
            //    .arg(selected.zLowLeft)
            //    .arg(selected.xTopRight)
            //    .arg(selected.yTopRight)
            //    .arg(selected.zTopRight));
            QPoint p1(selected->xLowLeft, selected->yLowLeft);
            QPoint p2(selected->xTopRight, selected->yTopRight);
            QRect  rect  = QRect(p1, p2);
            sdw->normalizeRegionExternalStart(rect);
        }
        sdw->setProcessingParameters(par);
    }
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
  else if (auto sdw = dynamic_cast<ScanCollector*>(currentWidget)) {
    ProcessingParametersDialog ppd(sdw->getProcessingParameters(), false);
    if (ppd.exec() == QDialog::Accepted) {
        sdw->applyParameters(ppd.getProcessingParameters()); 
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
