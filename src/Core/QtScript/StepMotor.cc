/*
 * Core/QtScript/StepMotor.cc
 */

#include "Core/QtScript/StepMotor.hh"
#include <QtCore/QString>
#include <QtWidgets/QMessageBox>
#include <boost/thread.hpp>
//#include "../../../../../vcpkg/installed/x64-windows/include/boost/thread/detail/thread_group.hpp"

#include <chrono>
#include <mutex>
#include <thread>


script::StepMotor::StepMotor(const devices::StepMotorPtr& motor, QJSEngine* scriptEngine)
  : motor(motor), scriptEngine(scriptEngine)
{
    addMotor(motor);
}

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
double script::StepMotor::valWrapExceptions(F thunk)
{
  try {
      return thunk();
  } catch (uts::devtalk::IncorrectArgumentException& e) {
    scriptEngine->throwError(QString::fromUtf8(e.description.c_str()));
  } catch (uts::devtalk::CommunicationException& e) {
    scriptEngine->throwError(QString::fromUtf8(e.reason.c_str()));
  } catch (Ice::Exception& e) {
    scriptEngine->throwError(QString::fromUtf8(e.what()));
  }
  //QMessageBox::critical(this, QString::fromStdString("Ошибка/n"), QString::fromUtf8(e.what()));
  return NAN;
}

double script::StepMotor::getMachineCoordinate(devices::StepMotorPtr motorIn)
{
  devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);

  //return valWrapExceptions([=] () -> QString { return QString::number(motor->getMachineCoordinate(), 'f', 3);  });
  return valWrapExceptions([=] () { return motor->getMachineCoordinate();  });
}

double script::StepMotor::getTechnologicalCoordinate(devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    return valWrapExceptions([=] () { return motor->getTechnologicalCoordinate(); });
}

void script::StepMotor::moveForward(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveForward(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveBackward(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveBackward(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveToMachine(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveToMachine(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveRelative(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveRelative(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveToTechnological(double velocity, double startAcceleration, double stopAcceleration, double target, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveToTechnological(velocity, startAcceleration, stopAcceleration, target); });
}

void script::StepMotor::moveIntoZero(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveIntoZero(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::moveOutOfZero(double velocity, double startAcceleration, double stopAcceleration, devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->moveOutOfZero(velocity, startAcceleration, stopAcceleration); });
}

void script::StepMotor::stop(devices::StepMotorPtr motorIn)
{
    devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    wrapExceptions([=] () { motor->stop(); });
}

std::size_t script::StepMotor::addMotor(const devices::StepMotorPtr& motor)
{
  std::size_t index = motors.size();
  motors.push_back(motor);
  return index;
}

devices::StepMotorPtr script::StepMotor::motorsAt(int index)
{
    return motors.at(index);
}

/// <summary>
/// данный StepMotor - по оси X. motors[1] - по оси Y. motors[2] - по оси Z. 
/// 
/// </summary>
void script::StepMotor::moveXYZ(double velocity, double startAcceleration, double stopAcceleration, std::vector<double> target)

{
    std::size_t threadNum = motors.size();
    std::size_t targetNum = target.size();
    std::vector<std::thread >threadArr;
    if (targetNum < 1) return;
    //devices::StepMotorPtr motor = (motorIn == nullptr ? this->motor : motorIn);
    std::vector<double> start_coord;
    double velocityLocal;
    double startAccelerationLocal;
    double stopAccelerationLocal;
    double way = 0.0;
    std::vector<double> wayArray;
    std::size_t count = std::min(threadNum, targetNum);
    for (int i = 0; i < count; i++) {
        double coord = this->getTechnologicalCoordinate(motors.at(i));
        start_coord.push_back(coord);
        double diff = target.at(i) - coord;
        way += diff* diff;
        wayArray.push_back(diff);
    }
    way = std::sqrt(way);
    boost::thread_group g;
    //g.create_thread(boost::bind(&t2));
    // плодите потоков сколько хочется, можно в цикле
    //g.join_all();
    std::mutex threadMtx;
    std::lock_guard<std::mutex> lockGuard(threadMtx);

    for (int i = 0; i < count; i++) {
        if (target[i] != start_coord[i]) {
            std::chrono::milliseconds interval((count-i)*100);

            velocityLocal = std::abs(wayArray.at(i)) * velocity / way;
            startAccelerationLocal = std::abs(wayArray.at(i)) * startAcceleration / way;
            stopAccelerationLocal = std::abs(wayArray.at(i)) * stopAcceleration / way;
//            x.nativeMotor.moveToTechnological(velocity, acceleration, acceleration, target_x);
            g.create_thread([=]() {
                std::this_thread::sleep_for(interval);
                ////std::lock_guard<std::mutex> lockGuard(threadMtx);
                motors.at(i)->moveToTechnological(velocityLocal, startAccelerationLocal, stopAccelerationLocal, target[i]);
                /*wrapExceptions([=]() { motors.at(i)->moveToTechnological(velocity, startAcceleration, stopAcceleration, target[i]); });*/
                });

            //threadArr.push_back(std::thread([=]() {
            //    std::lock_guard<std::mutex> lockGuard(threadMtx);
            //    wrapExceptions([=]() { motors.at(i)->moveToTechnological(velocity, startAcceleration, stopAcceleration, target[i]); });
            //    }));
            //if (threadArr.back().joinable())
            //    threadArr.back().join();
//            t.detach();
        }
    }
    g.join_all();
}


