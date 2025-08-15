#pragma once

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

void realtime::RTScanCollector::
resizeRtPeaks(int width, int height) {
    for (auto& linePeak : m_scanArm->rtPeaks) {
        for (auto& peaks : linePeak)
            peaks.resize(0);
        linePeak.resize(0);
    }
    m_scanArm->rtPeaks.resize(height);
    for (auto& linePeak : m_scanArm->rtPeaks)
        linePeak.resize(width);
}
Q_SLOT void realtime::RTScanCollector::setAreaAdditionalScale(double value)
{
    m_field->setAreaAdditionalScale(value);
    //return Q_SLOT void();
}
realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ProcessingParameters& parameters, ScanFactory& scanFactory)
    : m_rtCtxt(trCtxt), m_parameters(parameters), scanFactory(scanFactory)
{
    m_soundDisplay = new SoundDisplay();
    m_peakDisplay = new PeakDisplay();
    m_field = new FieldWidget(m_parameters.headAndScanCollectorParameters.width, m_parameters.headAndScanCollectorParameters.height, m_parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox);
    auto leftFrame = new QFrame();
    //m_field->group->setParent(leftFrame);
    auto leftFrameLayout = new QVBoxLayout;
    QList<QAbstractButton*> buttonList = m_field->group->buttons();
    for (QList<QAbstractButton*>::const_iterator it = buttonList.cbegin(); it != buttonList.cend(); ++it)
    {
        leftFrameLayout->addWidget(*it);
    }
    //auto scaleFrame = new QFrame();
    //auto scaleFrameLayout = new QVBoxLayout;
    auto scaleSpinBox = new QDoubleSpinBox();
    scaleSpinBox->setMaximum(100.0);
    scaleSpinBox->setMinimum(0.001);
    scaleSpinBox->setSingleStep(0.01);
    scaleSpinBox->setValue(1.0);
    connect(scaleSpinBox, &QDoubleSpinBox::valueChanged,this,  &realtime::RTScanCollector::setAreaAdditionalScale);

    //scaleFrameLayout->addWidget(scaleSpinBox);
    //scaleFrame->setLayout(scaleFrameLayout);
    //leftFrameLayout->addWidget(scaleFrame);
    leftFrameLayout->addWidget(scaleSpinBox);
    leftFrame->setLayout(leftFrameLayout);
    auto mainLayout = new QVBoxLayout;
    auto topPanelLayout = new QHBoxLayout;
    auto splitterTopPanelVsField   = new QSplitter(Qt::Vertical);
    auto splitterSoundDisplayVsPeakDisplay = new QSplitter(Qt::Horizontal);
    splitterSoundDisplayVsPeakDisplay->addWidget(leftFrame);
    splitterSoundDisplayVsPeakDisplay->addWidget(m_soundDisplay);
    splitterSoundDisplayVsPeakDisplay->addWidget(m_peakDisplay);
    splitterSoundDisplayVsPeakDisplay->setStretchFactor(0, 1);
    splitterSoundDisplayVsPeakDisplay->setStretchFactor(1, 5);
    splitterSoundDisplayVsPeakDisplay->setStretchFactor(2, 2);

    splitterTopPanelVsField->addWidget(splitterSoundDisplayVsPeakDisplay);
    splitterTopPanelVsField->addWidget(m_field);
    splitterTopPanelVsField->setStretchFactor(0, 1);
    splitterTopPanelVsField->setStretchFactor(1, 5);

    mainLayout->addWidget(splitterTopPanelVsField);
    setLayout(mainLayout);

    m_scanArm = std::make_shared<ScanArm>();
    m_scanArm->parameters = m_parameters;
    m_scanArm->sound.sampleRate = m_parameters.headAndScanCollectorParameters.soundsSampleRate;
    m_scanArm->sound.samples.reserve(m_scanArm->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);
    m_scanArm->trajectory.sampleRate = m_parameters.headAndScanCollectorParameters.headsSampleRate;
    m_scanArm->trajectory.pos.reserve(m_scanArm->trajectory.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);
    resizeRtPeaks(m_parameters.headAndScanCollectorParameters.width, m_parameters.headAndScanCollectorParameters.height);
    m_scanArm->sourceScanChunks.chunks.reserve(0.5 * m_scanArm->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);
    sourceScanChunks = std::make_shared<SourceScanChunks>(m_scanArm->sourceScanChunks);

    m_soundDisplay->setScan(m_scanArm);
    m_peakDisplay->setScan(m_scanArm);
    m_field->setScan(m_scanArm);
    m_field->setScanChunks(sourceScanChunks);
}

realtime::RTScanCollector::
RTScanCollector(RTContext& trCtxt, ScanFactory& scanFactory, ScanArm scanArm)
    : m_rtCtxt(trCtxt), scanFactory(scanFactory)
{
    auto mainLayout = new QVBoxLayout;
    auto splitter   = new QSplitter(Qt::Vertical);
    
    setLayout(mainLayout);
    m_soundDisplay = new SoundDisplay();
    m_peakDisplay = new PeakDisplay();
    m_scanArm = std::make_shared<ScanArm>(scanArm);
    if (m_scanArm->shiftIsCorrect) {
        m_shift.x = m_scanArm->shiftX;
        m_shift.y = m_scanArm->shiftY;
        m_shift.z = m_scanArm->shiftZ;
        m_shift.is_correct = m_scanArm->shiftIsCorrect;

    }
    m_parameters = m_scanArm->parameters;
    m_field = new FieldWidget(m_parameters.headAndScanCollectorParameters.width, m_parameters.headAndScanCollectorParameters.height, m_parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox);
    splitter->addWidget(m_soundDisplay);
    splitter->addWidget(m_peakDisplay);
    splitter->addWidget(m_field);
    mainLayout->addWidget(splitter);

    sourceScanChunks = std::make_shared<SourceScanChunks>();
    sourceScanChunks->chunks.reserve(0.5 * m_scanArm->sound.sampleRate * SEC_PER_MINUTE * m_parameters.headAndScanCollectorParameters.maximumTimeMinutes);

    m_soundDisplay->setScan(m_scanArm);
    m_peakDisplay->setScan(m_scanArm);
    m_field->setScan(m_scanArm);
    m_field->setScanChunks(sourceScanChunks);
    m_field->findPeak();
    m_field->timeout();
}

void realtime::RTScanCollector::
startCursor() {
    if (m_isThisCursorStarted)
        return;
    if (auto head = std::dynamic_pointer_cast<realtime::RTHead>(m_rtCtxt.getRTDevice("APLHead"))) {
        connect(head.get(), &realtime::RTHead::newData, this, &realtime::RTScanCollector::headData);
        //connect(head.get(), &realtime::RTHead::newData, head.get(), &realtime::RTHead::onNewData);

        head->start(m_scanArm->trajectory.sampleRate);
    }
    m_field->runCursorTimer();
    m_isThisCursorStarted = true;
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
    m_isThisCursorStarted = false;
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
        
        m_scanArm->shiftX = m_shift.x;
        m_scanArm->shiftY = m_shift.y;
        m_scanArm->shiftZ = m_shift.z;
        m_scanArm->shiftIsCorrect = true;

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

    this->loadLastOpenDir();

    auto newPathname = QFileDialog::getOpenFileName(this, "Открыть скан", lastOpenDir, "скан (*.ask)");
    if (!newPathname.isEmpty()) {
        //Core::BinaryPersistentVariable<OriginalData > binaryScan{ newPathname.toStdString() };
        lastOpenDir = QFileInfo(newPathname).dir().path();
        //settings->setValue(QString::fromUtf8("lastOpenDir"), lastOpenDir);
        this->saveLastOpenDir();
        //emit started("Обработка скана", stagesNumber);
        //case ScanProcessingStage::RawDataObtained:
        //    emit stageStarted("Выравнивание строк - 1", scan->lines.size());

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
        binaryScan->scanName = newPathname.toStdString();
   
        //auto ew = new realtime::RTScanCollector{ m_rtCtxt, this->m_parameters, scanFactory};
        auto ew = new realtime::RTScanCollector{ m_rtCtxt, scanFactory, *binaryScan };
        ew->setWindowTitle(newPathname);
        ew->setAttribute(Qt::WA_DeleteOnClose, true);
        mdiArea->addSubWindow(ew);
        ew->showMaximized();
        //*m_scanArm = *binaryScan;
        //ew->m_scanArm->parameters = binaryScan->parameters;
        //ew->m_scanArm->sound.sampleRate = binaryScan->sound.sampleRate;
        //ew->m_scanArm->sound.samples.reserve(binaryScan->sound.samples.size());
        //ew->m_scanArm->sound.samples = std::move(binaryScan->sound.samples);
        //ew->m_scanArm->trajectory.sampleRate = binaryScan->trajectory.sampleRate;
        //ew->m_scanArm->trajectory.pos.reserve(binaryScan->trajectory.pos.size());
        //ew->m_scanArm->trajectory.pos = std::move(binaryScan->trajectory.pos);
        //ew->m_scanArm->rtPeaks.reserve(binaryScan->rtPeaks.size());
        ////std::memcpy(&m_scanArm->rtPeaks,&binaryScan->rtPeaks, sizeof binaryScan->rtPeaks);
        //ew->m_scanArm->rtPeaks = std::move(binaryScan->rtPeaks);
        //ew->m_soundDisplay->setScan(ew->m_scanArm);
        //ew->m_peakDisplay->setScan(ew->m_scanArm);
        ////m_field->setScan(m_scanArm);
        ////m_field->findPeak();
        ////m_field->timeout();
        ////m_field->drawArea();
        //////m_field->runTimers();
        //////m_isThisStarted = m_isStarted = true;

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
ScanArmToScan(std::shared_ptr<ScanArm> scanArm, std::shared_ptr<Scan> &scanIn)
{
    //std::shared_ptr<Scan> scan;
    scanIn = std::make_shared<Scan>();
    scanIn->parameters.initialSkip =                  scanArm->parameters.initialSkip;
    scanIn->parameters.stepForSplitFrequencyRanges =  scanArm->parameters.stepForSplitFrequencyRanges;
    scanIn->parameters.peakMagnitudeLimit =           scanArm->parameters.peakMagnitudeLimit;
    scanIn->parameters.peakBackstep =                 scanArm->parameters.peakBackstep;
    scanIn->parameters.peakForestep =                 scanArm->parameters.peakForestep;
    scanIn->parameters.peakPauseCount =               scanArm->parameters.peakPauseCount;
    scanIn->parameters.headAndScanCollectorParameters.peakForestepSound            = scanArm->parameters.headAndScanCollectorParameters.peakForestepSound;
    scanIn->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox = scanArm->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox;
    scanIn->parameters.headAndScanCollectorParameters.currentNumArea =  scanArm->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox;
    scanIn->parameters.headAndScanCollectorParameters.firstStepShift =  scanArm->parameters.headAndScanCollectorParameters.firstStepShift;
    scanIn->parameters.headAndScanCollectorParameters.needPackInLine = scanArm->parameters.headAndScanCollectorParameters.needPackInLine;
    scanIn->parameters.headAndScanCollectorParameters.needShowTrajectory = scanArm->parameters.headAndScanCollectorParameters.needShowTrajectory;
    scanIn->parameters.headAndScanCollectorParameters.needShowCountOfPeak = scanArm->parameters.headAndScanCollectorParameters.needShowCountOfPeak;
    
    scanIn->parameters.headAndScanCollectorParameters.needIgnoreFirstLine = scanArm->parameters.headAndScanCollectorParameters.needIgnoreFirstLine;
    scanIn->parameters.headAndScanCollectorParameters.needTrimFirstLine = scanArm->parameters.headAndScanCollectorParameters.needTrimFirstLine;
    
    auto m_field_NumArea = m_field->getNumArea();
    scanIn->parameters.headAndScanCollectorParameters.currentNumArea = m_field_NumArea;
    m_parameters.headAndScanCollectorParameters.currentNumArea = m_field_NumArea;
    
    ::std::vector< ::FrequencyRange > ranges;

    //auto scanArmParametersRangesSize = scanArm->parameters.ranges.size();
    //for (std::size_t i = 0; i < scanArmParametersRangesSize; i++) {
    //    FrequencyRange frequencyRange;
    //    frequencyRange.from = scanArm->parameters.ranges[i].from;
    //    frequencyRange.to = scanArm->parameters.ranges[i].to;
    //    ranges.push_back(frequencyRange);
    //}

    //scanIn->parameters.ranges = ranges;
    if (m_parameters.ranges.size()>0) scanIn->parameters.ranges = m_parameters.ranges;
    else scanIn->parameters.ranges = constructCommonRanges();
    
    scanIn->parameters.extremumOfRanges =  m_parameters.extremumOfRanges;

    scanIn->parameters.specNormalization = m_parameters.specNormalization;

    scanIn->parameters.colorStopsList = m_parameters.colorStopsList;

    scanIn->parameters.defectClassification = m_parameters.defectClassification;

    scanIn->parameters.defectPoints = m_parameters.defectPoints;

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

    scanIn->parameters.columnModelOrder = 0;

    auto rtPeaksSizeY = scanArm->rtPeaks.size();
    const auto& sound = scanArm->sound;

    //if (this->m_parameters.headAndScanCollectorParameters.needPackInSquare) {
    if (scanArm->parameters.headAndScanCollectorParameters.needPackInSquare) {
        // разбивка на строки
        size_t maxPeakLenght = 0;
        ::std::vector< ::std::vector<  size_t > > peaksLenght;
        for (std::size_t i = 0; i < rtPeaksSizeY; i++) {
            size_t lineLenght = 0;
            ::std::vector< size_t  >  peaksLenghtLine;
            auto rtPeaksSizeX = scanArm->rtPeaks.at(i).size();
            for (std::size_t j = 0; j < rtPeaksSizeX; j++) {
                auto peaks = scanArm->rtPeaks.at(i).at(j);
                auto peaksSize = peaks.size();
                unsigned int peaksLenght = 0;
                for (auto peak : peaks) {
                    peaksLenght += (peak.endIndex - peak.beginIndex);
                }
                if (peaksLenght > maxPeakLenght) maxPeakLenght = peaksLenght;
                peaksLenghtLine.push_back(peaksLenght );
            }
            peaksLenght.push_back(peaksLenghtLine);
        }
        //***
        for (std::size_t i = 0; i < rtPeaksSizeY; i += m_field_NumArea) {
            SourceScanLine line;
            auto rtPeaksSizeX = scanArm->rtPeaks.at(i).size();
            line.startCoordinate = 0;
            line.finalCoordinate = rtPeaksSizeX;
            line.lineCoordinate = (rtPeaksSizeY-1)-i; //**********
            line.finalLineCoordinate = line.lineCoordinate;
            line.sampleRate = sound.sampleRate;
            for (std::size_t j = 0; j < rtPeaksSizeX; j+= m_field_NumArea) {
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
            scanIn->lines.push_back(line);
        }
        // конец разбивка на строки 
    }
    else if (scanArm->parameters.headAndScanCollectorParameters.needPackInLine) {
        //вместо разбивки на строки делаем в одну строку:
        SourceScanLine line;
        line.startCoordinate = 0;
        line.finalCoordinate = scanIn->parameters.headAndScanCollectorParameters.width * scanIn->parameters.headAndScanCollectorParameters.height;
        line.lineCoordinate = 0; //**********
        line.finalLineCoordinate = 1; // высоту строки делаем = 1;
        //line.finalLineCoordinate = line.lineCoordinate;
        line.sampleRate = sound.sampleRate;
        line.samples.insert(line.samples.end(), sound.samples.begin(), sound.samples.end());
        scanIn->lines.push_back(line);
        //конец вместо разбивки на строки делаем в одну строку:
    }
    scanIn->scanArm = *scanArm.get();
    scanIn->scanArm.isScanArmReady = true;
}

void realtime::RTScanCollector::SetShift(float x, float y, float z)
{
    m_shift.x = x;
    m_shift.y = y;
    m_shift.z = z;
    m_shift.is_correct = true;
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
        ScanArmToScan(scanArmIn, m_scan);
        for (auto newLine: m_scan->lines) {
            scanFactory.addRangeScanLine(newLine);
        }
        //scanFactory.finishScan(QString("nameOfScan"));
        QString scanName = "nameOfScan";
        if (scanArmIn->scanName != "") scanName = QString::fromUtf8( scanArmIn->scanName);
        scanFactory.finishScan(m_scan, boost::optional<QString&>(scanName));
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

Q_SLOT void realtime::RTScanCollector::makeScanAndShowCuttered(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea)
{
    ::Sound  sound;
    sound.sampleRate = scanArmIn->sound.sampleRate;
    for (auto s: scanArmIn->sound.samples) {
        if (s > scanArmIn->parameters.peakMagnitudeLimit) 
            sound.samples.push_back(scanArmIn->parameters.peakMagnitudeLimit);
        else
            sound.samples.push_back(s);

    }
    scanArmIn->sound = sound;
    scanArmIn->scanName = scanArmIn->scanName + " cutted";
    makeScanAndShow(scanArmIn, taskExecutor, scanFactory, mdiArea);
    return Q_SLOT void();
}

Q_SLOT void realtime::RTScanCollector::makeScanAndShowCutteredAbs(std::shared_ptr<ScanArm> scanArmIn, BackgroundTaskExecutor& taskExecutor, ScanFactory& scanFactory, QMdiArea* mdiArea)
{
    ::Sound  sound;
    sound.sampleRate = scanArmIn->sound.sampleRate;
    for (auto s: scanArmIn->sound.samples) {
        if (std::abs(s) > scanArmIn->parameters.peakMagnitudeLimit) {
            if (s < 0) {
              sound.samples.push_back(-1.5*scanArmIn->parameters.peakMagnitudeLimit);

            }
            else {
                sound.samples.push_back(scanArmIn->parameters.peakMagnitudeLimit);

            }
        }
        else
            sound.samples.push_back(s);
    }
    scanArmIn->sound = sound;
    scanArmIn->scanName = scanArmIn->scanName + " cutted Abs";
    makeScanAndShow(scanArmIn, taskExecutor, scanFactory, mdiArea);
    return Q_SLOT void();
    //std::shared_ptr<::Sound  >sound = std::make_shared<::Sound>();
    //sound->sampleRate = scanArmIn->sound.sampleRate;
    //for (auto s: scanArmIn->sound.samples) {
    //    if (std::abs(s) > scanArmIn->parameters.peakMagnitudeLimit) {
    //        if (s < 0) {
    //          sound->samples.push_back(-1.5*scanArmIn->parameters.peakMagnitudeLimit);

    //        }
    //        else {
    //            sound->samples.push_back(scanArmIn->parameters.peakMagnitudeLimit);

    //        }
    //    }
    //    else
    //        sound->samples.push_back(s);
    //}
    //scanArmIn->sound = *sound;
    //scanArmIn->scanName = scanArmIn->scanName + " cutted Abs";
    //makeScanAndShow(scanArmIn, taskExecutor, scanFactory, mdiArea);
    //return Q_SLOT void();
}

realtime::RTScanCollector::
~RTScanCollector() {
    stop();
}