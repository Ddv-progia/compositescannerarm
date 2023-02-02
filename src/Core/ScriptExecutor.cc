/*
 * Core/ScriptExecutor.cc
 */

#include "Core/Devices.hh"
#include "Core/ScriptExecutor.hh"

ScriptExecutor::ScriptExecutor(QScriptEngine* scriptEngine, QObject* parent)
  : QObject(parent), scriptEngine(scriptEngine), agent(scriptEngine, this)
{ 
  connect(&agent, SIGNAL(lineChanged(int)), this, SIGNAL(lineChanged(int)));
  scriptEngine->setAgent(&agent);
}

void ScriptExecutor::runScript(const QString& code, const QString& filename, bool intermediate)
{
  stopped = false;
  emit scriptStarted();
  scriptEngine->evaluate(code, filename);
  if (scriptEngine->hasUncaughtException()) {
    if (! scriptEngine->uncaughtException().isUndefined()) {
      try{
        devices::audioDataCollector->stop();
        //devices::xAxisMotor->stop();
        //devices::yAxisMotor->stop();
        devices::coile->stop();
      }catch(...){
      }

      emit errorMessage(QString("Ошибка в строке %1: %2").arg(scriptEngine->uncaughtExceptionLineNumber()).arg(scriptEngine->uncaughtException().toString()));
    }
  }

  if (! intermediate) emit scriptFinished();
}

void ScriptExecutor::stop()
{
  scriptEngine->abortEvaluation();
  stopped = true;
}

bool ScriptExecutor::isStopped() const
{
  return stopped;
}

ScriptAgent::ScriptAgent(QScriptEngine* engine, ScriptExecutor* executor)
  : QScriptEngineAgent(engine), mainScriptId(-1), executor(executor)
{ }

void ScriptAgent::positionChange(qint64 scriptId, int lineNumber, int columnNumber)
{
  if (executor->isStopped()) {
    engine()->currentContext()->throwValue(engine()->undefinedValue());
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

