#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <thread>
#include <chrono>


extern uts::devtalk::device::utscp::APLHeadPrx devices::head;

realtime::RTHead::
RTHead() : RTReceiverI("APLHead"){}

void realtime::RTHead::
start(int sampleRate) {
    if (m_isStarted.load(std::memory_order_seq_cst))
        return;

    int period = std::round(1000000.0 / sampleRate); // в микросекундах
    m_thread = std::thread([&](const uts::devtalk::device::utscp::APLHeadPrx &head, int period) {
        try {
            m_isStarted.store(true, std::memory_order_seq_cst);
            if (head)
                head->getPosition(period , 0);
            m_isStarted.store(false, std::memory_order_seq_cst);
        }
        catch (...) {
            std::cout << "APLHead not found" << '\n';
        }
        }, devices::head, period);
    m_thread.detach();
}

void realtime::RTHead::
stop() {
    if(devices::head)
        devices::head->stop();
    if (m_thread.joinable())
        m_thread.join();
}

void realtime::RTHead::
dataReady(const ::uts::devtalk::ByteSeq& data, const ::Ice::Current&) {
    //using namespace std::chrono;
    ////int64_t timeStamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    std::time_t timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (std::abs(timeStamp- m_positionLast.m_timeStampLast)<10) {
        return;
    }
    //std::time_t timeStamp = 0;
    //timeStamp = std::time(nullptr);
    //const auto p1 = std::chrono::system_clock::now();
    std::vector<int> intData(data.size() / sizeof(int) );
    int *pos = (int *)data.data();
    for (auto &floatVal : intData)
        memcpy(&floatVal, pos++, sizeof(int));
    
    double enc1Rad = ((intData.at(3) + m_hand.encShifts.at(0)) % m_hand.ENC_MAX) / m_hand.ENC_TO_RAD;
    double enc2Rad = ((intData.at(1) + m_hand.encShifts.at(1)) % m_hand.ENC_MAX) / m_hand.ENC_TO_RAD;
    double enc3Rad = ((intData.at(2) + m_hand.encShifts.at(2)) % m_hand.ENC_MAX) / m_hand.ENC_TO_RAD + enc2Rad;

    double length = -((m_hand.edgeLengths.at(2) * cos(enc2Rad) ) + m_hand.edgeLengths.at(3) * cos(enc3Rad)) - m_hand.edgeLengths.at(1);
    double angPx = m_hand.edgeLengths.at(0) * sin(enc1Rad);
    double angPy = m_hand.edgeLengths.at(0) * cos(enc1Rad);
    double k = -1 / (angPy / angPx);
    double d = pow(angPx * 2.0, 2) - 4 * ( pow(angPx, 2) - ( pow(length, 2) / (1 + pow(k, 2)) ) );

    float x = ((-angPx * 2.0) + sqrt(d)) / 2;
    float y = x * k + (angPy - k * angPx);
    float z = (m_hand.edgeLengths.at(2) * sin(enc2Rad)) + m_hand.edgeLengths.at(3) * sin(enc3Rad);

    //memcpy(&timeStamp, &timestampint64_t, sizeof(std::time_t));
    emit newData(x, y, z, timeStamp);
}

void realtime::RTHead::
onNewData(float x, float y, float z, std::time_t timeStamp) {
    m_positionLast.m_timeStampLast = timeStamp;
    m_positionLast.x = x;
    m_positionLast.y = y;
    m_positionLast.z = z;
};


realtime::RTHead::
~RTHead() {

}