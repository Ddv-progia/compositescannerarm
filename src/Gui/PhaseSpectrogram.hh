
#pragma once

#include <QDialog>

#include <UCL/PlotView/PlotItemAction.hh>
#include <qwt_plot_zoomer.h>
#include "Core/ScanData.hh"
#include <vector>

#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>

#define USE_TREND_VERSION //отладочная директива


class PhaseSpectorgram: public QDialog
{
  Q_OBJECT
  QwtPlot* plot;
  QwtPlotCurve* curve;
  QwtPlotCurve* originalCurve;
  QwtPlotCurve* deviationsCurve;
  QwtPlotCurve* koeffsCurve;
  QwtPlotZoomer* zoom;
  boost::multi_array<std::complex<double>, 2> data;
  std::vector<qreal> frequencyData;
  boost::multi_array<std::complex<double>, 2>::iterator current;
  boost::multi_array<std::complex<double>, 2>::index currentIndex;

public: 
  PhaseSpectorgram(const boost::multi_array<std::complex<double>,2>&  data,const std::vector<qreal>&  frequencyData,QWidget* parent = 0);

  Q_SLOT void recalculatePlot();
  Q_SLOT void stepForward();
  Q_SLOT void stepBackward();

  Q_SIGNAL void positionChanged(int shift);
};

QVector<double> findCurveParams(const std::vector<double>& yVals,const std::vector<double>& xVals, int border);

QVector<double> findCurveAngles(std::vector<double>& yVals, std::vector<double>& xVals, int border);