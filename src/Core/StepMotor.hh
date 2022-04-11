/*
 * Core/SerialStepMotor.hh
 */

#pragma once

#include <memory>
#include <boost/thread/mutex.hpp>
#include <DevTalk/Device/SerialStepMotor.hh>

namespace devices {

  class StepMotor
  {
  public:
    StepMotor(const uts::devtalk::SerialStepMotorPrx& motor,
              unsigned stepsPerRevolution,
              double motorReduction,
	            double toothStep,
	            double toothCount);

    double getMachineCoordinate() const;
    double getTechnologicalCoordinate() const;

    void setTechnologicalZero();
    void setTechnologicalZero(double newTechnologicalZero);

    bool isMoving();
    void waitStop(int pollPeriod = 500);

    void moveForward(double velocity, double startAcceleration, double stopAcceleration);
    void beginMoveForward(double velocity, double startAcceleration, double stopAcceleration);
    void moveBackward(double velocity, double startAcceleration, double stopAcceleration);
    void beginMoveBackward(double velocity, double startAcceleration, double stopAcceleration);
    void moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target);
    void moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target);

    void moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target);
    
    void moveIntoZero(double velocity, double startAcceleration, double stopAcceleration);
    void moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration);

    void stop();
  private:
    double getGearRatio() const;
    std::pair<double, double> getGearRatioAndOffset() const;
    uts::devtalk::CompletionWaitTimingPtr getDefaultTimeout() const;

    uts::devtalk::SerialStepMotorPrx motor;

    unsigned stepsPerRevolution;
    double motorReduction;
    double toothStep;
    double toothCount;
    double technologicalZero;

    mutable boost::mutex mutex;
  };

  typedef std::shared_ptr<StepMotor> StepMotorPtr;

}
