#include "chartvievforonewavewidget.h"
#include <QDebug>
#include <QString>

#include <QLayout>
#include <iostream>

ChartViewForOneWaveWidget::ChartViewForOneWaveWidget()
{
    m_flagMousePress = false;
    m_textItem = new QGraphicsTextItem("");
    m_verticalLine = new QGraphicsLineItem(0,0,0,height());
    m_horizontalLine = new QGraphicsLineItem(0,0,width(),0);

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

void ChartViewForOneWaveWidget::
setPeakMagnitude(double magnitude) {
    if (!m_upSeries) {
        m_upSeries = new QLineSeries();
        m_upSeries->setPen(QPen(Qt::green, 2));
        chart()->addSeries(m_upSeries);
        chart()->setAxisX(chart()->axisX(), m_upSeries);   // Назначить ось xAxis, осью X для diagramA
        chart()->setAxisY(chart()->axisY(), m_upSeries);
    }
    
    if (!m_downSeries) {
        m_downSeries = new QLineSeries();
        m_downSeries->setPen(QPen(Qt::green, 2));
        chart()->addSeries(m_downSeries);
        chart()->setAxisX(chart()->axisX(), m_downSeries);   // Назначить ось xAxis, осью X для diagramA
        chart()->setAxisY(chart()->axisY(), m_downSeries);
    }
    m_downSeries->replace(QVector<QPointF>{ { 0, -magnitude }, { 5000,-magnitude }});
    m_upSeries->replace(QVector<QPointF>{ { 0, magnitude }, { 5000, magnitude }});
}
void ChartViewForOneWaveWidget::
updateLineInfo(QPointF point){
    //m_modelOneWave->changeCurentPosition(chart()->mapToValue(point));
    m_verticalLine->setLine(point.x(),0,point.x(),scene()->height());
    m_horizontalLine->setLine(0,point.y(),scene()->width(),point.y());
    auto const scenePos = mapToScene(QPoint(point.x(),point.y()));
    auto const chartItemPos = chart()->mapFromScene(scenePos);
    auto const valueGivenSeries = chart()->mapToValue(chartItemPos);
    /*std::cout << point.x() << " " << point.y() << " "
           << scenePos.x() << " " << scenePos.y() << " "
        << chartItemPos.x() << " " << chartItemPos.y() << " "
        << valueGivenSeries.x() << " " << valueGivenSeries.y() << " "
        << std::endl;*/
    /*m_textItem->setHtml("<div style='background-color:transparent;'> " +
tr("Amplitude (DB): -") + QString::number((int)valueGivenSeries.y()) + "<br>" +
tr("Frequency (Hz): ") + QString::number((int)valueGivenSeries.x()) + "<br>" +
tr("Difference: ") + QString::number(m_modelOneWave->delta()) + "<br>" +
tr("Band: ") + QString::number(m_modelOneWave->band()) + "<br>" +
tr("Amp max: ") + QString::number(m_modelOneWave->maximum()) + "<br>" +
tr("Amp min: ") + QString::number(m_modelOneWave->minimum()) + " </div>"
);*/
    /*if(point.x() + m_textItem->boundingRect().width() <= width()){
        m_textItem->setPos(point);
    }
    else
        m_textItem->setPos(QPointF(width() - m_textItem->boundingRect().width(),point.y() ));*/
}


void ChartViewForOneWaveWidget::
resizeEvent(QResizeEvent* event){
    
    /*if (!m_isCreatedSeries) {
        m_isCreatedSeries = true;
        m_upSeries = new QLineSeries();
        m_downSeries = new QLineSeries();
        //m_upSeries->replace(QVector<QPointF>{ { 0, 0.5 }, { 5000,0.5 }});
        //m_downSeries->replace(QVector<QPointF>{ { 0, -0.5 }, { 5000,-0.5 }});
        m_upSeries->setPen(QPen(Qt::green, 2));
        m_downSeries->setPen(QPen(Qt::green, 2));
        chart()->addSeries(m_upSeries);
        chart()->addSeries(m_downSeries);
        chart()->setAxisX(chart()->axisX(), m_upSeries);   // Назначить ось xAxis, осью X для diagramA
        chart()->setAxisY(chart()->axisY(), m_upSeries);
        chart()->setAxisX(chart()->axisX(), m_downSeries);   // Назначить ось xAxis, осью X для diagramA
        chart()->setAxisY(chart()->axisY(), m_downSeries);
    }*/
    QChartView::resizeEvent(event);
}

void ChartViewForOneWaveWidget::
mousePressEvent(QMouseEvent *event){
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
    //updateLineInfo(m_verticalLine->line().p1());
}
