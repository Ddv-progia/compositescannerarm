#include "RTAudioCollector.h"
#include "Core/Devices.hh"
#include <array>
#include <chrono>

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
    //std::time_t timeStampNewData = std::time(nullptr);
    //auto timeStampNewData = std::chrono::system_clock::now();

    using namespace std::chrono;
    /*int64_t */std::time_t timeStampNewData = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();


    std::time_t timeStampFromSample;

    auto size = m_sound->samples.size();
    auto dataAudioSize = (data.size() - sizeof(LONGLONG));
    auto sizeRes = dataAudioSize / sizeof(float);
    m_sound->samples.resize(size + sizeRes);
    memcpy(&timeStampFromSample, data.data()+ dataAudioSize, sizeof(LONGLONG));
        //unsigned int x;
        //unsigned int y;
        //x = (timeStampFromSample & 0x00000000FFFFFFFF);
        //y = (timeStampFromSample & 0xFFFFFFFF00000000) >> 32;
        //x = (x & 0x00FF00FF) << 8 | (x & 0xFF00FF00) >> 8;
        //x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
        //y = (y & 0x00FF00FF) << 8 | (y & 0xFF00FF00) >> 8;
        //y = (y & 0x0000FFFF) << 16 | (y & 0xFFFF0000) >> 16;
        //timeStampFromSample = 0xFFFFFFFFFFFFFFFF;
        //timeStampFromSample = (timeStampFromSample & x) << 32;
        //timeStampFromSample = (timeStampFromSample|y);
    //auto n = timeStampFromSample;
    //int m = (n & ~0xffff) ^ (0xff00 & (n << 8)) ^ ((n >> 8) & 0xff);
    //memcpy(m_sound->samples.data() + size, data.data(), sizeRes * sizeof(float));
    memcpy(m_sound->samples.data() + size, data.data(), dataAudioSize);
    emit newData(size, dataAudioSize, timeStampNewData, timeStampFromSample);
}

realtime::RTAudioCollector::
~RTAudioCollector() {

}