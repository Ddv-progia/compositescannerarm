/*
 * Core/QtScript/StepMotor.cc
 */

#include "Core/QtScript/StepMotor.hh"
#include <QtCore/QString>
#include <QtWidgets/QMessageBox>

script::StepMotor::StepMotor(const devices::StepMotorPtr& motor, QJSEngine* scriptEngine)
  : motor(motor), scriptEngine(scriptEngine)
{ }

template<typename F>
void script::StepMotor::wrapExceptions(F thunk)
{
  try {
    thunk();
  } catch (uts::devtalk::IncorrectArgumentException& e) {
      auto title = QString::fromUtf8("Ошибка/n");
      auto str = QString::fromUtf8(e.what());
      QMessageBox::critical(nullptr, title, str);
      scriptEngine->throwError(QString::fromUtf8(e.description.c_str()));

  } catch (uts::devtalk::CommunicationException& e) {
      scriptEngine->throwError(QString::fromUtf8(e.reason.c_str()));
  } catch (Ice::Exception& e) {
      scriptEngine->throwError(QString::fromUtf8(e.what()));
  }
    //QMessageBox::critical(this, QString::fromStdString("Ошибка/n"), QString::fromUtf8(e.what()));
}

template<typename F>
QJSValue script::StepMotor::valWrapExceptions(F thunk)
{
  try {
      thunk();
      return QString("Ok");
  } catch (uts::devtalk::IncorrectArgumentException& e) {
    scriptEngine->throwError(QString::fromUtf8(e.description.c_str()));
  } catch (uts::devtalk::CommunicationException& e) {
    scriptEngine->throwError(QString::fromUtf8(e.reason.c_str()));
  } catch (Ice::Exception& e) {
    scriptEngine->throwError(QString::fromUtf8(e.what()));
  }
  //QMessageBox::critical(this, QString::fromStdString("Ошибка/n"), QString::fromUtf8(e.what()));
  return QString::fromStdString("");
}

QJSValue script::StepMotor::getMachineCoordinate()
{
  return valWrapExceptions([=] () -> QString { return QString::number(motor->getMachineCoordinate(), 'f', 3);  });
}

QJSValue script::StepMotor::getTechnologicalCoordinate()
{
  return valWrapExceptions([=] () { return motor->getTechnologicalCoordinate(); });
}

void script::StepMotor::moveForward(double velocity, double startAcceleration, double stopAcceleration)
{
  wrapExceptions([=] () { motor->moveForward(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveBackward(double velocity, double startAcceleration, double stopAcceleration)
{
  wrapExceptions([=] () { motor->moveBackward(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  wrapExceptions([=] () { motor->moveToMachine(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  wrapExceptions([=] () { motor->moveRelative(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  wrapExceptions([=] () { motor->moveToTechnological(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveIntoZero(double velocity, double startAcceleration, double stopAcceleration)
{
  wrapExceptions([=] () { motor->moveIntoZero(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration)
{
  wrapExceptions([=] () { motor->moveOutOfZero(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::stop()
{
  wrapExceptions([=] () { motor->stop(); });
}
