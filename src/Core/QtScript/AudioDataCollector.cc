/*
 * Core/QtScript/AudioDataCollector.cc
 */

#include "Core/QtScript/AudioDataCollector.hh"

script::AudioDataCollector::AudioDataCollector(const uts::devtalk::AudioDataCollectorPrx& collector, QScriptEngine* scriptEngine)
  : collector(collector), scriptEngine(scriptEngine)
{ }

void script::AudioDataCollector::start(int sampleRate)
{
  wrapExceptions([=]() { collector->start(sampleRate, 1); });
}

void script::AudioDataCollector::stop(double startCoordinate, double finalCoordinate, double lineCoordinate)
{
  wrapExceptions([=]() { 
    auto samples = collector->stop();
    emit lineFechted(SourceScanLine{ samples.samples.front(),
                    static_cast<unsigned int>(samples.sampleRate),
                    startCoordinate,
                    finalCoordinate,
                    lineCoordinate });
  });
}

void script::AudioDataCollector::discard()
{
  wrapExceptions([=]() { collector->discard(); });
}

template<typename F>
void script::AudioDataCollector::wrapExceptions(F f)
{
  try {
    f();
  } catch (uts::devtalk::AudioIOException& e) {
    scriptEngine->currentContext()->throwError(QString::fromUtf8(e.reason.c_str()));
  } catch (Ice::Exception& e) {
    scriptEngine->currentContext()->throwError(QString::fromUtf8(e.what()));
  }
}


