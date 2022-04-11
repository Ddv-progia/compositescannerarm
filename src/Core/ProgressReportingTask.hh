/*
 * Core/ProgressReportingTask.hh
 */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

class ProgressReportingTask : public QObject
{
  Q_OBJECT

public:
  virtual void operator()() = 0;

protected:
  Q_SIGNAL void started(const QString& name, int stageCount);
  Q_SIGNAL void stageStarted(const QString& name, int maximumValue);
  Q_SIGNAL void stageProgressed();
  Q_SIGNAL void finished();
  Q_SIGNAL void terminated(const QString& errorMessage);
  Q_SIGNAL void createdTask(ProgressReportingTask* newTask);
};
