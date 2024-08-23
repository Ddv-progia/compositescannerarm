#include "RTScanCollector.h"
#include "RTAudioCollector.h"
#include "RTHead.h"
#include <array>
#include <QtWidgets/qboxlayout.h>
#include "soundDisplay.h"
#include "peakDisplay.h"
#include "Core/BinaryPersistentVariable.hh"
#include "Core/BinaryTransformation.hh"
#include "Core/Devices.hh"
#include <Core/DevicesConfigurationReflection.hh>
#include <Core/ScanDataReflection.hh>
#include "Core/ScanAlgorithms.hh"
#include <Gui/ScanDisplayWindow.hh>

//#include "Core/ProgressReportingTask.hh"
#include "Core/SaveScanTask.hh"
#include "Core/LineEncoding.hh"

bool realtime::RTScanCollector::m_isStarted = false;

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters, ScanFactory& scanFactory)
    : m_rtCtxt(trCtxt), m_parameters(parameters), scanFactory(scanFactory)
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

    m_scanArm = std::make_shared<ScanArm>();
    m_scanArm->parameters = m_parameters;
    m_scanArm->sound.sampleRate = m_parameters.headAndScanCollectorParameters.soundsSampleRate;
    m_scanArm->sound.samples.reserve(m_scanArm->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);
    m_scanArm->trajectory.sampleRate = m_parameters.headAndScanCollectorParameters.headsSampleRate;
    m_scanArm->trajectory.pos.reserve(m_scanArm->trajectory.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);

    sourceScanChunks = std::make_shared<SourceScanChunks>();
    sourceScanChunks->chunks.reserve(0.5 * m_scanArm->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);

    m_soundDisplay->setScan(m_scanArm);
    m_peakDisplay->setScan(m_scanArm);
    m_field->setScan(m_scanArm);
    m_field->setScanChunks(sourceScanChunks);
}

void realtime::RTScanCollector::
start() {
    if (m_isStarted)
        return;
    //if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
    //    connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
    //    head->start(m_scanArm->trajectory.sampleRate);
    //}
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        //connect(head.get(), &realtime::RTHead::newData, head.get(), &realtime::RTHead::onNewData);

        head->start(m_scanArm->trajectory.sampleRate);
    }
    if (auto col = std::dynamic_pointer_cast<realtime::RTAudioCollector>(m_rtCtxt.getRTDevice("AudioDataCollector"))) {
        connect(col.get(), &realtime::RTAudioCollector::newData, this, &realtime::RTScanCollector::audioData);
        col->start(&m_scanArm->sound);
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
        //disconnect(head.get(), &realtime::RTHead::newData, head.get(), &realtime::RTHead::onNewData);
        head->stop();
    }
    m_field->stopTimers();
    emit ready(m_scanArm);

}

void realtime::RTScanCollector::
headData(float x, float y, float z, time_t timeStamp) {
    if (std::abs(timeStamp - m_positionLast.m_timeStampLast) < m_minimumTimeStampProlong) {
        return;
    }
    else {
        m_positionLast.m_timeStampLast = timeStamp;
    }

    if (!m_shift.is_correct) {
        //m_shift.x = -x;
        //m_shift.y = -y;
        //m_shift.x = (m_parameters.headAndScanCollectorParameters.width / 2) - x;
        //m_shift.y = (m_parameters.headAndScanCollectorParameters.height / 2) - y;
        m_shift.x =  - x;
        m_shift.y = (m_parameters.headAndScanCollectorParameters.height) - y;
        m_shift.z = -z;
        m_scanArm->sound.samples.clear();
        //m_scanArm->trajectory.pos.clear();
        m_shift.is_correct = true;
    }
    auto trajectory_pos_size = m_scanArm->trajectory.pos.size();
    m_scanArm->trajectory.pos.push_back({x + m_shift.x,y + m_shift.y,z + m_shift.z, 0, 0,unsigned long long int(timeStamp)});
    auto item = m_scanArm->trajectory.pos.at(trajectory_pos_size);
    //std::cout << "x: "<<x<<"; y: "<<y<<"; z: "<<z<<"; " <<" timeStamp  " << timeStamp << '\n';
    //std::cout << "trajectory.pos[" << trajectory_pos_size << "]" << "x: " << item.x << "; y: " << item.y
    //           << "; z: " << item.z << "; " << " timeStamp  " << item.timeStamp << '\n';
}

void realtime::RTScanCollector::
audioData(size_t startpositionOfChunk, size_t sizeOfChunk, std::time_t timeStampNewData, std::time_t timeStampFromChunk) {
    //std::cout << "soundDataSize : " << m_scanArm->sound.samples.size() << " "
    //    << "trajectoryDataSize : " << m_scanArm->trajectory.pos.size() << '\n';
    //std::cout << "audioData [" << sourceScanChunks->chunks.size() <<"]: " << "timeStampNewData: " << timeStampNewData << " "
    //    << "timeStampFromSound: " << timeStampFromChunk << '\n';
    ::SourceScanChunk localchunk;
    localchunk.timestamp = timeStampNewData;
    localchunk.timestampFromSound = timeStampFromChunk;
    localchunk.startpositionOfChunk = startpositionOfChunk;
    localchunk.endpositionOfChunk = startpositionOfChunk + sizeOfChunk;
    localchunk.sizeOfChunk = sizeOfChunk;
    sourceScanChunks->chunks.push_back(localchunk);
}

void realtime::RTScanCollector::save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea) {
    saveAs(taskExecutor, mdiArea);
}

void realtime::RTScanCollector::saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea) {
    auto newPathname = QFileDialog::getSaveFileName(this, "Сохранить скан в файл", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        Core::BinaryPersistentVariable<ScanArm > binaryScan{ newPathname.toStdString() };
        binaryScan = *m_scanArm;
        binaryScan.save();

    }
}

void realtime::RTScanCollector::load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea) {
    auto newPathname = QFileDialog::getOpenFileName(this, "Открыть скан", QString(), "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        auto ew = new realtime::RTScanCollector{ m_rtCtxt, this->m_parameters, scanFactory};
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
        *m_scanArm = *binaryScan;
        ew->m_scanArm->parameters = binaryScan->parameters;
        ew->m_scanArm->sound.sampleRate = binaryScan->sound.sampleRate;
        ew->m_scanArm->sound.samples.reserve(binaryScan->sound.samples.size());
        ew->m_scanArm->sound.samples = std::move(binaryScan->sound.samples);
        ew->m_scanArm->trajectory.sampleRate = binaryScan->trajectory.sampleRate;
        ew->m_scanArm->trajectory.pos.reserve(binaryScan->trajectory.pos.size());
        ew->m_scanArm->trajectory.pos = std::move(binaryScan->trajectory.pos);
        ew->m_scanArm->rtPeaks.reserve(binaryScan->rtPeaks.size());
        ew->m_scanArm->rtPeaks = std::move(binaryScan->rtPeaks);

        ew->m_soundDisplay->setScan(m_scanArm);
        ew->m_peakDisplay->setScan(m_scanArm);
        m_field->setScan(m_scanArm);
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
        //    auto linesSize = m_scanArm->lines.size();
        //    //emit stageStarted("Чтение строк", linesSize);
        //    for (std::uint32_t i = 0; i < linesSize; i++) {
        //        scanFactory->addRangeScanLine(m_scanArm->lines.at(i));
        //    }
        //    //emit finished();
        //    scanFactory->finishScan(boost::optional<QString&>(newPathname));
        //}
        
        //makeScanAndShow(ew->m_scanArm, taskExecutor, scanFactory, mdiArea);
    }

}

//std::shared_ptr<Scan> ScanCollector::ScanArmToScan(std::shared_ptr<ScanArm> scanArm)
void realtime::RTScanCollector::
ScanArmToScan(std::shared_ptr<ScanArm> scanArm)
{
    //std::shared_ptr<Scan> scan;
    m_scan = std::make_shared<Scan>();
    m_scan->parameters.initialSkip =                 m_parameters.initialSkip;
    m_scan->parameters.stepForSplitFrequencyRanges = m_parameters.stepForSplitFrequencyRanges;
    m_scan->parameters.peakMagnitudeLimit =          m_parameters.peakMagnitudeLimit;
    m_scan->parameters.peakBackstep =                m_parameters.peakBackstep;
    m_scan->parameters.peakForestep =                m_parameters.peakForestep;
    m_scan->parameters.peakPauseCount =              m_parameters.peakPauseCount;

    
    ::std::vector< ::FrequencyRange > ranges;

    //auto scanArmParametersRangesSize = scanArm->parameters.ranges.size();
    //for (std::size_t i = 0; i < scanArmParametersRangesSize; i++) {
    //    FrequencyRange frequencyRange;
    //    frequencyRange.from = scanArm->parameters.ranges[i].from;
    //    frequencyRange.to = scanArm->parameters.ranges[i].to;
    //    ranges.push_back(frequencyRange);
    //}

    //m_scan->parameters.ranges = ranges;
    if (m_parameters.ranges.size()>0) m_scan->parameters.ranges = m_parameters.ranges;
    else m_scan->parameters.ranges = constructCommonRanges();
    
    m_scan->parameters.extremumOfRanges =  m_parameters.extremumOfRanges;

    m_scan->parameters.specNormalization = m_parameters.specNormalization;

    m_scan->parameters.colorStopsList = m_parameters.colorStopsList;

    m_scan->parameters.defectClassification = m_parameters.defectClassification;

    m_scan->parameters.defectPoints = m_parameters.defectPoints;

    //putVal<std::uint32_t>(db, "parameters.defectPoints.@size", scan.parameters.defectPoints.size());
    //for (std::size_t i = 0; i < scan.parameters.defectPoints.size(); i++) {
    //    putStr(db, (boost::format("parameters.defectPoints.@%1%.title") % i).str(), scan.parameters.defectPoints[i].title);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.red.range") % i).str(), scan.parameters.defectPoints[i].red.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.limit") % i).str(), scan.parameters.defectPoints[i].red.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.amplification") % i).str(), scan.parameters.defectPoints[i].red.amplification);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.blue.range") % i).str(), scan.parameters.defectPoints[i].blue.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.limit") % i).str(), scan.parameters.defectPoints[i].blue.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.amplification") % i).str(), scan.parameters.defectPoints[i].blue.amplification);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.green.range") % i).str(), scan.parameters.defectPoints[i].green.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.limit") % i).str(), scan.parameters.defectPoints[i].green.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.amplification") % i).str(), scan.parameters.defectPoints[i].green.amplification);
    //}

    m_scan->parameters.columnModelOrder = 0;

    auto rtPeaksSizeY = scanArm->rtPeaks.size();
    const auto& sound = scanArm->sound;
    size_t maxPeaksLenght = 0;
    ::std::vector< ::std::vector<  size_t > > peaksLenght;
    for (auto line: scanArm->rtPeaks) {
        ::std::vector<  size_t > l;
        for (auto peaks : line) {
            size_t s = peaks.size();
            if (maxPeaksLenght < s) maxPeaksLenght = s;
            l.push_back(s);
        }
        peaksLenght.push_back(l);
    }

    for (std::size_t i = 0; i < rtPeaksSizeY; i++) {
        size_t lineLenght = 0;
        ::std::vector< size_t  >  peaksLengtLine;
        auto rtPeaksSizeX = scanArm->rtPeaks.at(i).size();
        for (std::size_t j = 0; j < rtPeaksSizeX; j++) {
            auto peaks = scanArm->rtPeaks.at(i).at(j);
            unsigned int peaksLenght = 0;
            for (auto peak : peaks) {
                peaksLenght += peak.endIndex - peak.beginIndex;
            }
            if (peaksLenght > maxPeakLenght) maxPeakLenght = peaksLenght;
            peaksLengtLine.push_back(peaksLenght);
        }
        peaksLenght.push_back(peaksLengtLine);
    }
    //***
    for (std::size_t i = 0; i < rtPeaksSizeY; i++) {
        SourceScanLine line;
        auto rtPeaksSizeX = scanArm->rtPeaks.at(i).size();
        line.startCoordinate = 0;
        line.finalCoordinate = rtPeaksSizeX;
        line.lineCoordinate = i;
        line.finalLineCoordinate = i;
        line.sampleRate = sound.sampleRate;


        for (std::size_t j = 0; j < rtPeaksSizeX; j++) {
            auto peaks = scanArm->rtPeaks.at(i).at(j);
            //const auto& lastPeak = peaks.back();
            std::size_t insertedLength = 0;
            for (auto peak : peaks) {
                line.samples.insert(line.samples.end(),
                    sound.samples.begin() + peak.beginIndex,
                    sound.samples.begin() + peak.endIndex);
            }
            auto insertMoreLength = maxPeakLenght - peaksLenght.at(i).at(j);
            line.samples.insert(line.samples.end(), insertMoreLength, 0.0);

        }
        m_scan->lines.push_back(line);
    }

    //auto rtPeaksSizeY = scanArm->rtPeaks.size();

    //auto linesSize = scanArm->lines.size();
    //for (std::size_t i = 0; i < linesSize; i++) {
    //    auto lineArm = scanArm->lines.at(i);
    //    SourceScanLine line;
    //    line.finalCoordinate = lineArm.finalCoordinate;
    //    line.finalLineCoordinate= lineArm.finalCoordinate;
    //    line.lineCoordinate= lineArm.lineCoordinate;
    //    line.sampleRate= lineArm.sampleRate;
    //    line.startCoordinate= lineArm.startCoordinate;
    //    line.timestampEnd= lineArm.timestampEnd;
    //    line.timestampStart= lineArm.timestampStart;
    //    for (std::size_t j = 0; j < lineArm.samples.size(); j++) {
    //        line.samples.push_back(lineArm.samples.at(j));
    //    }
    //    scan->lines.push_back(line);
    //}

    //return m_scan;
}

void realtime::RTScanCollector::
makeScanAndShow(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea)
{
    //ScanFactory::ScanFactory(BackgroundTaskExecutor & taskExecutor)
    //    : taskExecutor(taskExecutor)
    //void ScanFactory::startNewScan(const ProcessingParameters & newProcessingParameters)
    //{
    //    std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
    //    this->processingParameters = newProcessingParameters;
    //    rangeScanLines.clear();
    //}

    //void ScanFactory::addRangeScanLine(const SourceScanLine & newLine)
    //{
    //    std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
    //    rangeScanLines.push_back(newLine);
    //}

    //void ScanFactory::finishScan(boost::optional<QString&> name)
    //{
    //    std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
    //    try {
    //        auto scan = std::make_shared<Scan>();
    //        scan->processingStage = ScanProcessingStage::RawDataObtained;
    //        if (!name) {
    //            scan->scanName = "New scan";
    //            saveToTempDirectory(rangeScanLines);
    //        }
    //        else
    //            scan->scanName = name->toStdString();
    //        auto task = new ScanProcessingTask(rangeScanLines, processingParameters, scan);
    //        connect(task, SIGNAL(newScanReady(const std::shared_ptr<Scan>&)), this, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), Qt::DirectConnection);
    //        taskExecutor.enqueue(task);
    //    }
    //    catch (...) {
    //        throw std::exception("Error while finishing scan");
    //    }
    //}

    scanFactory.startNewScan(scanArmIn->parameters);

    //std::shared_ptr<Scan> scan;
    try {
        //std::shared_ptr<Scan> scan;
        //scan = ScanArmToScan(scanArmIn);
        ScanArmToScan(scanArmIn);
        for (auto newLine: m_scan->lines) {
            scanFactory.addRangeScanLine(newLine);
        }
        //scanFactory.finishScan(QString("nameOfScan"));
        QString scanName = "nameOfScan";
        scanFactory.finishScan(boost::optional<QString&>(scanName));
    }
    catch (...) {
        QMessageBox::critical(this, "Ошибка!", QString("Ошибка во время преобразования скана."),1,2);
        throw std::exception("Ошибка во время преобразования скана.");
    }
        //taskExecutor.enqueue(new SaveScanTask("d:\\dump Stuchalka 2024\\exp.csp", *scan, LineEncoding::Float));

        //auto sdw = new ScanDisplayWindow(ScanArmToScan(m_scanArm));
        //sdw->setAttribute(Qt::WA_DeleteOnClose, true);
        //mdiArea->addSubWindow(sdw);
        //sdw->showMaximized();
        ////connect(sdw, SIGNAL(ScanDisplayWindow::refreshScan(std::shared_ptr<Scan>&)), &scanFactory, SLOT(ScanDisplayWindow::recalculateScan(std::shared_ptr<Scan>&)));
}

realtime::RTScanCollector::
~RTScanCollector() {
    stop();
}