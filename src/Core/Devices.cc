/*
 * Core/Devices.cc
 */

#include <boost/format.hpp>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>  
#include <DevTalk/Factory/AudioDataCollector.hh>
#include <DevTalk/Factory/SerialPort.hh>
#include <DevTalk/LowLevelBoard/APLSystem.hh>
#include <DevTalk/Factory/UnitestAPLSystem.hh>
#include <DevTalk/Factory/UnitestAPLCoil.hh>
#include <DevTalk/Device/APLMultiDevice.hh>

//#include <DevTalkCAND/Plugin.hh>
//#include <DevTalk/Drivers/Unitest/APLMultiDevice.hh>

#include <Ice/Communicator.h>
#include <QtWidgets/QMessageBox>

#include "Core/Devices.hh"
#include <UCL/Ice/Create.hh>
#include <thread>

namespace {

  template<typename Proxy>
  Proxy getObject(const Ice::CommunicatorPtr& comm, const char* name)
  {
    return Proxy::checkedCast(comm->stringToProxy((boost::format("%1%@DevTalk-Composite") % name).str()));
  }

}

uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;
uts::devtalk::CoilPrx devices::coil;
::uts::devtalk::device::utscp::APLCoilPrx coilApl;
devices::StepMotorPtr devices::xAxisMotor;
devices::StepMotorPtr devices::yAxisMotor;
uts::devtalk::utscp::APLSystemPrx devices::aplSystemPrx;
uts::devtalk::utscp::APLSystemPrx devices::aplSystem1112Prx;
uts::devtalk::drivers::utscp::APLMultiDevicePrx devices::aplMultiDevicePrx;
uts::devtalk::drivers::utscp::APLMultiDevicePrx devices::aplMultiDevice1112Prx;

void devices::setup(const DevicesConfiguration& conf, const Ice::CommunicatorPtr& comm, ObjectKeeper& objectKeeper)

{
  auto audioDataCollectorFactory = getObject<uts::devtalk::AudioDataCollectorFactoryPrx>(comm, "Factory/AudioDataCollector");

  //*******
  //audioDataCollector = audioDataCollectorFactory->getInstance(conf.audioDataCollector); //дубляж?
  //auto adcs = audioDataCollectorFactory->getNames(); //дубляж?

  //"Factory/Unitest-APL-System"
  uts::devtalk::utscp::UnitestAPLSystemFactoryPrx aplMultiDeviceFactory;
  uts::devtalk::utscp::UnitestAPLCoilFactoryPrx coilFactory;
  //name = "APL-universal" address = "10.0.254.222" port = "1111" >
  //auto timeOut = uts::ice::create<uts::devtalk::TimeOut>();
  audioDataCollector = audioDataCollectorFactory->getInstance(conf.audioDataCollector);
  auto adcs = audioDataCollectorFactory->getNames();
  if (conf.coil == 2022) {  // Если версия катушки 2022, значит катушка управляется платой APL
      aplMultiDeviceFactory = getObject<uts::devtalk::utscp::UnitestAPLSystemFactoryPrx>(comm, "Factory/Unitest-APL-System");
      coilFactory = getObject<uts::devtalk::utscp::UnitestAPLCoilFactoryPrx>(comm, "Factory/APL-Coil");

      if (aplMultiDeviceFactory) {
          //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          aplSystemPrx = aplMultiDeviceFactory->make("APL-universal", "10.0.254.223", std::stoi("1111")); // временно! для работы разремарить
          //aplSystemPrx = aplMultiDeviceFactory->make("APL-universal", "10.0.254.223", std::stoi("1112"));
          //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          aplSystem1112Prx = aplMultiDeviceFactory->make("APL-universal", "10.0.254.223", std::stoi("1112"));
          //aplSystemPrx = aplSystem1112Prx;// временно! !!!! для работы ЗАРЕМАРИТЬ
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          aplMultiDevicePrx = uts::devtalk::drivers::utscp::APLMultiDevicePrx::checkedCast(aplSystemPrx, "APL-Multi-Device");
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (!aplMultiDevicePrx)
              throw std::exception("Невозможно получить driver APLMultiDevicePrx");
          aplMultiDevice1112Prx = uts::devtalk::drivers::utscp::APLMultiDevicePrx::checkedCast(aplSystem1112Prx, "APL-Multi-Device");
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (!aplMultiDevice1112Prx)
              throw std::exception("Невозможно получить driver aplMultiDevice1112Prx");
            
          try{
              if (coilFactory) { 
                  coil = coilFactory->makeCoil("1", aplMultiDevicePrx);
                  objectKeeper.registerObject(coil);
              }

              coilApl = coilFactory->make("1", aplMultiDevicePrx);
              objectKeeper.registerObject(coilApl);

              //////auto t2 = uts::ice::create<uts::devtalk::TimeOut>(1000.0, 2000.0);
              //////auto timePtr(uts::devtalk::ToT().timeOut());
              ////auto timeOut = uts::ice::create<uts::devtalk::CompletionWaitTiming>(2.0, 3.0, IceUtil::None);
              //////////auto t(uts::devtalk::ToT(2, 1).timeOut());
              //auto state2 = coilApl->getState();
              //  auto str2 = "getState = "+std::to_string(state2.digitalInputs);
              //QMessageBox::information(nullptr, "info", str2.c_str());
              ////coilApl->switchWorkingMode(timeOut);
              //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              //auto ver2 = coilApl->getFirmwareVersion();
              //auto str3 = "getFirmwareVersion = "+std::to_string(ver2.day) + "." + std::to_string(ver2.month) + "." + std::to_string(ver2.year);
              //QMessageBox::information(nullptr, "info", str3.c_str());
              //coilApl->stop();
          }
          catch (uts::devtalk::CommunicationException& exc) {
              QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
          }
          catch (Ice::Exception& exc) {
              QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.what()));
          }

      }
  }
  else {
      auto coilFactory = getObject<uts::devtalk::CoilFactoryPrx>(comm, "Factory/Coil");
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
      try {
          auto motorPort = serialPortFactory->make(motorPortConfig);
          objectKeeper.registerObject(motorPort);

          coil = coilFactory->make(uts::devtalk::SerialDeviceProtocolVersion8, motorPort, conf.coil);
          objectKeeper.registerObject(coil);
          //uts::devtalk::SerialPortFactoryPrx serialPortFactory;
          //serialPortFactory = getObject<uts::devtalk::SerialPortFactoryPrx>(comm, "Factory/WindowsSerialPort");

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
      }
      catch (uts::devtalk::CommunicationException& exc) {
          QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
      }
      catch (Ice::Exception& exc) {
          QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.what()));
      }


  }
  //*******

  bool simulate = false; //*******
  //if (!ctx.simulate) {
  if (!simulate) {       //*******
      //auto mk = [=]() -> uts::devtalk::utscp::APLSystemPrx {
      //    //auto factory = getObjectChecked<uts::devtalk::utscp::UnitestAPLSystemFactoryPrx>(ctx.node, factoryName);
      //    auto factory = getObjectChecked<uts::devtalk::utscp::UnitestAPLSystemFactoryPrx>(comm, "Factory/Unitest-APL-System");
      //    auto obj = factory->make(nameAPLSystem, addressString, std::stoi(portString));
      //    ctx.objectKeeper->registerObject(obj);
      //    return obj;
      //};
      //ctx.proxyRegistry->addLazy(nameAPLSystem, mk);

      //DEVTALK_CAND_LOG_CTX(info) << "Добавлено устройство Unitest-APL-System " << nameAPLSystem;

      //for (auto& fel : el.children("Facet")) {
      //    auto facetName = getAttributeValue(fel, "facet-name");
      //    auto objectName = getAttributeValue(fel, "object-name");
      //    auto mkFacet = [=]() {
      //        auto obj = ctx.proxyRegistry->get(nameAPLSystem);
      //        return Ice::ObjectPrx::checkedCast(obj, facetName);
      //    };
      //    ctx.proxyRegistry->addLazy(nameAPLSystem + "/" + objectName, mkFacet);
      //    DEVTALK_CAND_LOG_CTX(info) << "Добавлен фасад " << facetName << " для устройства Unitest-APL-System " << nameAPLSystem << ": " << objectName;
      //}
  }

  if (aplSystem1112Prx && aplMultiDevicePrx)
  {
      //*******
      std::string str;
      //////auto otvet3 = aplSystem1112Prx->runCommand("gyro");
      ////auto otvet3 = aplSystem1112Prx->runCommand("logstart 4 10000 10");
      //auto otvet3 = aplSystem1112Prx->runCommand("help 4");
      //str = std::to_string(otvet3) + " ";
      //QString str3 = QString::fromUtf8(str.c_str());
      //QMessageBox::information(nullptr, "logstart ", str3);

      try {
          auto timeOut2 = uts::ice::create<uts::devtalk::TimeOut>(1000.0, 2000.0);
          auto otvet4 = aplMultiDevicePrx->getEncoders("1", timeOut2);
          str = "";
          for (auto i = 0; i < otvet4.size(); i++) {
              str += std::to_string(otvet4[i]) + " ";
          }
          QString str4 = QString::fromUtf8(str.c_str());
          QMessageBox::information(nullptr, "getEncoders ", str4);

          auto otvet5 = aplMultiDevicePrx->getGyro("1", timeOut2);
          str = "";
          for (auto i = 0; i < otvet5.size(); i++) {
              str += std::to_string(otvet5[i]) + " ";
          }
          QString str5 = QString::fromUtf8(str.c_str());
          QMessageBox::information(nullptr, "getGyro ", str5);

          auto timeOut3 = uts::ice::create<uts::devtalk::TimeOut>(1000.0, 5000.0);
          //auto otvet6 = aplMultiDevice1112Prx->logStart("1",10000, 10, timeOut3);
          auto otvet6 = aplMultiDevicePrx->logStart("", 10000, 10, timeOut3);
          str = "";
          QString str6 = "";
          for (auto i = 0; i < otvet6.size(); i++) {
              str = "";
              for (auto j = 0; j < otvet6[i].size(); j++) {
                  str += std::to_string(otvet6[i][j]) + " ";
              }
              str += "\r\n";
              str6 += QString::fromUtf8(str.c_str());
          }
          QMessageBox::information(nullptr, "logStart ", str6);
      }
      catch (uts::devtalk::CommunicationException& exc) {
          QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
      }
      catch (Ice::Exception& exc) {
          QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.what()));
      }

      //*******
  }
}
