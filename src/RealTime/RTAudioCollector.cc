#include "RTAudioCollector.h"
#include "Core/Devices.hh"
#include <array>


extern uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;

realtime::RTAudioCollector::
RTAudioCollector() 
    : RTReceiverI("AudioDataCollector"){

}

void realtime::RTAudioCollector::
start(int sampleRate,Sound *sound) {
    m_sound = sound;
    try {
        if (devices::audioDataCollector)
            devices::audioDataCollector->start(sampleRate, 1);
    }
    catch (...) {
        std::cout << "AudioDataCollector ready or not found \n";
    }
}

void realtime::RTAudioCollector::
stop() {
    try {
        if(devices::audioDataCollector)
        devices::audioDataCollector->stop();
    }
    catch (...) {
        std::cout << "AudioDataCollector на сервере не существует \n"
            "возможно нужно выключить антивирус" << '\n';
    }
}

void realtime::RTAudioCollector::
dataReady(const ::uts::devtalk::ByteSeq& data, const ::Ice::Current&) {
    std::vector<float> floatData;
    floatData.resize((data.size() - sizeof(LONGLONG) ) / sizeof(float) );
    auto pos = data.data();
    for (auto &floatVal : floatData) {
        memcpy(&floatVal, pos, sizeof(float));
        pos += sizeof(float);
    }
    LONGLONG timestamp;
    memcpy(&timestamp, pos, sizeof(LONGLONG));
    if (m_sound)
        m_sound->samples.insert(m_sound->samples.end(), floatData.begin(), floatData.end());
    emit newData();
}

realtime::RTAudioCollector::
~RTAudioCollector() {

}