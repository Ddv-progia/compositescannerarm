/*
 * Core/Devices.cc
 */

#include <boost/format.hpp>
#include <DevTalk/Factory/AudioDataCollector.hh>
#include <DevTalk/Factory/SerialPort.hh>
#include <Ice/Communicator.h>

#include "Core/Devices.hh"

namespace {

  template<typename Proxy>
  Proxy getObject(const Ice::CommunicatorPtr& comm, const char* name)
  {
    return Proxy::checkedCast(comm->stringToProxy((boost::format("%1%@DevTalk-Composite") % name).str()));
  }

}

uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;
uts::devtalk::CoilPrx devices::coil;
devices::StepMotorPtr devices::xAxisMotor;
devices::StepMotorPtr devices::yAxisMotor;

void devices::setup(const DevicesConfiguration& conf, const Ice::CommunicatorPtr& comm, ObjectKeeper& objectKeeper)
{
  auto audioDataCollectorFactory = getObject<uts::devtalk::AudioDataCollectorFactoryPrx>(comm, "Factory/AudioDataCollector");
  audioDataCollector = audioDataCollectorFactory->getInstance(conf.audioDataCollector);
  auto adcs = audioDataCollectorFactory->getNames();

  auto serialPortFactory = getObject<uts::devtalk::SerialPortFactoryPrx>(comm, "Factory/WindowsSerialPort");

  auto motorPortConfig = new uts::devtalk::WindowsSerialPortConfig; 
  motorPortConfig->baudRate = 9600;
  motorPortConfig->dataBits = uts::devtalk::SerialPort::DataBits8;
  motorPortConfig->deviceName = conf.comPort;
  motorPortConfig->handshake = uts::devtalk::SerialPort::HandshakeNo;
  motorPortConfig->parity = uts::devtalk::SerialPort::ParityNone;
  motorPortConfig->stopBits = uts::devtalk::SerialPort::StopBits1;
  motorPortConfig->readIntervalTimeout = 4000;
  motorPortConfig->readTotalTimeoutConstant = 100;
  motorPortConfig->readTotalTimeoutMultiplier = 1;
  motorPortConfig->writeTotalTimeoutConstant = 0;
  motorPortConfig->writeTotalTimeoutMultiplier = 0;
  auto motorPort = serialPortFactory->make(motorPortConfig);
  objectKeeper.registerObject(motorPort);

  auto serialMotorFactory = getObject<uts::devtalk::SerialStepMotorFactoryPrx>(comm, "Factory/SerialStepMotor");
  
  uts::devtalk::SerialStepMotorConfig xcfg;
  xcfg.port = motorPort;
  xcfg.address = conf.xStepMotor.address;
  xcfg.protocolVersion = uts::devtalk::SerialDeviceProtocolVersion8;
  auto xAxisPrx = serialMotorFactory->make(xcfg);

  uts::devtalk::SerialStepMotorConfig ycfg;
  ycfg.port = motorPort;
  ycfg.address = conf.yStepMotor.address;
  ycfg.protocolVersion = uts::devtalk::SerialDeviceProtocolVersion8;
  auto yAxisPrx = serialMotorFactory->make(ycfg);
  
  xAxisMotor = std::make_shared<devices::StepMotor>(xAxisPrx, conf.xStepMotor.stepsPerRevolution, conf.xStepMotor.motorReduction, conf.xStepMotor.toothStep, conf.xStepMotor.toothCount);
  yAxisMotor = std::make_shared<devices::StepMotor>(yAxisPrx, conf.yStepMotor.stepsPerRevolution, conf.yStepMotor.motorReduction, conf.yStepMotor.toothStep, conf.yStepMotor.toothCount);
  objectKeeper.registerObject(xAxisPrx);
  objectKeeper.registerObject(yAxisPrx);

  auto coilFactory = getObject<uts::devtalk::CoilFactoryPrx>(comm, "Factory/Coil");
  coil = coilFactory->make(uts::devtalk::SerialDeviceProtocolVersion8, motorPort, conf.coil);
  objectKeeper.registerObject(coil);
}
