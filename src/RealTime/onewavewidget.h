#ifndef ONEWAVEWIDGET_H
#define ONEWAVEWIDGET_H

#include <QtCharts/QtCharts>
#include "Core/ScanData.hh"

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
    float m_dividerFrequency = 1;
    float m_dividerAmplitude = 1;
public:
    OneWaveWidget();
    ~OneWaveWidget();
    void update(const QList<QPointF> &newPoints);
    void setRangeFrequency(int min = 0, int max = 1000, float divider = 1);
    void setRangeAmplitude(int min = 0, int max = 1000, float divider = 1);
public slots:
    void changeVerticalCoord(int downValue ,int upValue);
    void changeHorizontalCoord(int downValue ,int upValue);
    //void redraw();
};

#endif // ONEWAVEWIDGET_H
