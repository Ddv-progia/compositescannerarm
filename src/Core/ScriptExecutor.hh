/*
 * Core/ScriptExecutor.hh
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptEngineAgent>

class ScriptExecutor;
class ScriptAgent : public QObject, public QScriptEngineAgent
{
  Q_OBJECT
public:
  ScriptAgent(QScriptEngine* engine, ScriptExecutor* executor);

  virtual void positionChange(qint64 scriptId, int lineNumber, int columnNumber) override;
  virtual void scriptLoad(qint64 scriptId, const QString& program, const QString& filename, int baseLineNumber) override;

protected:
  Q_SIGNAL void lineChanged(int lineNumber);

private:
  qint64 mainScriptId;
  ScriptExecutor* executor;
};

class ScriptExecutor : public QObject
{
  Q_OBJECT
public:
  explicit ScriptExecutor(QScriptEngine* scriptEngine, QObject* parent = 0);

  Q_SLOT void runScript(const QString& code, const QString& filename, bool intermediate);
  Q_SLOT void setProperty(const QString& name, const QScriptValue& value) {
    scriptEngine->globalObject().setProperty(name, value);
  }

  void stop();
  bool isStopped() const;

protected:
  Q_SIGNAL void scriptStarted();
  Q_SIGNAL void errorMessage(const QString& msg);
  Q_SIGNAL void lineChanged(int lineNumber);
  Q_SIGNAL void scriptFinished();

private:
  QScriptEngine* scriptEngine;
  ScriptAgent agent;
  volatile bool stopped;
};