/*
 * Core/ScriptExecutor.hh
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QJSEngine>

//#include <QtScript/QScriptEngineAgent>

#include <QSettings>

class ScriptExecutor;
//class ScriptAgent : public QObject, public QScriptEngineAgent
class ScriptAgent : public QObject
{
  Q_OBJECT
public:
  ScriptAgent(QJSEngine* engine, ScriptExecutor* executor);


  //virtual void positionChange(qint64 scriptId, int lineNumber, int columnNumber) override;
  //virtual void scriptLoad(qint64 scriptId, const QString& program, const QString& filename, int baseLineNumber) override;
  virtual void positionChange(qint64 scriptId, int lineNumber, int columnNumber);
  virtual void scriptLoad(qint64 scriptId, const QString& program, const QString& filename, int baseLineNumber);

protected:
  Q_SIGNAL void lineChanged(int lineNumber);

private:
  qint64 mainScriptId;
  ScriptExecutor* executor;
  QJSEngine* engine;
};

class ScriptExecutor : public QObject
{
  Q_OBJECT
public:
  explicit ScriptExecutor(QJSEngine* scriptEngine, QObject* parent = 0);


  Q_SLOT void runScript(const QString& code, const QString& filename, bool intermediate);
  QJSValue commonModule;
  QString settingsFile;
  QSettings* settings;
  Q_SLOT void setProperty(const QString& name, const QJSValue& value) {
    scriptEngine->globalObject().setProperty(name,  value);
      settings->setValue(name, value.toVariant());
  }

  void stop();
  bool isStopped() const;


protected:
  Q_SIGNAL void scriptStarted();
  Q_SIGNAL void errorMessage(const QString& msg);
  Q_SIGNAL void lineChanged(int lineNumber);
  Q_SIGNAL void scriptFinished();

private:
  QJSEngine* scriptEngine;
  ScriptAgent agent;
  volatile bool stopped;
};