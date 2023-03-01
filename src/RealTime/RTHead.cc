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
        m_isStarted.store(true, std::memory_order_seq_cst);
        if (head)
            head->getPosition(period * 1000, 0);
        m_isStarted.store(false, std::memory_order_seq_cst);
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
    for (auto &floatVal : intData) {
        memcpy(&floatVal, pos++, sizeof(int));
    }
    //for (int ind = 0; ind < intData.size(); ++ind)
      //  std::cout << intData.at(ind) << " ";
    
   
    uint32_t shift1 = 4449;
    uint16_t enc1 = (intData.at(3) + shift1) % UINT16_MAX;
    const double PI = 3.1415926535897932384626433832795;
    const double COEFF = UINT16_MAX / (PI * 2.0);
    const double R1 = 57;// 200.39;
    double angle = enc1 / COEFF;
    double x1 = R1 * sin(angle);
    double y1 = R1 * cos(angle);
    double z1 = 0.0;
    //std::cout << enc1 << " : " << x1 << " " << y1 << " " << z1 << std::endl;

    uint32_t shift2 = 64620;
    uint16_t enc2 = (intData.at(1) + shift2) % UINT16_MAX;
    const double R2 = 801.978;
    angle = enc2 / COEFF;
    double x2 = R2 * cos(angle);
    double y2 = 0.0;
    double z2 = R2 * sin(angle);
    //std::cout << enc2 / 182.0444 << " : " << x2 << " " << y2 << " " << z2 << std::endl;

    uint32_t shift3 = 22489;
    uint16_t enc3 = (intData.at(2) + shift3) % UINT16_MAX;
    const double R3 = 815.529;
    angle += enc3 / COEFF;
    double x3 = (x2 + R3 * cos(angle)) * -1;
    double y3 = 0.0;
    double z3 = z2 + R3 * sin(angle);
    //std::cout <<  enc3 / 182.0444 << " : " << x3 << " " << y3 << " " << z3 << std::endl << std::endl;

    //std::cout << x1 << " " << y1  << " " << enc1 << '\n';
    //Прямая
    //double k = (y2 - y1) / (x2 - x1);
    //double b = (x2 * y1 - x1 * y2) / (x2 - x1);
    double k =  y1 / x1;
    double b = 0;
    double kPerp = -1 / k;
    double bPerp = y1 - kPerp * x1;

    double length = x3 - 191.121;
    double ck = pow(x1, 2) - ( pow(length, 2) / (1 + pow(kPerp, 2)) );
    double bk = x1 * 2.0;
    double ak = 1.0;
    //a*x^2 -  b*x + c
    double d = bk * bk - 4 * ak * ck;
    double resultX, resultX2;
    resultX = ((-bk) + sqrt(d)) / (2 * ak);
    std::cout << resultX << " " << resultX * kPerp + bPerp << " " << z3 << std::endl << std::endl;

}

realtime::RTHead::
~RTHead() {

}