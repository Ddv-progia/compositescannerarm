/*
 * Core/QtScript/StepMotor.hh
 */

#pragma once

#include <QtCore/QObject>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include "Core/StepMotor.hh"

namespace script {

  class StepMotor : public QObject
  {
    Q_OBJECT
  public:
    StepMotor(const devices::StepMotorPtr& motor, QScriptEngine* scriptEngine);

    Q_INVOKABLE QScriptValue getMachineCoordinate();
    Q_INVOKABLE QScriptValue getTechnologicalCoordinate();

    Q_INVOKABLE void moveForward(double velocity, double startAcceleration, double stopAcceleration);
    Q_INVOKABLE void moveBackward(double velocity, double startAcceleration, double stopAcceleration);

    Q_INVOKABLE void moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target);
    Q_INVOKABLE void moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target);

    Q_INVOKABLE void moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target);

    Q_INVOKABLE void moveIntoZero(double velocity, double startAcceleration, double stopAcceleration);
    Q_INVOKABLE void moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration);

    Q_INVOKABLE void stop();
  private:
    devices::StepMotorPtr motor;
    QScriptEngine* scriptEngine;

    template<typename F>
    void wrapExceptions(F thunk);

    template<typename F>
    QScriptValue valWrapExceptions(F thunk);
  };

}