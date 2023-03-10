#include "onewavewidget.h"
#include <QList>
//#include "customdelegates.h"

#include "chartvievforonewavewidget.h"
#include "qxtspanslider.h"


OneWaveWidget::OneWaveWidget()
{
    m_hSplitter = new QSplitter;
    m_sliderAmplitude = new QxtSpanSlider(Qt::Vertical);

    m_sliderAmplitude->setRange(-1000,1000);
    m_sliderAmplitude->setSpan(m_sliderAmplitude->minimum(),m_sliderAmplitude->maximum());

    m_sliderFrequency = new QxtSpanSlider(Qt::Horizontal);
    m_sliderFrequency->setRange(-10,500);
    m_sliderFrequency->setSpan(m_sliderFrequency->minimum(),m_sliderFrequency->maximum());

    m_vLayout = new QVBoxLayout;
    m_vLayout->setMargin(0);
    m_graphicGridLayout = new QGridLayout();
    m_graphicGridLayout->setMargin(0);
    m_graphicsWidget = new QWidget();

    xAxis = new QValueAxis;
    xAxis->setRange(m_sliderFrequency->minimum(), m_sliderFrequency->maximum());
    xAxis->setTitleText(tr("Lines Hz"));
    xAxis->setTitleBrush(Qt::magenta);
    xAxis->setLabelsColor(Qt::magenta);
    //xAxis->setTickCount(10);

    yAxis = new QValueAxis;
    yAxis->setRange(m_sliderAmplitude->minimum() / 1000.0, m_sliderAmplitude->maximum() / 1000.0);
    yAxis->setTitleText(tr("Amplitude"));
    yAxis->setTitleBrush(Qt::yellow);
    yAxis->setLabelsColor(Qt::yellow);
    m_chartView = new ChartViewForOneWaveWidget();

    m_chartView->chart()->setTheme(QChart::ChartThemeDark);
    m_series = new QLineSeries;
    m_series->setPen(QPen(Qt::red, 2));
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

    m_updateTimer = std::make_unique<QTimer>();
    m_updateTimer->start(100);
    connect(m_updateTimer.get(), &QTimer::timeout, this, &OneWaveWidget::redraw);
}

void OneWaveWidget::
setScan(std::shared_ptr<Scan> scan) {
    m_scan = scan;
    m_chartView->setPeakMagnitude(m_scan->parameters.peakMagnitudeLimit);
}

OneWaveWidget::
~OneWaveWidget(){

}

void OneWaveWidget::update(const QList<QPointF> &newPoints){ //Добавляет точки
    //xAxis->setRange(newPoints.first().y(), newPoints.last().y());
    m_series->replace(newPoints);
    m_chartView->dataUpdate();
}


void OneWaveWidget::changeVerticalCoord(int downValue ,int upValue){
    yAxis->setRange(downValue / 1000.0, upValue / 1000.0);
}

void OneWaveWidget::changeHorizontalCoord(int downValue ,int upValue){
    xAxis->setRange(downValue , upValue );
}

void OneWaveWidget::redraw(){
    if (!m_scan)
        return;
    QList<QPointF> points;
    auto sampl = m_scan->originalScan.sound.samples;
    //QVector<QPoint> points{ 5000 < sampl.size() ? 5000 : sampl.size() };
    //int samplPerSecond = m_scan->originalScan.sound.sampleRate;
    int x = 0;
    int indBegin = 50000 < sampl.size() ? 50000 : sampl.size();
    for (int ind = indBegin, x = 0; ind > 0; --ind /*-= 100*/) {
        points.push_back({ QPointF(float(x++) * 0.01, sampl[sampl.size() - ind]) });
    }
    update(std::move(points));
}
