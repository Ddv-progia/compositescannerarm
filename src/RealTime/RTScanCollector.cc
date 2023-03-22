#include "RTScanCollector.h"
#include "RTAudioCollector.h"
#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <QtWidgets/qboxlayout.h>

bool realtime::RTScanCollector::m_isStarted = false;

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters)
    : m_rtCtxt(trCtxt), m_parameters(parameters)
{
    auto mainLayout = new QVBoxLayout;
    auto splitter = new QSplitter(Qt::Vertical);
    
    setLayout(mainLayout);
    m_oneWave = new OneWaveWidget();
    m_field = new FieldWidget(WIDTH, HEIGHT);
    splitter->addWidget(m_oneWave);
    splitter->addWidget(m_field);
    mainLayout->addWidget(splitter);

    m_scan = std::make_shared<Scan>();
    m_scan->parameters = m_parameters;
    m_scan->originalScan.sound.sampleRate = SOUDS_SAMPLE_RATE;
    m_scan->originalScan.sound.samples.reserve(m_scan->originalScan.sound.sampleRate * SEC_PER_MINUTE * MAXIMUM_TIME);
    m_scan->originalScan.trajectory.sampleRate = HEAD_SAMPLE_RATE;
    m_scan->originalScan.trajectory.pos.reserve(m_scan->originalScan.trajectory.sampleRate * SEC_PER_MINUTE * MAXIMUM_TIME);

    m_oneWave->setScan(m_scan);
    m_field->setScan(m_scan);



}

void realtime::RTScanCollector::
start() {
    if (m_isStarted)
        return;
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        head->start(m_scan->originalScan.trajectory.sampleRate);
    }
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->start(&m_scan->originalScan.sound);
    }
    m_isThisStarted = m_isStarted = true;
}

void realtime::RTScanCollector::
stop() {
    if (!m_isThisStarted)
        return;
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        disconnect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->stop();
    }
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        disconnect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        head->stop();
    }
    m_isStarted = false;
}

void realtime::RTScanCollector::
headData(float x, float y, float z) {
    if (!m_shift.is_correct) {
        m_shift.is_correct = true;
        m_shift.x = (WIDTH / 2) - x;
        m_shift.y = (HEIGHT / 2) - y;
        m_shift.z = -z;
        m_scan->originalScan.sound.samples.clear();
    }
    m_scan->originalScan.trajectory.pos.push_back({x + m_shift.x,y + m_shift.y,z + m_shift.z, 0, 0});
}

void realtime::RTScanCollector::
audioData() {
    //std::cout << "dataSize : " << m_scan->originalScan.sound.samples.size() << " "
       // << "dataSize : " << m_scan->originalScan.trajectory.pos.size() << '\n';
}

realtime::RTScanCollector::
~RTScanCollector() {
    stop();
}