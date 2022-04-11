/*
 * Core/QtScript/StepMotor.cc
 */

#include <boost/chrono.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>

#include "Core/StepMotor.hh"

using uts::devtalk::VelocityTraits;

devices::StepMotor::StepMotor(const uts::devtalk::SerialStepMotorPrx& motor,
                             unsigned stepsPerRevolution,
                             double motorReduction,
	                     double toothStep,
	                     double toothCount)
  : motor(motor), stepsPerRevolution(stepsPerRevolution), motorReduction(motorReduction), toothStep(toothStep), toothCount(toothCount), technologicalZero(0)
{ }

double devices::StepMotor::getMachineCoordinate() const
{
  return double(motor->getCoordinate()) * getGearRatio();
}

double devices::StepMotor::getTechnologicalCoordinate() const
{
  double ratio, zero;
  std::tie(ratio, zero) = getGearRatioAndOffset();
  return double(motor->getCoordinate()) * ratio - zero;
}

void devices::StepMotor::setTechnologicalZero()
{
  setTechnologicalZero(getMachineCoordinate());
}

void devices::StepMotor::setTechnologicalZero(double newTechnologicalZero)
{
  boost::lock_guard<boost::mutex> lock(mutex);
  technologicalZero = newTechnologicalZero;
}

bool devices::StepMotor::isMoving()
{
  return motor->getStatus().moving;
}

void devices::StepMotor::waitStop(int pollPeriod)
{
  while (motor->getStatus().moving)
    boost::this_thread::sleep_for(boost::chrono::milliseconds(pollPeriod));
}

void devices::StepMotor::moveForward(double velocity, double startAcceleration, double stopAcceleration)
{

  double ratio = getGearRatio();
  motor->moveForward(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                        static_cast<Ice::Int>(startAcceleration / ratio), 
                                        static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());
}

void devices::StepMotor::beginMoveForward(double velocity, double startAcceleration, double stopAcceleration)
{
  double ratio = getGearRatio();
  motor->begin_moveForward(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio),
    static_cast<Ice::Int>(startAcceleration / ratio),
    static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());
}

void devices::StepMotor::moveBackward(double velocity, double startAcceleration, double stopAcceleration)
{
  double ratio = getGearRatio();
  motor->moveBackward(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                         static_cast<Ice::Int>(startAcceleration / ratio), 
                                         static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());
}

void devices::StepMotor::beginMoveBackward(double velocity, double startAcceleration, double stopAcceleration)
{
  double ratio = getGearRatio();
  motor->begin_moveBackward(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio),
    static_cast<Ice::Int>(startAcceleration / ratio),
    static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());
}

void devices::StepMotor::moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  double ratio = getGearRatio();
  motor->moveTo(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                   static_cast<Ice::Int>(startAcceleration / ratio), 
                                   static_cast<Ice::Int>(stopAcceleration / ratio)),
                                   static_cast<Ice::Int>(target / ratio), IceUtil::None);
}

void devices::StepMotor::moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  double ratio = getGearRatio();
  motor->moveRelative(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                         static_cast<Ice::Int>(startAcceleration / ratio), 
                                         static_cast<Ice::Int>(stopAcceleration / ratio)),
                                         static_cast<Ice::Int>(target / ratio), IceUtil::None);
}

void devices::StepMotor::moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target)
{
  double ratio, zero;
  std::tie(ratio, zero) = getGearRatioAndOffset();
  motor->moveTo(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                   static_cast<Ice::Int>(startAcceleration / ratio), 
                                   static_cast<Ice::Int>(stopAcceleration / ratio)), 
                                   static_cast<Ice::Int>(target / ratio + zero), IceUtil::None);
}

void devices::StepMotor::moveIntoZero(double velocity, double startAcceleration, double stopAcceleration)
{
  double ratio = getGearRatio();
  motor->moveIntoZero(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                         static_cast<Ice::Int>(startAcceleration / ratio), 
                                         static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());

}

void devices::StepMotor::moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration)
{
  double ratio = getGearRatio();
  motor->moveOutOfZero(new VelocityTraits(static_cast<Ice::Int>(velocity / ratio), 
                                          static_cast<Ice::Int>(startAcceleration / ratio), 
                                          static_cast<Ice::Int>(stopAcceleration / ratio)), getDefaultTimeout());
}

void devices::StepMotor::stop()
{
  motor->stop();
}

double devices::StepMotor::getGearRatio() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return toothStep * toothCount / (motorReduction * double(stepsPerRevolution));
}

std::pair<double, double> devices::StepMotor::getGearRatioAndOffset() const
{
  boost::lock_guard<boost::mutex> lock(mutex);
  return std::make_pair(toothStep * toothCount / (motorReduction * double(stepsPerRevolution)), technologicalZero);
}

uts::devtalk::CompletionWaitTimingPtr
devices::StepMotor::getDefaultTimeout() const
{
  return new uts::devtalk::CompletionWaitTiming(100, 1, 0.5);
}
