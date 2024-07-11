#include "Gui/AutoScanWindow.hh"


bool AutoScanWindow::allFieldsFilled()
{
  return !(this->sampleRateEdit->text().isEmpty() ||
  this->xVelocityEdit->text().isEmpty() ||
  this->yVelocityEdit->text().isEmpty() ||
  this->xAccelerationEdit->text().isEmpty() ||
  this->yAccelerationEdit->text().isEmpty() ||
  this->xBeginEdit->text().isEmpty() ||
  this->yBeginEdit->text().isEmpty() ||
  this->xEndEdit->text().isEmpty() ||
  this->yEndEdit->text().isEmpty() ||
  this->halfPeriodEdit->text().isEmpty() || 
  this->workingHalfPeriodEdit->text().isEmpty() ||
  this->initialHalfPeriodEdit->text().isEmpty() ||
  this->minimumLevelEdit->text().isEmpty() ||
  this->yStepEdit->text().isEmpty());
}

AutoScanParameters AutoScanWindow::parameters()
 {
  AutoScanParameters params;
  params.isXBack = this->xBackBox->isChecked();
  params.shouldReturn = this->returnBox->isChecked();

  params.sampleRate = this->sampleRateEdit->text().toInt();

  params.xMotorVelocity = this->xVelocityEdit->text().toInt();
  params.yMotorVelocity = this->yVelocityEdit->text().toInt();
  params.xMotorAcceleration = this->xAccelerationEdit->text().toInt();
  params.yMotorAcceleration = this->yAccelerationEdit->text().toInt();
  params.step = this->yStepEdit->text().toInt();

  params.xStartPoint = this->xBeginEdit->text().toInt();
  params.yStartPoint = this->yBeginEdit->text().toInt();
  params.xEndPoint = this->xEndEdit->text().toInt();
  params.yEndPoint = this->yEndEdit->text().toInt();

  params.halfPeriod = this->halfPeriodEdit->text().toInt();
  params.initialHalfPeriod = this->initialHalfPeriodEdit->text().toInt();
  params.workingHalfPeriod = this->workingHalfPeriodEdit->text().toInt();
  params.minimumLevel = this->minimumLevelEdit->text().toInt();

  params.dirrection = this->dirrectionCombo->currentText();
  params.mode = this->modeCombo->currentText();

  return params;
}
AutoScanWindow::AutoScanWindow(QString fileName,int xCurrent, int yCurrent, QWidget* parent):scanScriptFilename(fileName),QDialog(parent)
{ 
  setupUi(this);

  this->sampleRateEdit->setValidator(new QIntValidator(0,100000));
  this->xVelocityEdit->setValidator(new QIntValidator(0,200));
  this->yVelocityEdit->setValidator(new QIntValidator(0,200));
  this->xAccelerationEdit->setValidator(new QIntValidator(0,200));
  this->yAccelerationEdit->setValidator(new QIntValidator(0,200));
  this->yStepEdit->setValidator(new QIntValidator(0,100));
  this->xBeginEdit->setValidator(new QIntValidator);
  this->yBeginEdit->setValidator(new QIntValidator);
  this->xEndEdit->setValidator(new QIntValidator);
  this->yEndEdit->setValidator(new QIntValidator);

  this->xBeginEdit->setText(QString::number(xCurrent));
  this->yBeginEdit->setText(QString::number(yCurrent));
  this->settingsFile = "SettingsForAutoScanWindow.ini";
  this->settings = new QSettings(settingsFile, QSettings::IniFormat);

  connect(this->scanButton,SIGNAL(pressed()),SLOT(startScan()));
  connect(this->stopButton,SIGNAL(pressed()),SLOT(stopScan()));

}

Q_SLOT void AutoScanWindow::startScan()
{
  if(allFieldsFilled()){
    auto params = parameters();
    //QScriptValue scriptParams;

    //scriptParams.setProperty(QString::fromUtf8("xBack"),QScriptValue(params.isXBack));
    //scriptParams.setProperty(QString::fromUtf8("xStartPoint"),QScriptValue(params.xStartPoint));
    //scriptParams.setProperty(QString::fromUtf8("yStartPoint"),QScriptValue(params.yStartPoint));
    //scriptParams.setProperty(QString::fromUtf8("xEndPoint"), QScriptValue(params.xEndPoint));
    //scriptParams.setProperty(QString::fromUtf8("yEndPoint"), QScriptValue(params.yEndPoint));
    //scriptParams.setProperty(QString::fromUtf8("sampleRate"), QScriptValue(params.sampleRate));
    //scriptParams.setProperty(QString::fromUtf8("xVelocity"), QScriptValue(params.xMotorVelocity));
    //scriptParams.setProperty(QString::fromUtf8("yVelocity"), QScriptValue(params.yMotorVelocity));
    //scriptParams.setProperty(QString::fromUtf8("xAcceleration"), QScriptValue(params.xMotorAcceleration));
    //scriptParams.setProperty(QString::fromUtf8("yAcceleration"), QScriptValue(params.yMotorAcceleration));
    //scriptParams.setProperty(QString::fromUtf8("yStep"), QScriptValue(params.step));
    //scriptParams.setProperty(QString::fromUtf8("shouldReturn"), QScriptValue(params.shouldReturn));
    //scriptParams.setProperty(QString::fromUtf8("halfPeriod"), QScriptValue(params.halfPeriod));
    //scriptParams.setProperty(QString::fromUtf8("initialHalfPeriod"), QScriptValue(params.initialHalfPeriod));
    //scriptParams.setProperty(QString::fromUtf8("workingHalfPeriod"), QScriptValue(params.workingHalfPeriod));
    //scriptParams.setProperty(QString::fromUtf8("minimumLevel"), QScriptValue(params.minimumLevel));
    //scriptParams.setProperty(QString::fromUtf8("dirrection"), QScriptValue(params.dirrection));
    //scriptParams.setProperty(QString::fromUtf8("mode"), QScriptValue(params.mode));
    //this->settings->beginGroup("scriptParams");
    this->settings->setValue(QString::fromUtf8("xBack"), params.isXBack);
    this->settings->setValue(QString::fromUtf8("xStartPoint"), params.xStartPoint);
    this->settings->setValue(QString::fromUtf8("yStartPoint"), params.yStartPoint);
    this->settings->setValue(QString::fromUtf8("xEndPoint"), params.xEndPoint);
    this->settings->setValue(QString::fromUtf8("yEndPoint"), params.yEndPoint);
    this->settings->setValue(QString::fromUtf8("sampleRate"), params.sampleRate);
    this->settings->setValue(QString::fromUtf8("xVelocity"), params.xMotorVelocity);
    this->settings->setValue(QString::fromUtf8("yVelocity"), params.yMotorVelocity);
    this->settings->setValue(QString::fromUtf8("xAcceleration"), params.xMotorAcceleration);
    this->settings->setValue(QString::fromUtf8("yAcceleration"), params.yMotorAcceleration);
    this->settings->setValue(QString::fromUtf8("yStep"), params.step);
    this->settings->setValue(QString::fromUtf8("shouldReturn"), params.shouldReturn);

    this->settings->setValue(QString::fromUtf8("halfPeriod"),params.halfPeriod);
    this->settings->setValue(QString::fromUtf8("initialHalfPeriod"),params.initialHalfPeriod);
    this->settings->setValue(QString::fromUtf8("workingHalfPeriod"),params.workingHalfPeriod);
    this->settings->setValue(QString::fromUtf8("minimumLevel"),params.minimumLevel);
    this->settings->setValue(QString::fromUtf8("dirrection"),params.dirrection);
    this->settings->setValue(QString::fromUtf8("mode"),params.mode);
    //this->settings->endGroup();
    //emit setProperty(QString::fromUtf8("autoScanParams"),scriptParams);
    emit setProperty(QString::fromUtf8("autoScanParams"), QString::fromUtf8("scriptParams"));

    QFile script(scanScriptFilename);
    if (script.open(QIODevice::ReadOnly)) {
      emit startScanScript(QString::fromUtf8(script.readAll().data()), scanScriptFilename, false);
    }
  }
  else QMessageBox::warning(this,QString("Внимание"),QString("Не все поля заполнены. Заполните поля и повторите попытку"));
}
Q_SLOT void AutoScanWindow::stopScan()
{
  emit stop();
}