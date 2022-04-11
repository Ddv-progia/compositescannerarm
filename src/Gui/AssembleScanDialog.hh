/*
 * Gui/AssembleScanDialog.hh
 */

#pragma once

#include <vector>
#include <QtCore/QString>

#include "ui_AssembleScanDialog.h"

class AssembleScanDialog : public QDialog
{
  Q_OBJECT
public:
  explicit AssembleScanDialog(QWidget* parent = 0);
  std::vector<QString> getFiles();

private:
  Ui::AssembleScanDialog ui;

  void connectSignals();

  Q_SLOT void addSource();
  Q_SLOT void removeSource();
  Q_SLOT void clearSources();

  Q_SLOT void moveTop();
  Q_SLOT void moveUp();
  Q_SLOT void moveDown();
  Q_SLOT void moveBottom();
};