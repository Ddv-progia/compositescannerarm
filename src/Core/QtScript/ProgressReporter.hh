/*
 * Core/QtScript/ProgressReporter.hh
 */

#pragma once

#include <QtCore/QObject>

namespace script {

  class ProgressReporter : public QObject
  {
    Q_OBJECT
  public:
    explicit ProgressReporter(QObject* parent = 0)
      : QObject(parent)
    { }

  protected:
    Q_SIGNAL void taskStarted(int stepCount);
    Q_SIGNAL void taskProgressed();
    Q_SIGNAL void taskFinished();
  };

}
