/*
 * main.cc
 */

#include <boost/thread/thread.hpp>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>
//#include <QtCore/QTextCodec>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <UCL/PlotView/Init.hh>
#include "Core/CommonXmlLoad.hh"

#include <mfapi.h>
#include <ObjBase.h>

#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ConfigurationLocator.hh"
#include "Core/Devices.hh"
#include "Core/DevicesConfigurationReflection.hh"
#include "Core/InitializeMediaFoundationTask.hh"
#include "Core/ObjectKeeper.hh"
#include "Core/ScanFactory.hh"
#include "Gui/MainWindow.hh"
#include "Core/PersistentVariable.hh"
#include <UCL/Customization/FromVariable/String.hh>




int main(int argc, char* argv[])
{
  BackgroundTaskExecutor bte;
  boost::thread backgroundTasksThread(boost::ref(bte));

  ScanFactory scanFactory(bte);
  MFStartup(MF_VERSION, MFSTARTUP_FULL);
  bte.enqueue(new InitializeMediaFoundationTask());

  qRegisterMetaType<std::shared_ptr<Scan>>("std::shared_ptr<Scan>");
  qRegisterMetaType<std::shared_ptr<Scan>>("std::vector<float>");
  qRegisterMetaType<RangeScanLine>("RangeScanLine");
  qRegisterMetaType<SourceScanLine>("SourceScanLine");
  uts::plotting::initialize();

  QApplication app(argc, argv);

  std::unique_ptr<ObjectKeeper> objectKeeper(new ObjectKeeper);
  objectKeeper->start();


  Ice::InitializationData initData;
  initData.properties = Ice::createProperties();
  initData.properties->setProperty("Ice.ImplicitContext", "Shared");
  initData.properties->setProperty("Ice.MessageSizeMax", "500000000");
  initData.properties->setProperty("Ice.Default.Locator", "Composite-Scanner-Arm-Grid/Locator:tcp -h localhost -p 4062");
  auto communicator = Ice::initialize(argc, argv, initData);
  
  try {
    Configuration::init(argc, argv);
    QString path = Configuration::getConfigurationPathname("etc/Devices-Configuration.xml");
    PersistentVariable<DevicesConfiguration> conf(path.toStdString(), "Devices-Configuration");
    conf.load();
    devices::setup(*conf, communicator, *objectKeeper);

  } catch (uts::devtalk::CommunicationException& exc) {
    QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.reason.c_str()));
  } catch (Ice::Exception& exc) {
    QMessageBox::critical(0, "Ошибка", QString::fromUtf8(exc.what()));
  }
  //QFile file(Configuration::getConfigurationPathname("etc/style.css"));
  //if (file.exists())
      //if (file.open(QFile::ReadOnly))
          //app.setStyleSheet(QLatin1String(file.readAll()));
  realtime::RTContext rtCtxt(communicator);
  MainWindow mw(rtCtxt, bte, scanFactory);
  mw.show();
  app.exec();

  objectKeeper->stop();
  uts::plotting::uninitialize();

  communicator->shutdown();
  communicator->waitForShutdown();
  communicator->destroy();
  
  backgroundTasksThread.interrupt();
  if(backgroundTasksThread.joinable())
    backgroundTasksThread.join();

  MFShutdown();
}
