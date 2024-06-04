#include "RTScanCollector.h"
#include "RTAudioCollector.h"
#include "RTHead.h"
#include "Core/Devices.hh"
#include <array>
#include <QtWidgets/qboxlayout.h>
#include "soundDisplay.h"
#include "peakDisplay.h"
#include "Core/BinaryPersistentVariable.hh"
#include "Core/BinaryTransformation.hh"
#include <Core/ScanDataReflection.hh>
#include <Core/DevicesConfigurationReflection.hh>


bool realtime::RTScanCollector::m_isStarted = false;

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters)
    : m_rtCtxt(trCtxt), m_parameters(parameters)
{
    auto mainLayout = new QVBoxLayout;
    auto splitter = new QSplitter(Qt::Vertical);
    
    setLayout(mainLayout);
    m_soundDisplay = new SoundDisplay();
    m_peakDisplay = new PeakDisplay();
    m_field = new FieldWidget(WIDTH, HEIGHT);
    splitter->addWidget(m_soundDisplay);
    splitter->addWidget(m_peakDisplay);
    splitter->addWidget(m_field);
    mainLayout->addWidget(splitter);

    m_scan = std::make_shared<Scan>();
    m_scan->parameters = m_parameters;
    m_scan->sound.sampleRate = SOUDS_SAMPLE_RATE;
    m_scan->sound.samples.reserve(m_scan->sound.sampleRate * SEC_PER_MINUTE * MAXIMUM_TIME);
    m_scan->trajectory.sampleRate = HEAD_SAMPLE_RATE;
    m_scan->trajectory.pos.reserve(m_scan->trajectory.sampleRate * SEC_PER_MINUTE * MAXIMUM_TIME);

    m_soundDisplay->setScan(m_scan);
    m_peakDisplay->setScan(m_scan);
    m_field->setScan(m_scan);
}

void realtime::RTScanCollector::
start() {
    if (m_isStarted)
        return;
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        head->start(m_scan->trajectory.sampleRate);
    }
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->start(&m_scan->sound);
    }
    m_isThisStarted = m_isStarted = true;
}

void realtime::RTScanCollector::
stop() {
    if (!m_isThisStarted)
        return;
    m_isStarted = false;
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        disconnect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->stop();
    }
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        disconnect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        head->stop();
    }

    emit ready(m_scan);
}

void realtime::RTScanCollector::
headData(float x, float y, float z) {
    if (!m_shift.is_correct) {
        m_shift.is_correct = true;
        m_shift.x = (WIDTH / 2) - x;
        m_shift.y = (HEIGHT / 2) - y;
        m_shift.z = -z;
        m_scan->sound.samples.clear();
    }
    m_scan->trajectory.pos.push_back({x + m_shift.x,y + m_shift.y,z + m_shift.z, 0, 0});
}

void realtime::RTScanCollector::
audioData() {
    std::cout << "dataSize : " << m_scan->sound.samples.size() << " "
        << "dataSize : " << m_scan->trajectory.pos.size() << '\n';
}

void realtime::RTScanCollector::save(BackgroundTaskExecutor& taskExecutor) {
    auto newPathname = QFileDialog::getSaveFileName(this, "Сохранить скан в файл", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        Core::BinaryPersistentVariable<Scan > binaryScan{ newPathname.toStdString() };
        binaryScan = *m_scan;
        binaryScan.save();
    }
}

void realtime::RTScanCollector::saveAs(BackgroundTaskExecutor& taskExecutor) {
    auto newPathname = QFileDialog::getSaveFileName(this, "Сохранить скан в файл", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        Core::BinaryPersistentVariable<Scan > binaryScan{ newPathname.toStdString() };
        binaryScan = *m_scan;
        binaryScan.save();
    }
}

void realtime::RTScanCollector::load(BackgroundTaskExecutor& taskExecutor) {
    auto newPathname = QFileDialog::getOpenFileName(this, "Открыть скан", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        Core::BinaryPersistentVariable<OriginalData > binaryScan{ newPathname.toStdString() };
        binaryScan.load();
        m_scan->sound.samples = std::move(binaryScan->sound.samples);
        m_scan->sound.sampleRate = binaryScan->sound.sampleRate;
        m_scan->trajectory.pos = std::move(binaryScan->trajectory.pos);
        m_scan->trajectory.sampleRate = binaryScan->trajectory.sampleRate;
        m_scan->rtPeaks = std::move(binaryScan->peaks);
        m_scan->parameters = binaryScan->parameters;
    }
}

realtime::RTScanCollector::
~RTScanCollector() {
    stop();
}