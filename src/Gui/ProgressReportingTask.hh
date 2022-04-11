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
  virtual void operator()() const = 0;

protected:
  Q_SIGNAL void started(const QString& name);
  Q_SIGNAL void stageStarted(const QString& name, int maximumValue);
  Q_SIGNAL void stageProgressed(int value);
  Q_SIGNAL void finished();
};
