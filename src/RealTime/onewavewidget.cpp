#include "onewavewidget.h"
#include <QList>
//#include "customdelegates.h"

#include "chartvievforonewavewidget.h"
#include "qxtspanslider.h"
#include <iostream>


OneWaveWidget::OneWaveWidget(QString title ,QString xAxeTitle, QString yAxeTitle, Qt::GlobalColor m_penColor, Qt::GlobalColor m_xAxeColor, Qt::GlobalColor m_yAxeColor)
{
    this->setWindowTitle(title);
    m_hSplitter = new QSplitter;
    m_sliderAmplitude = new QxtSpanSlider(Qt::Vertical);

    m_sliderFrequency = new QxtSpanSlider(Qt::Horizontal);

    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_graphicGridLayout = new QGridLayout();
    m_graphicGridLayout->setContentsMargins(0, 0, 0, 0);
    m_graphicsWidget = new QWidget();

    xAxis = new QValueAxis;
    //xAxis->setTitleText(tr("Time"));
    xAxis->setTitleText(xAxeTitle);

    xAxis->setTitleBrush(m_xAxeColor);
    xAxis->setLabelsColor(m_xAxeColor);
    //xAxis->setTickCount(10);

    yAxis = new QValueAxis;

    //yAxis->setTitleText(tr("Amplitude"));
    yAxis->setTitleText(yAxeTitle);
    yAxis->setTitleBrush(m_yAxeColor);
    yAxis->setLabelsColor(m_yAxeColor);
    m_chartView = new ChartViewForOneWaveWidget();

//    m_chartView->chart()->setTheme(QChart::ChartThemeDark);
    m_chartView->chart()->setTheme(QChart::ChartThemeQt);
    m_series = new QLineSeries;
    m_series->setPen(QPen(m_penColor, 2));
    m_chartView->chart()->addSeries(m_series);
    m_chartView->chart()->setAxisX(xAxis, m_series);   // Назначить ось xAxis, осью X для diagramA
    m_chartView->chart()->setAxisY(yAxis, m_series);

    m_graphicGridLayout->addWidget(m_sliderAmplitude,0,0);
    m_graphicGridLayout->addWidget(m_sliderFrequency,1,1);
    m_graphicGridLayout->addWidget(m_chartView,0,1);

    m_graphicsWidget->setLayout(m_graphicGridLayout);

    m_hSplitter->addWidget(m_graphicsWidget);

    m_vLayout->addWidget(m_hSplitter);

    setLayout(m_vLayout);

    connect(m_sliderAmplitude,&QxtSpanSlider::spanChanged,this,&OneWaveWidget::changeVerticalCoord);
    connect(m_sliderFrequency,&QxtSpanSlider::spanChanged,this,&OneWaveWidget::changeHorizontalCoord);
}

OneWaveWidget::
~OneWaveWidget(){

}

void OneWaveWidget::update(const QList<QPointF> &newPoints){ //Добавляет точки
    m_series->replace(newPoints);
    m_chartView->dataUpdate();
}

void OneWaveWidget::setRangeFrequency(int min, int max, float divider) {
    if (!m_sliderFrequency) return;
    m_dividerFrequency = divider;
    m_sliderFrequency->setRange(min, max);
    m_sliderFrequency->setSpan(m_sliderFrequency->minimum(), m_sliderFrequency->maximum());
    xAxis->setRange(m_sliderFrequency->minimum() / m_dividerFrequency, m_sliderFrequency->maximum() / m_dividerFrequency);
}

void OneWaveWidget::setRangeAmplitude(int min, int max, float divider) {
    if (!m_sliderAmplitude || !yAxis) return;
    m_dividerAmplitude = divider; 
    m_sliderAmplitude->setRange(min, max);
    m_sliderAmplitude->setSpan(m_sliderAmplitude->minimum(), m_sliderAmplitude->maximum());
    yAxis->setRange(m_sliderAmplitude->minimum() / m_dividerAmplitude, m_sliderAmplitude->maximum() / m_dividerAmplitude);
}

void OneWaveWidget::setXAxeTitle(QString xAxeTitle)
{
    xAxis->setTitleText(xAxeTitle);

}

void OneWaveWidget::setYAxeTitle(QString yAxeTitle)
{
    yAxis->setTitleText(yAxeTitle);
}

void OneWaveWidget::changeVerticalCoord(int downValue ,int upValue){
    yAxis->setRange(downValue / m_dividerAmplitude, upValue / m_dividerAmplitude);
}

void OneWaveWidget::changeHorizontalCoord(int downValue ,int upValue){
    xAxis->setRange(downValue / m_dividerFrequency, upValue / m_dividerFrequency);
}

/*void OneWaveWidget::redraw() {
    QList<QPointF> points;
    auto sampl = m_scanArm->originalScan.sound.samples;
    //QVector<QPoint> points{ 5000 < sampl.size() ? 5000 : sampl.size() };
    //int samplPerSecond = m_scanArm->originalScan.sound.sampleRate;
    int x = 0;
    int indBegin = 50000 < sampl.size() ? 50000 : sampl.size();
    for (int ind = indBegin, x = 0; ind > 0; --ind ) {
        points.push_back({ QPointF(float(x++) * 0.01, sampl[sampl.size() - ind]) });
    }
    update(std::move(points));
}*/
