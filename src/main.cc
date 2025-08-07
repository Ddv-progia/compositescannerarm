/*
 * main.cc
 */
#include <windows.h>
#include <shellapi.h>
#include <chrono>
#include <thread>

#include <boost/thread/thread.hpp>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QMutex>
//#include <QtCore/QTextCodec>

#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

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
#include <QtCore/QObject>

#include "Core/ScanFactory.hh"
#include "Gui/MainWindow.hh"
#include "Core/PersistentVariable.hh"
#include <UCL/Customization/FromVariable/String.hh>


void __cdecl ThreadFunc(void* str)
{
    auto CommandResult = L"";
    bool Result = false;
    bool IsRunning = false;
    DWORD retSize;
    LPTSTR pTemp = NULL;
    //std::string* strStr = static_cast<std::string*>(str);
    QString* strStr = static_cast<QString*>(str);
    QString s = "/r "+ * strStr;
    //std::string* strStr = static_cast<std::string*>(str);
    //std::string s = "/ r "+ * strStr;
    LPCWSTR cmndStr;
    ////cmndStr = s.c_str();
    //cmndStr = reinterpret_cast<LPCWSTR>(s.c_str());
    cmndStr = reinterpret_cast<LPCWSTR>(s.utf16());
    
    //Result = (BOOL)ShellExecute(GetActiveWindow(), L"OPEN", L"cmd", L"/r d:\\Composite-Scanner-Arm\\icegridnode-start.cmd", NULL, SW_SHOWMINIMIZED);
    Result = (BOOL)ShellExecute(GetActiveWindow(), L"OPEN", L"cmd", cmndStr, NULL, SW_SHOWMINIMIZED);

    if (Result)
    {
        IsRunning = TRUE;

        //while (IsRunning)
        //{
        //    if (CheckCommandExecutionStatus())
        //    {
        //        break;
        //    }
        //}
    }
    else
    {
        retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
            NULL,
            GetLastError(),
            LANG_NEUTRAL,
            (LPTSTR)&pTemp,
            0,
            NULL);
        MessageBox(NULL, pTemp, L"Error", MB_OK);
    }

    IsRunning = FALSE;
    _endthreadex(0);
}


int main(int argc, char* argv[])
{


    BOOL IsRunning = FALSE;
    QString pathCmd;
    try {
        Configuration::init(argc, argv);
        pathCmd = Configuration::getConfigurationPathname("icegridnode-start.cmd");

    }
    catch (...) {
        QMessageBox::critical(0, "Ошибка при получении каталога конфигурации", "Проверьте наличие файла \"icegridnode - start.cmd\" \r в каталоге конфигурации");
    }

    //std::string s;
    //QString s;
    //s = "d:\\Composite-Scanner-Arm\\icegridnode-start.cmd";
    //HANDLE hThread = (HANDLE)_beginthread(ThreadFunc, 0, &s);
    HANDLE hThread = (HANDLE)_beginthread(ThreadFunc, 0, &pathCmd);

    //Result = (BOOL)ShellExecute(GetActiveWindow(), L"OPEN", L"cmd", L"d:\\Composite-Scanner-Arm\\icegridnode-start.cmd", NULL, SW_SHOWNORMAL);
    //ShellExecute(0, L"open", L"cmd.exe", x, 0, SW_HIDE);

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    int timeToSleepInSecond (10);
    //std::this_thread::sleep_for(std::chrono::seconds(timeToSleepInSecond));
    QMutex mutex;
    QWidget* progress;
    progress = new QWidget();
    progress->setWindowTitle("Запуск сервисных служб");
    int pbMaximum = 100;
    QProgressBar* pb;
    pb = new QProgressBar();
    pb->setRange(0, pbMaximum);
    pb->setMinimumWidth(200);
    pb->setAlignment(Qt::AlignCenter);
    QPushButton* pdSkip = new QPushButton("&Пропустить");
    QTimer* timer;
    timer = new QTimer();
    bool continueApp = false;
    QObject::connect(timer, &QTimer::timeout, 
        [pb, pdSkip, timer , progress, &continueApp, pbMaximum]() {
            //if (pb->value() == 0) mutex.lock();
                auto val = pb->value()+1;
                pb->setValue(val);
                if (pb->value() == pbMaximum) {
                    continueApp = true;
                    //mutex.unlock();
                    timer->stop();
                }    
                else {
                    timer->start();
                }
        });

    QObject::connect(pdSkip, &QPushButton::clicked, [&continueApp, timer, progress]() {
        continueApp = true;
/*        timer->stop();
        progress->close();*/ });

    QHBoxLayout* phbL = new QHBoxLayout;
    phbL->addWidget(pb);
    phbL->addWidget(pdSkip);
    progress->setLayout(phbL);

    timer->start(std::chrono::milliseconds (timeToSleepInSecond * 1000 / pbMaximum));
    progress->show();
    while (!continueApp)
    {
        app.processEvents();
    }
    emit pdSkip->clicked();
    progress->close();
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
    QString path = Configuration::getConfigurationPathname("etc\\Devices-Configuration.xml");
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
