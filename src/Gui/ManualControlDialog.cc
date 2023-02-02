/*
 * Gui/ManualControlDialog.cc
 */

#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>

#include "Core/Devices.hh"
#include "Gui/ManualControlDialog.hh"

ManualControlDialog::ManualControlDialog(QWidget* parent)
{
  ui.setupUi(this);
}

void ManualControlDialog::keyPressEvent(QKeyEvent* e)
{
  if (e->isAutoRepeat()) return;

  switch(e->key()) {
  case Qt::Key_Up:
    break;
  case Qt::Key_Down:
    break;
  case Qt::Key_Left:
    break;
  case Qt::Key_Right:
    break;
  case Qt::Key_Space:
    break;
  default:
    QDialog::keyPressEvent(e);
  }
}

void ManualControlDialog::keyReleaseEvent(QKeyEvent* e)
{
  if (e->isAutoRepeat()) return;
  switch(e->key()) {
  case Qt::Key_Up:
  case Qt::Key_Down:
    break;
  case Qt::Key_Left:
  case Qt::Key_Right:
    break;
  default:
    QDialog::keyReleaseEvent(e);
  }
}

void ManualControlDialog::mousePressEvent(QMouseEvent* e)
{
  setFocus();
}
