/*
 * Gui/CoilManualControl.cc
 */

#include <QtWidgets/QMessageBox>
#include <QSplitter>

#include "Core/Devices.hh"
#include "Gui/CoilManualControl.hh"

CoilCommandPanel::CoilCommandPanel(const uts::devtalk::CoilPrx& coilOld, int currentIndex, QWidget* parent)
    : QWidget(parent), coilOld(coilOld), useCoilOld(true)
{
    ui.setupUi(this);

    updateCommands();
    for (std::size_t i = 0; i < commands.size(); i++)
        ui.commandBox->addItem(commands[i].first);
    ui.commandBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.commandBox->setCurrentIndex(currentIndex);
    periodicCommandExecutionTimer = new QTimer(this);

    connect(ui.runButton, SIGNAL(clicked()), this, SLOT(runCurrentCommand()));
    connect(ui.runCycleButton, SIGNAL(clicked()), this, SLOT(runCurrentCommandPeriodically()));
    connect(ui.stopCycleButton, SIGNAL(clicked()), periodicCommandExecutionTimer, SLOT(stop()));
    connect(periodicCommandExecutionTimer, SIGNAL(timeout()), this, SLOT(runCurrentCommand()));

    connect(ui.addButton, SIGNAL(clicked()), this, SIGNAL(addControlPanel()));
    connect(ui.removeButton, SIGNAL(clicked()), this, SIGNAL(removeControlPanel()));
    addControlPanel();
}


CoilCommandPanel::CoilCommandPanel(const uts::devtalk::device::utscp::APLCoilPrx& coil, int currentIndex, QWidget* parent)
  : QWidget(parent), coil(coil), useCoilOld(false)
{
  ui.setupUi(this);

  updateCommands();
  for (std::size_t i = 0; i < commands.size(); i++)
    ui.commandBox->addItem(commands[i].first);
  ui.commandBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  ui.commandBox->setCurrentIndex(currentIndex);
  periodicCommandExecutionTimer = new QTimer(this);

  connect(ui.runButton, SIGNAL(clicked()), this, SLOT(runCurrentCommand()));
  connect(ui.runCycleButton, SIGNAL(clicked()), this, SLOT(runCurrentCommandPeriodically()));
  connect(ui.stopCycleButton, SIGNAL(clicked()), periodicCommandExecutionTimer, SLOT(stop()));
  connect(periodicCommandExecutionTimer, SIGNAL(timeout()), this, SLOT(runCurrentCommand()));

  connect(ui.addButton, SIGNAL(clicked()), this, SIGNAL(addControlPanel()));
  connect(ui.removeButton, SIGNAL(clicked()), this, SIGNAL(removeControlPanel()));
  addControlPanel();
}

void CoilCommandPanel::updateCommandsNew()
{
    commands.push_back(std::make_pair("Считать версию", [this]() { showVersion(coil->getFirmwareVersion()); }));
    commands.push_back(std::make_pair("Стоп", [this]() { coil->stop(); }));
    commands.push_back(std::make_pair("Восстановить настройки из EEPROM", [this]() { coil->resetFromEEPROM(); }));
    commands.push_back(std::make_pair("Сохранить настройки в EEPROM", [this]() { coil->saveToEEPROPM(); }));
    commands.push_back(std::make_pair("Восстановить начальные настройки", [this]() { coil->resetDefaults(); }));
    commands.push_back(std::make_pair("Включить генератор", [this]() { coil->switchOnGenerator(); }));
    commands.push_back(std::make_pair("Искать рабочий диапазон", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coil->searchWorkingRange(timing);
        }));
    commands.push_back(std::make_pair("Включить рабочий режим", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coil->switchWorkingMode(timing);
        }));
    commands.push_back(std::make_pair("Однократный старт", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coil->switchSingleWorkingMode(timing);
        }));
    commands.push_back(std::make_pair("Считать статус", [this]() { showState(coil->getState()); }));
    commands.push_back(std::make_pair("Считать уровень звука", [this]() { showResult(coil->getLevel()); }));
    commands.push_back(std::make_pair("Считать АЦП", [this]() { showResult(coil->getADC()); }));
    commands.push_back(std::make_pair("Увеличить полупериод на Х", [this]() { coil->increaseHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Уменьшить полупериод на Х", [this]() { coil->decreaseHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Увеличить ширину на Х", [this]() { coil->increaseWidth(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Уменьшить ширину на Х", [this]() { coil->decreaseWidth(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать полупериод", [this]() { showResult(coil->getHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать полупериод", [this]() { coil->setHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать ширину импульса", [this]() { showResult(coil->getWidth()); }));
    commands.push_back(std::make_pair("Записать ширину импульса", [this]() { coil->setWidth(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать мощность", [this]() { showResult(coil->getPower()); }));
    commands.push_back(std::make_pair("Записать мощность", [this]() { coil->setPower(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать максимальный полупериод", [this]() { showResult(coil->getMaximumHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать максимальный полупериод", [this]() { coil->setMaximumHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать минимальный полупериод", [this]() { showResult(coil->getMinimumHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать минимальный полупериод", [this]() { coil->setMinimumHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать шаг изменения полупериода", [this]() { showResult(coil->getHalfPeriodStep()); }));
    commands.push_back(std::make_pair("Записать шаг изменения полупериода", [this]() { coil->setHalfPeriodStep(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать стартовый полупериод", [this]() { showResult(coil->getInitialHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать стартовый полупериод", [this]() { coil->setInitialHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать рабочий полупериод", [this]() { showResult(coil->getWorkingHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать рабочий полупериод", [this]() { coil->setWorkingHalfPeriod(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать контрольное время", [this]() { showResult(coil->getTestTime()); }));
    commands.push_back(std::make_pair("Записать контрольное время", [this]() { coil->setTestTime(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать минимальный уровень звука", [this]() { showResult(coil->getMinimumLevel()); }));
    commands.push_back(std::make_pair("Записать минимальный уровень звука", [this]() { coil->setMinimumLevel(ui.argumentEdit->text().toDouble()); }));
}

void CoilCommandPanel::updateCommandsOld()
{
    commands.push_back(std::make_pair("Считать версию", [this]() { showVersion(coilOld->getFirmwareVersion()); }));
    commands.push_back(std::make_pair("Стоп", [this]() { coilOld->stop(); }));
    commands.push_back(std::make_pair("Восстановить настройки из EEPROM", [this]() { coilOld->resetFromEEPROM(); }));
    commands.push_back(std::make_pair("Сохранить настройки в EEPROM", [this]() { coilOld->saveToEEPROPM(); }));
    commands.push_back(std::make_pair("Восстановить начальные настройки", [this]() { coilOld->resetDefaults(); }));
    commands.push_back(std::make_pair("Включить генератор", [this]() { coilOld->switchOnGenerator(); }));
    commands.push_back(std::make_pair("Искать рабочий диапазон", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coilOld->searchWorkingRange(timing);
        }));
    commands.push_back(std::make_pair("Включить рабочий режим", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coilOld->switchWorkingMode(timing);
        }));
    commands.push_back(std::make_pair("Однократный старт", [this]() {
        IceUtil::Handle<uts::devtalk::CompletionWaitTiming> timing = new uts::devtalk::CompletionWaitTiming;
        timing->firstTestDelay = timing->testPause = timing->timeout = 0;
        coilOld->switchSingleWorkingMode(timing);
        }));
    commands.push_back(std::make_pair("Считать статус", [this]() { showState(coilOld->getState()); }));
    commands.push_back(std::make_pair("Считать уровень звука", [this]() { showResult(coilOld->getLevel()); }));
    commands.push_back(std::make_pair("Считать АЦП", [this]() { showResult(coilOld->getADC()); }));
    commands.push_back(std::make_pair("Увеличить полупериод на Х", [this]() { coilOld->increaseHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Уменьшить полупериод на Х", [this]() { coilOld->decreaseHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Увеличить ширину на Х", [this]() { coilOld->increaseWidth(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Уменьшить ширину на Х", [this]() { coilOld->decreaseWidth(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать полупериод", [this]() { showResult(coilOld->getHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать полупериод", [this]() { coilOld->setHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать ширину импульса", [this]() { showResult(coilOld->getWidth()); }));
    commands.push_back(std::make_pair("Записать ширину импульса", [this]() { coilOld->setWidth(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать мощность", [this]() { showResult(coilOld->getPower()); }));
    commands.push_back(std::make_pair("Записать мощность", [this]() { coilOld->setPower(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать максимальный полупериод", [this]() { showResult(coilOld->getMaximumHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать максимальный полупериод", [this]() { coilOld->setMaximumHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать минимальный полупериод", [this]() { showResult(coilOld->getMinimumHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать минимальный полупериод", [this]() { coilOld->setMinimumHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать шаг изменения полупериода", [this]() { showResult(coilOld->getHalfPeriodStep()); }));
    commands.push_back(std::make_pair("Записать шаг изменения полупериода", [this]() { coilOld->setHalfPeriodStep(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать стартовый полупериод", [this]() { showResult(coilOld->getInitialHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать стартовый полупериод", [this]() { coilOld->setInitialHalfPeriod(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать рабочий полупериод", [this]() { showResult(coilOld->getWorkingHalfPeriod()); }));
    commands.push_back(std::make_pair("Записать рабочий полупериод", [this]() { coilOld->setWorkingHalfPeriod(ui.argumentEdit->text().toDouble()); }));

    commands.push_back(std::make_pair("Считать контрольное время", [this]() { showResult(coilOld->getTestTime()); }));
    commands.push_back(std::make_pair("Записать контрольное время", [this]() { coilOld->setTestTime(ui.argumentEdit->text().toDouble()); }));
    commands.push_back(std::make_pair("Считать минимальный уровень звука", [this]() { showResult(coilOld->getMinimumLevel()); }));
    commands.push_back(std::make_pair("Записать минимальный уровень звука", [this]() { coilOld->setMinimumLevel(ui.argumentEdit->text().toDouble()); }));

}

void CoilCommandPanel::updateCommands()
{
  commands.clear();
  if (useCoilOld) {
      updateCommandsOld();
  }
  else {
      updateCommandsNew();
  }
}

void CoilCommandPanel::showResult(double x)
{
  ui.answerLabel->setText(QString::number(x));
}

void CoilCommandPanel::showVersion(uts::devtalk::Date x)
{
  ui.answerLabel->setText(QString("%1-%2-%3").arg(x.year).arg(x.month).arg(x.day));
}

void CoilCommandPanel::showState(const uts::devtalk::CoilState& state)
{
  auto jobToString = [] (uts::devtalk::CoilJob job) -> QString {
    switch (job) {
    default: return "";
    case uts::devtalk::CoilJobWorkingRangeSearch: return "начало поиска рабочего диапазона";
    case uts::devtalk::CoilJobFirstResponseSearch: return "поиск первого отклика";
    case uts::devtalk::CoilJobResponseStabilityTest: return "проверка устойчивости отклика";
    case uts::devtalk::CoilJobResponseLossSearch: return "поиск срыва отклика";
    case uts::devtalk::CoilJobResponseLessStabilityTest: return "проверка устойчивости срыва";
    case uts::devtalk::CoilJobResonanceFrequencyStart: return "старт с резонансной частоты";
    case uts::devtalk::CoilJobRaisingFrequencyToWorking: return "повышение частоты до рабочей";
    case uts::devtalk::CoilJobWorking: return "работа";
    }
  };

  auto failureToString = [] (const uts::devtalk::CoilFailure failure) -> QString {
    switch (failure) {
    default: return "";
    case uts::devtalk::CoilFailureNoResponse: return "не найден отклик на всём диапазоне ";
    case uts::devtalk::CoilFailureNoUpperLimit: return "не найдена верхняя граница частоты";
    case uts::devtalk::CoilFailureNoWorkingReponse: return "не найден отклик на рабочем диапазоне";
    case uts::devtalk::CoilFailureLostSignal: return "потеря сигнала при работе";
    }
  };

  ui.answerLabel->setText(QString("%1|%2|%3|%4 Job: %5 Fail: %6 Inputs: %7")
    .arg(state.working ? "<b>W</b>" : "W")
    .arg(state.sufficientLevel ? "<b>L</b>" : "L")
    .arg(state.frequencyFound ? "<b>F</b>" : "F")
    .arg(state.ready ? "<b>R</b>" : "R")
    .arg(jobToString(state.job))
    .arg(failureToString(state.failure))
    .arg(QString::number(state.digitalInputs, 2)));
}

void CoilCommandPanel::runCurrentCommand()
{
  try {
      commands[ui.commandBox->currentIndex()].second();
  } catch (uts::devtalk::CommunicationException& e) {
    QMessageBox::critical(this, "Ошибка", e.reason.c_str());
  } catch (Ice::Exception& e) {
    QMessageBox::critical(this, "Ошибка", e.what());
  } catch (std::exception& e) {
    QMessageBox::critical(this, "Ошибка", e.what());
  }
}

void CoilCommandPanel::runCurrentCommandPeriodically()
{
  periodicCommandExecutionTimer->start(ui.commandPeriodBox->value());
}

CoilManualControlDialog::CoilManualControlDialog(QWidget* parent)
{
  setWindowTitle(QString::fromUtf8("Датчик АСК"));
  panelsLayout = new QVBoxLayout;

  panelsLayout->addStretch();
  setLayout(panelsLayout);
  
  addControlPanelWithIndex(10);
  addControlPanelWithIndex(35);
  addControlPanelWithIndex(34);
  addControlPanelWithIndex(19);
  addControlPanelWithIndex(18);
  addControlPanelWithIndex(31);
  addControlPanelWithIndex(30);
  addControlPanelWithIndex(29);
  addControlPanelWithIndex(28);
  addControlPanelWithIndex(21);
  addControlPanelWithIndex(20);
  addControlPanelWithIndex(17);
  addControlPanelWithIndex(16);
  addControlPanelWithIndex(8);
  addControlPanelWithIndex(7);
  addControlPanelWithIndex(6);
  addControlPanelWithIndex(1);
}

CoilCommandPanel* CoilManualControlDialog::addControlPanelWithIndex(int index)
{
    CoilCommandPanel* ccp;
    if (!devices::coile)
    {
        ccp = new CoilCommandPanel(devices::coil, index); //TODO проверить,какая головка не пустая и создавать для неё 

    }
    else
    {
        ccp = new CoilCommandPanel(devices::coile, index);

    }
    //auto ccp = new CoilCommandPanel(devices::coile, index);
    connect(ccp, SIGNAL(addControlPanel()), this, SLOT(addControlPanel()));
    connect(ccp, SIGNAL(removeControlPanel()), ccp, SLOT(deleteLater()));
    panelsLayout->insertWidget(panelsLayout->indexOf(qobject_cast<QWidget*>(sender())) + 1, ccp);
    return ccp;
}

void CoilManualControlDialog::addControlPanel()
{
    CoilCommandPanel* ccp;
    if (!devices::coile)
    {
        ccp = new CoilCommandPanel(devices::coil);
    }
    else
    {
        ccp = new CoilCommandPanel(devices::coile);
    }
  ////auto ccp = new CoilCommandPanel(devices::coil); //TODO проверить,какая головка не пустая и создавать для неё 
  //auto ccp = new CoilCommandPanel(devices::coile);
  connect(ccp, SIGNAL(addControlPanel()), this, SLOT(addControlPanel()));
  connect(ccp, SIGNAL(removeControlPanel()), ccp, SLOT(deleteLater()));
  panelsLayout->insertWidget(panelsLayout->indexOf(qobject_cast<QWidget*>(sender())) + 1, ccp);
}
