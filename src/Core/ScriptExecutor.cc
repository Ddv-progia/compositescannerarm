/*
 * Core/ScriptExecutor.cc
 */

#include "Core/Devices.hh"
#include "Core/ScriptExecutor.hh"

ScriptExecutor::ScriptExecutor(QJSEngine* scriptEngine, QObject* parent)
  : QObject(parent), scriptEngine(scriptEngine), agent(scriptEngine, this), stopped(false)
{ 
    this->settingsFile = "SettingsForAutoScanWindow.ini";
    this->settings = new QSettings(settingsFile, QSettings::IniFormat);

  connect(&agent, SIGNAL(lineChanged(int)), this, SIGNAL(lineChanged(int)));
  //scriptEngine->setAgent(&agent);
}

void ScriptExecutor::runScript(const QString& code, const QString& filename, bool intermediate)
{
  stopped = false;
  emit scriptStarted();
  //scriptEngine->evaluate(code, filename);
  //if (scriptEngine->hasUncaughtException()) {
  //  if (! scriptEngine->uncaughtException().isUndefined()) {
  //    try{
  //      devices::audioDataCollector->stop();
  //      devices::xAxisMotor->stop();
  //      devices::yAxisMotor->stop();
  //      devices::coil->stop();
  //    }catch(...){
  //    }

  //    emit errorMessage(QString("Ошибка в строке %1: %2").arg(scriptEngine->uncaughtExceptionLineNumber()).arg(scriptEngine->uncaughtException().toString()));
  //  }
  //}
//          evaluate() can throw a script exception(e.g.due to a syntax error).If it does, then evaluate() returns the value that was thrown(typically an Error object).Use QJSValue::isError() to check for exceptions.
//          For detailed information about the error, use QJSValue::toString() to obtain an error message, and use QJSValue::property() to query the properties of the Error object.
// The following properties are available :
          //name
          //message
          //fileName
          //lineNumber
          //stack
  QJSValue result = scriptEngine->evaluate(code, filename);
  if (result.isError()) {
      qDebug()
          << "Uncaught exception at line"
          << result.property("lineNumber").toInt()
          << ":" << result.toString();
          try{
            devices::audioDataCollector->stop();
            devices::xAxisMotor->stop();
            devices::yAxisMotor->stop();
            devices::coil->stop();
          }catch(...){
          }

      emit errorMessage(QString("Ошибка в строке %1: %2").arg(result.property("lineNumber").toInt()).arg(result.property("message").toString()));
  }
  if (! intermediate) emit scriptFinished();
}

void ScriptExecutor::stop()
{
  //scriptEngine->abortEvaluation();
  scriptEngine->setInterrupted(true);
  stopped = true;
}

bool ScriptExecutor::isStopped() const
{
  return stopped;
}

ScriptAgent::ScriptAgent(QJSEngine* engine, ScriptExecutor* executor)
  : engine(engine), mainScriptId(-1), executor(executor)
  //: QScriptEngineAgent(engine), mainScriptId(-1), executor(executor)
{ }

void ScriptAgent::positionChange(qint64 scriptId, int lineNumber, int columnNumber)
{
  if (executor->isStopped()) {
    //engine()->currentContext()->throwValue(engine()->undefinedValue());
      engine->globalObject().setProperty("wasStopped", 1); //new2024 требует проверки      //myEngine.globalObject().setProperty("myNumber", 123);
      engine->setInterrupted(true);                        //new2024 требует проверки      //QJSValue myNumberPlusOne = myEngine.evaluate("myNumber + 1");
  } else if (mainScriptId == scriptId) {
    emit lineChanged(lineNumber);
  }
}

void ScriptAgent::scriptLoad(qint64 scriptId, const QString& program, const QString& filename, int baseLineNumber)
{
  if (filename.isEmpty()) {
    mainScriptId = scriptId;
  }
}

