#ifndef ONEWAVEWIDGET_H
#define ONEWAVEWIDGET_H

#include <QtCharts/QtCharts>
#include "Core/ScanData.hh"


class ICurve;
class AGraphicItem;
class ModelOneWave;
class ChartViewForOneWaveWidget;
class QxtSpanSlider;

class OneWaveWidget : public QWidget
{
    Q_OBJECT

    ChartViewForOneWaveWidget *m_chartView;
    QLineSeries* m_series;
    QValueAxis *xAxis;
    QValueAxis *yAxis;
    QSplitter *m_hSplitter;
    QxtSpanSlider *m_sliderAmplitude, *m_sliderFrequency;
    QVBoxLayout *m_vLayout;
    QGridLayout *m_graphicGridLayout;
    QWidget *m_graphicsWidget;
    std::unique_ptr<QTimer> m_updateTimer;
    std::shared_ptr<Scan> m_scan = nullptr;

public:
    OneWaveWidget();
    ~OneWaveWidget();
    void setScan(std::shared_ptr<Scan> scan);
    void update(const QList<QPointF> &newPoints);

public slots:
    void changeVerticalCoord(int downValue ,int upValue);
    void changeHorizontalCoord(int downValue ,int upValue);
    void redraw();

};

#endif // ONEWAVEWIDGET_H
