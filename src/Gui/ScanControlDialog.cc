/*
 * Gui/ScanControlDialog.cc
 */

#include "Gui/ScanControlDialog.hh"

ScanControlDialog::ScanControlDialog(QWidget* parent)
  : QMessageBox(parent), totalSteps(0), stepsMade(0)
{
  setIcon(QMessageBox::NoIcon);
  setWindowTitle("Выполняется сканирование");
  setText("Остановить сканирование");
  setStandardButtons(QMessageBox::Abort);
  setDefaultButton(QMessageBox::Abort);
}

void ScanControlDialog::newScanTask(int stepCount)
{
  totalSteps = stepCount;
  stepsMade = 0;
  setText(QString("Остановить сканирование (%1/%2)").arg(stepsMade + 1).arg(totalSteps));
}

void ScanControlDialog::scanTaskProgressed()
{
  stepsMade++;
  setText(QString("Остановить сканирование (%1/%2)").arg(stepsMade + 1).arg(totalSteps));
}

void ScanControlDialog::scanTaskFinished()
{

  setText("Остановить сканирование");
}
