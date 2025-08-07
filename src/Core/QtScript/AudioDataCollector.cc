/*Ice
 * Core/QtScript/AudioDataCollector.cc
 */

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Core/QtScript/AudioDataCollector.hh"
#include <QtWidgets/QMessageBox>
//using namespace std::chrono;

script::AudioDataCollector::AudioDataCollector(const uts::devtalk::AudioDataCollectorPrx& collector, QJSEngine* scriptEngineIn)
  : collector(collector), scriptEngine(scriptEngineIn)
{}

void script::AudioDataCollector::start(int sampleRate, int channelCount)
{
    //collector->start(sampleRate, channelCount);
        //auto timeStampNewData = std::chrono::system_clock::now();//std::time_t timeStampNewData = std::time(nullptr);boost::posix_time::microsec_clock::local_time()
    auto timestampStartLocal =  boost::posix_time::microsec_clock::local_time();
        //using namespace std::chrono;
       //    /*int64_t */std::time_t timeStampNewData = duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    auto timestampStartLocal2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::cout << "timestampStartLocal  = " << timestampStartLocal <<" \n";
    std::cout << "timestampStartLocal2 = " << timestampStartLocal2 <<" \n";
    timestampStart = timestampStartLocal2;

  wrapExceptions([=]() { collector->start(sampleRate, channelCount); });
}

void script::AudioDataCollector::start(int sampleRate)
{
    Ice::Int Channels = 1;
    start(sampleRate, Channels);
  //  Ice::Int sampleRateIce = sampleRate;
  ////wrapExceptions([=]() {  collector->start(sampleRate, Channels); });
  //collector->start(sampleRate, Channels);
}

void script::AudioDataCollector::stop(double startCoordinate, double finalCoordinate, double lineCoordinate, double finalLineCoordinate)
{
    uts::devtalk::AudioDataCollectorSamples* samples = new uts::devtalk::AudioDataCollectorSamples();
  wrapExceptions([=]() { 
   *samples = collector->stop();
  });
  if (samples->samples.size() > 0) {
      auto timestampStartLocal = boost::posix_time::microsec_clock::local_time();
      std::time_t  timestampStartLocal2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      //std::cout << "AudioDataCollector::stop timestampStartLocal  = " << timestampStartLocal << " \n"; // out:  "2025-Jul-23 10:33:26.564334"
      //std::cout << "AudioDataCollector::stop timestampStartLocal2 = " << timestampStartLocal2 << " \n";
      wrapExceptions([=]() {
          emit lineFechted(SourceScanLine{ samples->samples.front(),
                          static_cast<unsigned int>(samples->sampleRate),
                          startCoordinate,
                          finalCoordinate,
                          lineCoordinate,
                          finalLineCoordinate,
                          timestampStart,
                          (unsigned long long int)(timestampStartLocal2)
              });
          //std::cout << "AudioDataCollector lineCoordinate = " << lineCoordinate << " startCoordinate = " << startCoordinate << "\r\n";
          });
  }
}

void script::AudioDataCollector::stopAndDiscardSamples()
{
    wrapExceptions([=]() {
        auto samples = collector->stop();
        });
    wrapExceptions([=]() { collector->discard(); });
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
    scriptEngine->throwError(QString::fromUtf8(e.reason.c_str()));
  } catch (Ice::Exception& e) {
      scriptEngine->throwError(QString::fromUtf8(e.what()));
  }
}


