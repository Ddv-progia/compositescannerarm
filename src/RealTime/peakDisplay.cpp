#include "peakDisplay.h"
#include <QList>

#include <iostream>
#include <RealTime/onewavewidget.h>

using namespace realtime;

PeakDisplay::PeakDisplay()
{
    //

    //QString titleX = QString::fromLocal8Bit("time, миллисек.");
    QString titleX = QString::fromLocal8Bit("time");
    m_oneWave = new  OneWaveWidget("Peak Display", titleX);
    m_oneWave->setRangeFrequency(0, 1000);                //TODO magic number 1, 501
    m_oneWave->setRangeAmplitude(-1200, 1200, 1000);     //TODO magic number -1000, 1000, 1000

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addWidget(m_oneWave);
    setLayout(m_mainLayout);

    m_updateTimer = std::make_unique<QTimer>();
    //m_updateTimer->start(100);
    m_updateTimer->start(300);
    connect(m_updateTimer.get(), &QTimer::timeout, this, &PeakDisplay::update);
}

void PeakDisplay::
setScan(std::shared_ptr<ScanArm> scan) {
    m_scanArm = scan;
    const auto &parameters = m_scanArm->parameters;
    const auto& backStep = m_scanArm->sound.sampleRate * parameters.peakBackstep;
    const auto& foreStep = m_scanArm->sound.sampleRate * parameters.headAndScanCollectorParameters.peakForestepSound;
    //const auto& backStep = m_scanArm->sound.sampleRate * parameters.peakBackstep * 10.0; //TODO magic number 10
    //const auto& foreStep = m_scanArm->sound.sampleRate * parameters.peakForestep * 10.0;
    m_oneWave->setRangeFrequency(0, (backStep + foreStep) );
    //m_oneWave->setRangeFrequency(-parameters.peakBackstep*1000, parameters.peakForestep*1000); // Пределы по оси - миллисекунды до Максимума пика (-peakBackstep;peakForestep)
    //m_chartView->setPeakMagnitude(m_scanArm->parameters.peakMagnitudeLimit);
}

PeakDisplay::
~PeakDisplay(){

}



void PeakDisplay::update(){
    if (!m_scanArm||(m_scanArm->trajectory.pos.size()<1))
        return;
    try {
        const auto &lastPoint = m_scanArm->trajectory.pos.back();
        if (m_scanArm->rtPeaks.at(lastPoint.y).at(lastPoint.x).empty())
            return;

        auto sampl = m_scanArm->sound.samples;
        auto lastPeak = m_scanArm->rtPeaks.at(lastPoint.y).at(lastPoint.x).back();

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
