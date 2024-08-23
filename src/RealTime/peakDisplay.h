#ifndef PEAK_DISPLAY_H
#define PEAK_DISPLAY_H

#include "Core/ScanData.hh"
#include <QWidget.h>

class OneWaveWidget;
class QVBoxLayout;
namespace realtime {
    
    class PeakDisplay : public QWidget
    {
        Q_OBJECT

        OneWaveWidget* m_oneWave;
        QVBoxLayout* m_mainLayout;

        std::unique_ptr<QTimer> m_updateTimer;
        std::shared_ptr<ScanArm> m_scanArm = nullptr;

    public:
        PeakDisplay();
        ~PeakDisplay();
        void setScan(std::shared_ptr<ScanArm> scan);

    public slots:
        void update();
    };
};
#endif // PEAK_DISPLAY_H
