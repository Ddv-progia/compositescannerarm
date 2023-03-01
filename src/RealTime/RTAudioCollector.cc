#include "RTAudioCollector.h"
#include "Core/Devices.hh"
#include <array>


extern uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;

realtime::RTAudioCollector::
RTAudioCollector() 
    : RTReceiverI("AudioDataCollector"){

}

void realtime::RTAudioCollector::
start(int sampleRate) {
    if(devices::audioDataCollector)
       devices::audioDataCollector->start(sampleRate, 1);
}

void realtime::RTAudioCollector::
stop() {
    if(devices::audioDataCollector)
        devices::audioDataCollector->stop();
}

void realtime::RTAudioCollector::
dataReady(const ::uts::devtalk::ByteSeq& data, const ::Ice::Current&) {
    Ice::FloatSeq floatData;
    floatData.resize((data.size() - sizeof(LONGLONG) ) / sizeof(Ice::Float) );
    auto pos = data.data();
    for (auto &floatVal : floatData) {
        memcpy(&floatVal, pos, sizeof(Ice::Float));
        pos += sizeof(Ice::Float);
    }
    LONGLONG timestamp;
    memcpy(&timestamp, pos, sizeof(LONGLONG));

    std::cout << "Data : " << timestamp
        << " Size : " << floatData.size();
    if (!floatData.empty())
        std::cout << "Data : " << floatData.at(0);
    std::cout << std::endl;
}

realtime::RTAudioCollector::
~RTAudioCollector() {

}