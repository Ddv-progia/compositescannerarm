/*
 * Core/Devices.cc
 */
#include "Core/Devices.hh"
#include <boost/format.hpp>
#include <thread>
#include <chrono>  
#include <DevTalk/Factory/AudioDataCollector.hh>
#include <DevTalk/LowLevelBoard/APLSystem.hh>



//#include <DevTalkCAND/Plugin.hh>
//#include <DevTalk/Drivers/Unitest/APLMultiDevice.hh>

#include <Ice/Communicator.h>
#include <QtWidgets/QMessageBox>


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
uts::devtalk::device::utscp::APLCoilPrx  devices::coile;
uts::devtalk::utscp::APLSystemPrx devices::aplSystemPrx;
uts::devtalk::utscp::APLSystemPrx devices::aplSystem1112Prx;
uts::devtalk::drivers::utscp::APLMultiDevicePrx devices::aplMultiDevicePrx;
uts::devtalk::drivers::utscp::APLMultiDevicePrx devices::aplMultiDevice1112Prx;

void devices::setup(const DevicesConfiguration& conf, const Ice::CommunicatorPtr& comm, ObjectKeeper& objectKeeper)
{
    auto audioDataCollectorFactory = getObject<uts::devtalk::AudioDataCollectorFactoryPrx>(comm, "Factory/AudioDataCollector");
    audioDataCollector = audioDataCollectorFactory->getInstance(conf.audioDataCollector);
    auto adcs = audioDataCollectorFactory->getNames();

    QMessageBox::critical(0, "Ошибка", QString::fromStdString(adcs.back()));
    uts::devtalk::utscp::UnitestAPLSystemFactoryPrx aplMultiDeviceFactory = getObject<uts::devtalk::utscp::UnitestAPLSystemFactoryPrx>(comm, "Factory/Unitest-APL-System");
    uts::devtalk::utscp::UnitestAPLCoilFactoryPrx coilFactory = coilFactory = getObject<uts::devtalk::utscp::UnitestAPLCoilFactoryPrx>(comm, "Factory/APL-Coil");

    if (aplMultiDeviceFactory) {
        aplSystemPrx = aplMultiDeviceFactory->make("APL-universal", "10.0.254.223", 1111);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        aplSystem1112Prx = aplMultiDeviceFactory->make("APL-universal", "10.0.254.223", 1112);
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
                coile = coilFactory->make("1", aplMultiDevicePrx);
                objectKeeper.registerObject(coile);
            }

        }
        catch (uts::devtalk::CommunicationException& exc) {
            QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
        }
        catch (Ice::Exception& exc) {
            QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.what()));
        }

    }

  bool simulate = false; 
  //if (!ctx.simulate) {
  if (!simulate) {
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
