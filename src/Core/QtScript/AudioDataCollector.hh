/*
 * Core/QtScript/AudioDataCollector.hh
 */

#pragma once

#include <DevTalk/Device/AudioDataCollector.hh>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtScript/QScriptEngine>
#include <QLabel>

#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"

namespace script {

  class AudioDataCollector : public QObject
  {
    Q_OBJECT

    uts::devtalk::AudioDataCollectorPrx collector;
    QScriptEngine* scriptEngine;
  public:
    AudioDataCollector(const uts::devtalk::AudioDataCollectorPrx& collector, QScriptEngine* scriptEngine);

    Q_INVOKABLE void start(int sampleRate);
    Q_INVOKABLE void stop(double startCoordinate, double finalCoordinate, double lineCoordinate);
    Q_INVOKABLE void discard();

  protected:
    Q_SIGNAL void lineFechted(const SourceScanLine& samples);

  private:
    template<typename F>
    void wrapExceptions(F f);
  };

  class TestLabel : public QObject
  {
    Q_OBJECT

    QLabel* label;
    QScriptEngine* scriptEngine;
  public:
    TestLabel(QLabel* newLabel,QScriptEngine* scriptEngine):label(newLabel),scriptEngine(scriptEngine)
    {}
    Q_INVOKABLE void show()
    {
    }
    
    Q_INVOKABLE void setText(int num)
    {
      label->setText(QString::number(num));
    }

  };

}
