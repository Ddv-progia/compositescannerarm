/*
 * Gui/CoilManualControl.hh
 */

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <DevTalk/Device/Coil.hh>
#include <QtCore/QTimer>
#include <QtWidgets/QDialog>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

#include "ui_SingleCommandPanel.h"

class CoilCommandPanel : public QWidget
{
  Q_OBJECT
public:
  explicit CoilCommandPanel(const uts::devtalk::device::utscp::APLCoilPrx& coil, int currentIndex = 1, QWidget* parent = 0);
  explicit CoilCommandPanel(const uts::devtalk::CoilPrx& coilOld,                int currentIndex = 1, QWidget* parent = 0);

protected:
  Q_SIGNAL void addControlPanel();
  Q_SIGNAL void removeControlPanel();

private:
  Ui::SingleCommandPanel ui;
  uts::devtalk::device::utscp::APLCoilPrx coil;
  uts::devtalk::CoilPrx coilOld;

  bool useCoilOld;

  std::vector<std::pair<QString, std::function<void ()>>> commands;
  
  QTimer* periodicCommandExecutionTimer;

  void updateCommandsNew();
  void updateCommandsOld();
  void updateCommands();
  void showResult(double x);
  void showState(const uts::devtalk::CoilState& state);
  void showVersion(uts::devtalk::Date x);
  Q_SLOT void runCurrentCommand();
  Q_SLOT void runCurrentCommandPeriodically();
};

class CoilManualControlDialog : public QDialog
{
  Q_OBJECT
public:
  explicit CoilManualControlDialog(QWidget* parent = 0);

private:
  QVBoxLayout* panelsLayout;

  CoilCommandPanel* addControlPanelWithIndex(int index);
  Q_SLOT void addControlPanel();
  //void addControlPanel(int commandIndex);
};
