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

  connect(ui.leftButton, SIGNAL(clicked()), this, SLOT(moveXMinus()));
  connect(ui.rightButton, SIGNAL(clicked()), this, SLOT(moveXPlus()));
  connect(ui.upButton, SIGNAL(clicked()), this, SLOT(moveYMinus()));
  connect(ui.downButton, SIGNAL(clicked()), this, SLOT(moveYPlus()));
  connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(stop()));
}

void ManualControlDialog::keyPressEvent(QKeyEvent* e)
{
  if (e->isAutoRepeat()) return;

  switch(e->key()) {
  case Qt::Key_Up:
    moveYPlus();
    break;
  case Qt::Key_Down:
    moveYMinus();
    break;
  case Qt::Key_Left:
    moveXMinus();
    break;
  case Qt::Key_Right:
    moveXPlus();
    break;
  case Qt::Key_Space:
    stop();
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
    stopY();
    break;
  case Qt::Key_Left:
  case Qt::Key_Right:
    stopX();
    break;
  default:
    QDialog::keyReleaseEvent(e);
  }
}

void ManualControlDialog::mousePressEvent(QMouseEvent* e)
{
  setFocus();
}

void ManualControlDialog::moveXPlus()
{
  try {
    devices::xAxisMotor->beginMoveForward(ui.xVelocityBox->value(), ui.xAccelerationBox->value(), ui.xAccelerationBox->value());
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}

void ManualControlDialog::moveXMinus()
{
  try {
    devices::xAxisMotor->beginMoveBackward(ui.xVelocityBox->value(), ui.xAccelerationBox->value(), ui.xAccelerationBox->value());
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}

void ManualControlDialog::moveYMinus()
{
  try {
    devices::yAxisMotor->beginMoveBackward(ui.yVelocityBox->value(), ui.yAccelerationBox->value(), ui.yAccelerationBox->value());
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}

void ManualControlDialog::moveYPlus()
{
  try {
    devices::yAxisMotor->beginMoveForward(ui.yVelocityBox->value(), ui.yAccelerationBox->value(), ui.yAccelerationBox->value());
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}

void ManualControlDialog::stop()
{
  stopX();
  stopY();
}

void ManualControlDialog::stopX()
{
  try {
    devices::xAxisMotor->stop();
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}

void ManualControlDialog::stopY()
{
  try {
    devices::yAxisMotor->stop();
  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(exc.what()));
  }
}
