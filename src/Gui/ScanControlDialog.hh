/*
 * Gui/ScanControlDialog.hh
 */

#pragma once

#include <QtWidgets/QMessageBox>

class ScanControlDialog : public QMessageBox
{
  Q_OBJECT
public:
  explicit ScanControlDialog(QWidget* parent = 0);

  Q_SLOT void newScanTask(int stepCount);
  Q_SLOT void scanTaskProgressed();
  Q_SLOT void scanTaskFinished();

private:
  int totalSteps;
  int stepsMade;
};
