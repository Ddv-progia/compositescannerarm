#include "RTAudioCollector.h"
#include "Core/Devices.hh"
#include <array>


extern uts::devtalk::AudioDataCollectorPrx devices::audioDataCollector;

realtime::RTAudioCollector::
RTAudioCollector() : RTReceiverI("AudioDataCollector"){}

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

    auto size = m_sound->samples.size();
    auto sizeRes = (data.size() - sizeof(LONGLONG)) / sizeof(float);
    m_sound->samples.resize(size + sizeRes);
    memcpy(m_sound->samples.data() + size, data.data(), sizeRes * sizeof(float));

    emit newData();
}

realtime::RTAudioCollector::
~RTAudioCollector() {

}