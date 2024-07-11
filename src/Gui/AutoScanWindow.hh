#include "ui_AutoScanWindow.h"
#include "Core/ScriptExecutor.hh"

#include <QDialog>
#include <QFile>
#include <QMessageBox>
#include "ui_AutoScanWindow.h"
#include "Core\ScriptExecutor.hh"

#include <QSettings>

//QSettings* settings;
//settings = new QSettings(settingsFile, QSettings::IniFormat);
//
//auto isNeedToUseAngleFromPolydata = settings->value(USE_ANGLE_FROM_POLYDATA, VALUE_USE_ANGLE_FROM_POLYDATA).toBool();
//

struct AutoScanParameters
{
  int xMotorVelocity;
  int yMotorVelocity;
  int xMotorAcceleration;
  int yMotorAcceleration;

  int xStartPoint;
  int yStartPoint;
  int xEndPoint;
  int yEndPoint;
  int step;

  int halfPeriod;
  int initialHalfPeriod;
  int workingHalfPeriod;
  int minimumLevel;
  QString mode;
  QString dirrection;

  int sampleRate;
  bool isXBack;
  bool shouldReturn;
};


class AutoScanWindow:public QDialog, public Ui::AutoScanDialog
{
  Q_OBJECT
  QString scanScriptFilename;
  bool allFieldsFilled();
  AutoScanParameters parameters();
  QString settingsFile;
public:
  AutoScanWindow(QString fileName,int xCurrent, int yCurrent, QWidget* parent = 0);
  Q_SLOT void startScan();
  Q_SLOT void stopScan();

  Q_SIGNAL void stop();
  Q_SIGNAL void startScanScript(const QString&,const QString&,bool);
  //Q_SIGNAL void setProperty(const QString&,const QScriptValue&);
  Q_SIGNAL void setProperty(const QString&, const QString&);
  QSettings* settings;
  //settings = new QSettings(settingsFile, QSettings::IniFormat);
  //auto isNeedToUseAngleFromPolydata = settings->value(USE_ANGLE_FROM_POLYDATA, VALUE_USE_ANGLE_FROM_POLYDATA).toBool();

};


