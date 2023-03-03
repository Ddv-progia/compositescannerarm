#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <thread>

extern uts::devtalk::device::utscp::APLHeadPrx devices::head;

realtime::RTHead::
RTHead() : RTReceiverI("APLHead"){}

void realtime::RTHead::
start(int periodMs) {
    if (m_isStarted.load(std::memory_order_seq_cst))
        return;

    m_thread = std::thread([&](uts::devtalk::device::utscp::APLHeadPrx &head,int period) {
        try {
            m_isStarted.store(true, std::memory_order_seq_cst);
            if (head)
                head->getPosition(period * 1000, 0);
            m_isStarted.store(false, std::memory_order_seq_cst);
        }
        catch (...) {
            std::cout << "APLHead not found" << '\n';
        }
        }, devices::head, periodMs);
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
    emit newData(x, y, z);
}

realtime::RTHead::
~RTHead() {

}