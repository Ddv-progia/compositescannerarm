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
  explicit CoilCommandPanel(const uts::devtalk::CoilPrx& coil, QWidget* parent = 0);

protected:
  Q_SIGNAL void addControlPanel();
  Q_SIGNAL void removeControlPanel();

private:
  Ui::SingleCommandPanel ui;
  uts::devtalk::CoilPrx coil;
  std::vector<std::pair<QString, std::function<void ()>>> commands;
  
  QTimer* periodicCommandExecutionTimer;

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

  Q_SLOT void addControlPanel();
  void addControlPanel(int commandIndex);
};
