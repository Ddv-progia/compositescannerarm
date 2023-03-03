#include "chartvievforonewavewidget.h"
#include <QDebug>
#include <QString>
#include <QtCharts/QLineSeries>
#include <QLayout>

ChartViewForOneWaveWidget::ChartViewForOneWaveWidget()
{
    m_flagMousePress = false;
    m_textItem = new QGraphicsTextItem("");
    m_verticalLine = new QGraphicsLineItem(0,0,0,scene()->height());
    m_horizontalLine = new QGraphicsLineItem(0,0,scene()->width(),0);

    m_textItem->setDefaultTextColor(QColor(Qt::white));
    m_textItem->setFont(QFont("",12));
    m_verticalLine->setPen(QPen(Qt::white));
    m_verticalLine->setZValue(100);
    m_horizontalLine->setPen(QPen(Qt::white));
    m_horizontalLine->setZValue(100);
    if(scene()){
        scene()->addItem(m_textItem);
        scene()->addItem(m_verticalLine);
        scene()->addItem(m_horizontalLine);
    }

}

void ChartViewForOneWaveWidget::updateLineInfo(QPointF point){
    //m_modelOneWave->changeCurentPosition(chart()->mapToValue(point));
    m_verticalLine->setLine(point.x(),0,point.x(),scene()->height());
    m_horizontalLine->setLine(0,point.y(),scene()->width(),point.y());
    auto const scenePos = mapToScene(QPoint(point.x(),point.y()));
    auto const chartItemPos = chart()->mapFromScene(scenePos);
    auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
    /*m_textItem->setHtml("<div style='background-color:transparent;'> " +
tr("Amplitude (DB): -") + QString::number((int)valueGivenSeries.y()) + "<br>" +
tr("Frequency (Hz): ") + QString::number((int)valueGivenSeries.x()) + "<br>" +
tr("Difference: ") + QString::number(m_modelOneWave->delta()) + "<br>" +
tr("Band: ") + QString::number(m_modelOneWave->band()) + "<br>" +
tr("Amp max: ") + QString::number(m_modelOneWave->maximum()) + "<br>" +
tr("Amp min: ") + QString::number(m_modelOneWave->minimum()) + " </div>"
);*/
    if(point.x() + m_textItem->boundingRect().width() <= width()){
        m_textItem->setPos(point);
    }
    else
        m_textItem->setPos(QPointF(width() - m_textItem->boundingRect().width(),point.y() ));
}

void ChartViewForOneWaveWidget::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        updateLineInfo(event->pos());
        m_flagMousePress = true;
    }

    QGraphicsView::mousePressEvent(event);
}

void ChartViewForOneWaveWidget::mouseMoveEvent(QMouseEvent *event){
    if(m_flagMousePress){
        updateLineInfo(event->pos());
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ChartViewForOneWaveWidget::mouseReleaseEvent(QMouseEvent *event){
    m_flagMousePress = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void ChartViewForOneWaveWidget::dataUpdate(){
    updateLineInfo(m_verticalLine->line().p1());
}
