#include "RTAudioCollector.h"
#include "Core/Devices.hh"
#include <array>


extern uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;

realtime::RTAudioCollector::
RTAudioCollector() 
    : RTReceiverI("AudioDataCollector"){

}

void realtime::RTAudioCollector::
start(Sound *sound) {
    m_sound = sound;
    try {
        if (devices::audioDataCollector)
            devices::audioDataCollector->start(sound->sampleRate, 1);
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
    /*std::vector<float> floatData;
    floatData.resize((data.size() - sizeof(LONGLONG) ) / sizeof(float) );
    auto pos = data.data();
    for (auto &floatVal : floatData) {
        memcpy(&floatVal, pos, sizeof(float));
        pos += sizeof(float);
    }
    LONGLONG timestamp;
    memcpy(&timestamp, pos, sizeof(LONGLONG));
    std::cout << m_sound->samples.capacity() << " " << m_sound->samples.size() << "\n";
    if (m_sound)
        m_sound->samples.insert(m_sound->samples.end(), floatData.begin(), floatData.end());*/
    auto size = m_sound->samples.size();
    auto sizeRes = (data.size() - sizeof(LONGLONG)) / sizeof(float);
    m_sound->samples.resize(size + sizeRes);
    memcpy(m_sound->samples.data() + size, data.data(), sizeRes * sizeof(float));

    //std::cout << m_sound->samples.capacity() << " " << m_sound->samples.size() << "\n";
    /*auto sizeRes = (data.size() - sizeof(LONGLONG)) / sizeof(float);
    std::vector<float> floatData{ sizeRes };
    floatData.resize(sizeRes);
    memcpy(floatData.data(), data.data(), sizeRes * sizeof(float));*/
    emit newData();
}

realtime::RTAudioCollector::
~RTAudioCollector() {

}