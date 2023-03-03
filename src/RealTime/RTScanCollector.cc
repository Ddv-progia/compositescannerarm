#include "RTScanCollector.h"
#include "RTAudioCollector.h"
#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <QtWidgets/qboxlayout.h>



realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt)
    :m_rtCtxt(trCtxt)
{
    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    m_oneWave = new OneWaveWidget();
    mainLayout->addWidget(m_oneWave);

    if(auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead")))
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
    if(auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector")))
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
}

void realtime::RTScanCollector::
start() {
    m_scan = std::make_shared<Scan>();
    m_oneWave->setScan(m_scan);
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector")))
        col->start(1000,&m_scan->originalScan.sound);
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead")))
        head->start(100);
}

void realtime::RTScanCollector::
pause() {
    
}

void realtime::RTScanCollector::
stop() {
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector")))
        col->stop();
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead")))
        head->stop();
}

void realtime::RTScanCollector::
headData(float x, float y, float z) {
    std::cout << x << ":" << y << ":" << z << '\n';
}

void realtime::RTScanCollector::
audioData() {
    std::cout << "dataSize : " << m_scan->originalScan.sound.samples.size() << '\n';
}

realtime::RTScanCollector::
~RTScanCollector() {

}