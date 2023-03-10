#include "RTScanCollector.h"
#include "RTAudioCollector.h"
#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <QtWidgets/qboxlayout.h>

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters)
    : m_rtCtxt(trCtxt), m_parameters(parameters)
{
    auto mainLayout = new QVBoxLayout;
    auto splitter = new QSplitter(Qt::Vertical);
    
    setLayout(mainLayout);
    m_oneWave = new OneWaveWidget();
    m_field = new FieldWidget();
    splitter->addWidget(m_oneWave);
    splitter->addWidget(m_field);
    mainLayout->addWidget(splitter);

    if(auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead")))
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
    if(auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector")))
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
}

void realtime::RTScanCollector::
start() {
    m_scan = std::make_shared<Scan>();
    m_scan->parameters = m_parameters;
    m_scan->originalScan.sound.samples.reserve(1500000000);
    m_scan->originalScan.sound.sampleRate = 100000;
    m_oneWave->setScan(m_scan);
    m_field->setScan(m_scan);
    
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        //disconnect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        //connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        m_shift.is_correct = false;
        head->start(10);
    }
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector")))
        col->start(&m_scan->originalScan.sound);
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
calcShift(float x, float y, float z) {
    static bool read = false;
    if (read)
        return;
    ~read;
    m_shift.x = -x;
    m_shift.y = -y;
    m_shift.z = -z;
    headData(x, y, z);
    //m_scan->originalScan.sound.samples.clear();
    
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        disconnect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::calcShift);
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
    }
}
void realtime::RTScanCollector::
headData(float x, float y, float z) {
    if (!m_shift.is_correct) {
        m_shift.is_correct = true;
        m_shift.x = -x;
        m_shift.y = -y;
        m_shift.z = -z;
        m_scan->originalScan.sound.samples.clear();
    }
    m_scan->originalScan.trajectory.pos.push_back({x + m_shift.x,y + m_shift.y,z + m_shift.z, 0, 0});
    //std::cout << x << ":" << y << ":" << z << '\n';
}

void realtime::RTScanCollector::
audioData() {
    //std::cout << "dataSize : " << m_scan->originalScan.sound.samples.size() << '\n';
}

realtime::RTScanCollector::
~RTScanCollector() {

}