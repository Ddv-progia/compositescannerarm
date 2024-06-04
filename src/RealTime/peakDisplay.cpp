#include "peakDisplay.h"
#include <QList>
#include <iostream>
#include <RealTime/onewavewidget.h>


using namespace realtime;

PeakDisplay::PeakDisplay()
{
    m_oneWave = new  OneWaveWidget;
    m_oneWave->setRangeFrequency(-1, 501);
    m_oneWave->setRangeAmplitude(-1000, 1000, 1000);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_oneWave);
    setLayout(m_mainLayout);

    m_updateTimer = std::make_unique<QTimer>();
    m_updateTimer->start(100);
    connect(m_updateTimer.get(), &QTimer::timeout, this, &PeakDisplay::update);
}

void PeakDisplay::
setScan(std::shared_ptr<Scan> scan) {
    m_scan = scan;
    const auto &parameters = m_scan->parameters;
    const auto& backStep = m_scan->sound.sampleRate * parameters.peakBackstep * 10.0;
    const auto& foreStep = m_scan->sound.sampleRate * parameters.peakForestep * 10.0;
    m_oneWave->setRangeFrequency(-1, (backStep + foreStep) + 1);
    //m_chartView->setPeakMagnitude(m_scan->parameters.peakMagnitudeLimit);
}

PeakDisplay::
~PeakDisplay(){

}



void PeakDisplay::update(){
    if (!m_scan)
        return;
    try {
        const auto &lastPoint = m_scan->trajectory.pos.back();
        if (m_scan->rtPeaks.at(lastPoint.y).at(lastPoint.x).empty())
            return;

        auto sampl = m_scan->sound.samples;
        auto lastPeak = m_scan->rtPeaks.at(lastPoint.y).at(lastPoint.x).back();

        QList<QPointF> points;
        int x = 0;
        int indBegin = lastPeak.beginIndex < sampl.size() - 1 ? lastPeak.beginIndex : sampl.size() - 1;
        int indEnd = lastPeak.endIndex < sampl.size() - 1 ? lastPeak.endIndex : sampl.size() - 1;

        for (int ind = indBegin, x = 0; ind < indEnd; ++ind) {
            points.push_back({ QPointF(float(x++) * 10.0, sampl.at(ind)) });
        }
        m_oneWave->update(std::move(points));
    }catch(...){}
}
