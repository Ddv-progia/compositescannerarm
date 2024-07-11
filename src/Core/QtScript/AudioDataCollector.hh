/*
 * Core/QtScript/AudioDataCollector.hh
 */

#pragma once

#include <DevTalk/Device/AudioDataCollector.hh>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QJSEngine>

#include <QLabel>

#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"

namespace script {

  class AudioDataCollector : public QObject
  {
    Q_OBJECT
    uts::devtalk::AudioDataCollectorPrx collector;
    QJSEngine* scriptEngine;
  public:
    AudioDataCollector(const uts::devtalk::AudioDataCollectorPrx& collector, QJSEngine* scriptEngine);
    //AudioDataCollector(const uts::devtalk::AudioDataCollectorPrx& collector);

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
    QJSEngine* scriptEngine;
  public:
      TestLabel(QLabel* newLabel, QJSEngine* scriptEngine) :label(newLabel), scriptEngine(scriptEngine)
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
