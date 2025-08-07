/*
 * Core/QtScript/StepMotor.hh
 */

#pragma once

#include <QtCore/QObject>
#include <QString>
#include <QJSEngine>
#include <QJSValue>
#include "Core/StepMotor.hh"

namespace script {

  class StepMotor : public QObject
  {
    Q_OBJECT
  public:
    StepMotor(const devices::StepMotorPtr& motor, QJSEngine* scriptEngine);
    //double velocityOwn= 10;
    //double startAcceleration = 10;
    //double stopAcceleration = 10;
    Q_INVOKABLE double getMachineCoordinate(devices::StepMotorPtr motor = nullptr);
    Q_INVOKABLE double getTechnologicalCoordinate(devices::StepMotorPtr motor = nullptr);

    Q_INVOKABLE void moveForward(double velocity , double startAcceleration, double stopAcceleration, devices::StepMotorPtr motor = nullptr);
    Q_INVOKABLE void moveBackward(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motor = nullptr);

    Q_INVOKABLE void moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motor = nullptr);
    Q_INVOKABLE void moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motor = nullptr);

    Q_INVOKABLE void moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motor = nullptr);

    Q_INVOKABLE void moveIntoZero(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motor = nullptr);
    Q_INVOKABLE void moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motor = nullptr);

    Q_INVOKABLE void moveXYZ(double velocity, double startAcceleration, double stopAcceleration, std::vector<double> target);

    Q_INVOKABLE void stop(devices::StepMotorPtr motor = nullptr);
    Q_INVOKABLE std::size_t addMotor(const devices::StepMotorPtr& motor);
    Q_INVOKABLE devices::StepMotorPtr motorsAt(int index);
    ::std::vector<devices::StepMotorPtr > motors;
  private:
    devices::StepMotorPtr motor;
    QJSEngine* scriptEngine;

    template<typename F>
    void wrapExceptions(F thunk);

    template<typename F>
    double valWrapExceptions(F thunk);
    //QJSValue valWrapExceptions(F thunk);

  };

}