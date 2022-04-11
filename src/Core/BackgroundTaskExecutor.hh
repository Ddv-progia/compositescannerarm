/*
 * Core/BackgroundTaskExecutor.hh
 */

#pragma once

#include <memory>
#include <QtCore/QObject>

#include "Core/ProgressReportingTask.hh"

class BackgroundTaskExecutor : public QObject
{
  Q_OBJECT

  struct Impl;
  std::unique_ptr<Impl> impl;
public:
  BackgroundTaskExecutor();
  ~BackgroundTaskExecutor();

  Q_SLOT void enqueue(ProgressReportingTask* task);
  void operator()();

protected:
  Q_SIGNAL void started(const QString& name, int stageCount);
  Q_SIGNAL void stageStarted(const QString& name, int maximumValue);
  Q_SIGNAL void stageProgressed();
  Q_SIGNAL void finished();
  Q_SIGNAL void terminated(const QString& errorMessage);
};
