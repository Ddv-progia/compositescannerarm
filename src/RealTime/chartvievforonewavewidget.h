#ifndef CHARTVIEVFORONEWAVEWIDGET_H
#define CHARTVIEVFORONEWAVEWIDGET_H

#include <QtCharts/QChartView>
#include <QGraphicsTextItem>
#include <QtCharts/QLineSeries>

using namespace QtCharts;

class ChartViewForOneWaveWidget : public QChartView
{
    Q_OBJECT
    QGraphicsTextItem *m_textItem;
    QGraphicsLineItem *m_verticalLine,*m_horizontalLine;

    QLineSeries* m_upSeries = nullptr;
    QLineSeries* m_downSeries = nullptr;

    bool m_flagMousePress;
public:
    ChartViewForOneWaveWidget();
    ~ChartViewForOneWaveWidget()override{}
    void setPeakMagnitude(double magnitude);
    void updateLineInfo(QPointF point);

    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dataUpdate();
};

#endif // CHARTVIEVFORONEWAVEWIDGET_H
