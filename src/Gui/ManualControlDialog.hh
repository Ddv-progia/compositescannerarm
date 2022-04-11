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

  Q_SLOT void moveXPlus();
  Q_SLOT void moveXMinus();
  Q_SLOT void moveYMinus();
  Q_SLOT void moveYPlus();
  Q_SLOT void stop();
  void stopX();
  void stopY();
};