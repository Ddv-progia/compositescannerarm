#include "soundDisplay.h"
#include <QList>
#include <iostream>
#include<RealTime/onewavewidget.h>


using namespace realtime;

SoundDisplay::SoundDisplay()
{
    m_oneWave = new  OneWaveWidget("Sound Display");
    m_oneWave->setRangeFrequency(-10, 510);
    m_oneWave->setRangeAmplitude(-1500, 1500, 1000);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_oneWave);
    setLayout(m_mainLayout);

    m_updateTimer = std::make_unique<QTimer>();
    m_updateTimer->start(100);
    connect(m_updateTimer.get(), &QTimer::timeout, this, &SoundDisplay::update);
}

void SoundDisplay::
setScan(std::shared_ptr<ScanArm> scan) {
    m_scan = scan;
    //m_chartView->setPeakMagnitude(m_scan->parameters.peakMagnitudeLimit);
}

SoundDisplay::
~SoundDisplay(){

}


void SoundDisplay::update(){
    if (!m_scan)
        return;
    QList<QPointF> points;
    //auto &sampl = m_scan->sound.samples;
    ////QVector<QPoint> points{ 5000 < sampl.size() ? 5000 : sampl.size() };
    ////int samplPerSecond = m_scan->originalScan.sound.sampleRate;
    int x = 0;
    auto size_local = m_scan->sound.samples.size(); 
    //int indBegin = 50000 < size_local ? 50000 : sampl.size();
    int indBegin = 50000 < size_local ? 50000 : size_local ;
    for (int ind = indBegin, x = 0; ind > 0; --ind /*-= 100*/) {
        //points.push_back({ QPointF(float(x++) * 0.01, sampl[sampl.size() - ind]) });
        points.push_back({ QPointF(float(x++) * 0.01, m_scan->sound.samples[size_local - ind]) });
    }
    m_oneWave->update(points);
}
