#ifndef CHARTVIEVFORONEWAVEWIDGET_H
#define CHARTVIEVFORONEWAVEWIDGET_H

#include <QtCharts/QChartView>
#include <QGraphicsTextItem>

using namespace QtCharts;

class ChartViewForOneWaveWidget : public QChartView
{
    Q_OBJECT
    QGraphicsTextItem *m_textItem;
    QGraphicsLineItem *m_verticalLine,*m_horizontalLine;

    bool m_flagMousePress;
public:
    ChartViewForOneWaveWidget();
    ~ChartViewForOneWaveWidget()override{}

    void updateLineInfo(QPointF point);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dataUpdate();
};

#endif // CHARTVIEVFORONEWAVEWIDGET_H
