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
#include <Gui/ScanDisplayWindow.hh>

//#include "Core/ProgressReportingTask.hh"
#include "Core/SaveScanTask.hh"
#include "Core/LineEncoding.hh"

bool realtime::RTScanCollector::m_isStarted = false;

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters)
    : m_rtCtxt(trCtxt), m_parameters(parameters)
{
    auto mainLayout = new QVBoxLayout;
    auto splitter   = new QSplitter(Qt::Vertical);
    
    setLayout(mainLayout);
    m_soundDisplay = new SoundDisplay();
    m_peakDisplay = new PeakDisplay();
    m_field = new FieldWidget(m_parameters.headAndScanCollectorParameters.width, m_parameters.headAndScanCollectorParameters.height);
    splitter->addWidget(m_soundDisplay);
    splitter->addWidget(m_peakDisplay);
    splitter->addWidget(m_field);
    mainLayout->addWidget(splitter);

    m_scan = std::make_shared<ScanArm>();
    m_scan->parameters = m_parameters;
    m_scan->sound.sampleRate = m_parameters.headAndScanCollectorParameters.soundsSampleRate;
    m_scan->sound.samples.reserve(m_scan->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);
    m_scan->trajectory.sampleRate = m_parameters.headAndScanCollectorParameters.headsSampleRate;
    m_scan->trajectory.pos.reserve(m_scan->trajectory.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);

    sourceScanChunks = std::make_shared<SourceScanChunks>();

    m_soundDisplay->setScan(m_scan);
    m_peakDisplay->setScan(m_scan);
    m_field->setScan(m_scan);
}

void realtime::RTScanCollector::
start() {
    if (m_isStarted)
        return;
    //if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
    //    connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
    //    head->start(m_scan->trajectory.sampleRate);
    //}
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        head->start(m_scan->trajectory.sampleRate);
    }
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->start(&m_scan->sound);
    }
    m_field->runTimers();
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
    m_field->stopTimers();
    emit ready(m_scan);
}

void realtime::RTScanCollector::
headData(float x, float y, float z, time_t timeStamp) {
    if (!m_shift.is_correct) {
        m_shift.is_correct = true;
        m_shift.x = (m_parameters.headAndScanCollectorParameters.width / 2) - x;
        m_shift.y = (m_parameters.headAndScanCollectorParameters.height / 2) - y;
        m_shift.z = -z;
        m_scan->sound.samples.clear();
        //m_scan->trajectory.pos.clear();

    }
    auto trajectory_pos_size = m_scan->trajectory.pos.size();
    m_scan->trajectory.pos.push_back({x + m_shift.x,y + m_shift.y,z + m_shift.z, 0, 0,unsigned long long int(timeStamp)});
    auto item = m_scan->trajectory.pos.at(trajectory_pos_size);
    //std::cout << "x: "<<x<<"; y: "<<y<<"; z: "<<z<<"; " <<" timeStamp  " << timeStamp << '\n';
    std::cout << "trajectory.pos[" << trajectory_pos_size << "]" << "x: " << item.x << "; y: " << item.y
               << "; z: " << item.z << "; " << " timeStamp  " << item.timeStamp << '\n';
}

void realtime::RTScanCollector::
audioData(size_t startpositionOfChunk, size_t sizeOfChunk, std::time_t timeStampNewData, std::time_t timeStampFromChunk) {
    std::cout << "soundDataSize : " << m_scan->sound.samples.size() << " "
        << "trajectoryDataSize : " << m_scan->trajectory.pos.size() << '\n';

    std::cout << "audioData [" << sourceScanChunks->chunks.size() <<"]: " << "timeStampNewData: " << timeStampNewData << " "
        << "timeStampFromSound: " << timeStampFromChunk << '\n';
    ::SourceScanChunk localchunk;
    localchunk.timestamp = timeStampNewData;
    localchunk.timestampFromSound = timeStampFromChunk;
    localchunk.startpositionOfChunk = startpositionOfChunk;
    localchunk.sizeOfChunk = sizeOfChunk;
    sourceScanChunks->chunks.push_back(localchunk);

}

void realtime::RTScanCollector::save(BackgroundTaskExecutor& taskExecutor) {
    saveAs(taskExecutor);
}

void realtime::RTScanCollector::saveAs(BackgroundTaskExecutor& taskExecutor) {
    auto newPathname = QFileDialog::getSaveFileName(this, "Сохранить скан в файл", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        Core::BinaryPersistentVariable<ScanArm > binaryScan{ newPathname.toStdString() };
        binaryScan = *m_scan;
        binaryScan.save();


        try {
            taskExecutor.enqueue(new SaveScanTask("d:\\dump Stuchalka 2024\\exp.csp", *ScanArmToScan(m_scan), LineEncoding::Float));
        }
        catch (DbException& exc) {
            QMessageBox::critical(this, "Ошибка", exc.what());
        }
    }
    

}

void realtime::RTScanCollector::load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea) {
    auto newPathname = QFileDialog::getOpenFileName(this, "Открыть скан", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        auto ew = new realtime::RTScanCollector{ m_rtCtxt, this->m_parameters};
        ew->setAttribute(Qt::WA_DeleteOnClose, true);
        mdiArea->addSubWindow(ew);
        ew->showMaximized();

        //Core::BinaryPersistentVariable<OriginalData > binaryScan{ newPathname.toStdString() };
        Core::BinaryPersistentVariable<ScanArm > binaryScan{ newPathname.toStdString() };
        try {
            binaryScan.load();
        }
        catch (...) {
            QMessageBox::critical(this, "Ошибка", "Ошибка при загрузке *.ask файла");
            //auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
            //if (msg) {
            //    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
            //}
            //else {
            //    QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
            //}
        }
        ew->m_scan->sound.samples = std::move(binaryScan->sound.samples);
        ew->m_scan->sound.sampleRate = binaryScan->sound.sampleRate;
        ew->m_scan->trajectory.pos = std::move(binaryScan->trajectory.pos);
        ew->m_scan->trajectory.sampleRate = binaryScan->trajectory.sampleRate;
        //ew->m_scan->rtPeaks = std::move(binaryScan->peaks);
        ew->m_scan->rtPeaks = std::move(binaryScan->rtPeaks);
        ew->m_scan->parameters = binaryScan->parameters;

        m_field->findPeak();
        m_field->timeout();
        m_field->drawArea();
        //m_field->runTimers();
        //m_isThisStarted = m_isStarted = true;

        // отобразим анализ загруженного .ask скана
        //{
        //        //auto scan = std::make_shared<Scan>();
        //        //auto ew = new ScanDisplayWindow(scan);
        //        //ew->setAttribute(Qt::WA_DeleteOnClose);
        //        //mdiArea->addSubWindow(ew);
        //        //ew->showMaximized();
        //    try {
        //        taskExecutor.enqueue(new LoadScanTask(pathname, scanFactory, *processingParameters));
        //    }
        //    catch (DbException& exc) {
        //        QMessageBox::critical(this, "Ошибка", exc.what());
        //    }
        //    catch (uts::Exception& exc) {
        //        auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
        //        if (msg) {
        //            QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
        //        }
        //        else {
        //            QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
        //        }
        //    }
        //    scanFactory->startNewScan(this->m_parameters);
        //    ::std::vector< ::SourceScanLine > lines;
        //    auto linesSize = m_scan->lines.size();
        //    //emit stageStarted("Чтение строк", linesSize);
        //    for (std::uint32_t i = 0; i < linesSize; i++) {
        //        scanFactory->addRangeScanLine(m_scan->lines.at(i));
        //    }
        //    //emit finished();
        //    scanFactory->finishScan(boost::optional<QString&>(newPathname));
        //}

    }

}

realtime::RTScanCollector::
~RTScanCollector() {
    stop();
}