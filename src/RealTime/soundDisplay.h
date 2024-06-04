#ifndef SOUND_DISPLAY_H
#define SOUND_DISPLAY_H

#include <QtCharts/QtCharts>
#include "Core/ScanData.hh"

class OneWaveWidget;

namespace realtime {
    
    class SoundDisplay : public QWidget
    {
        Q_OBJECT

        OneWaveWidget* m_oneWave;
        QVBoxLayout* m_mainLayout;

        std::unique_ptr<QTimer> m_updateTimer;
        std::shared_ptr<Scan> m_scan = nullptr;

    public:
        SoundDisplay();
        ~SoundDisplay();
        void setScan(std::shared_ptr<Scan> scan);

    public slots:
        void update();
    };
};
#endif // SOUND_DISPLAY_H
