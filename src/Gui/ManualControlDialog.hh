/*
 * Gui/ManualControlDialog.hh
 */

#pragma once

#include "ui_ManualControlDialog.h"

class ManualControlDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ManualControlDialog(QWidget* parent = 0);

protected:
  virtual void keyPressEvent(QKeyEvent* e) override;
  virtual void keyReleaseEvent(QKeyEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;

private:
  Ui::ManualControlDialog ui;

};