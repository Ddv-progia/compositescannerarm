/*
 * Gui/PeakSpectrogramAction.hh
 */

#pragma once

#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QDialog>
#include <QProgressDialog>
#include <QThread>
#include <QApplication>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <UCL/PlotView/PlotItemAction.hh>
#include <UCL/PlotView/AttributeKeys.hh>
#include <fftw3.h>

#include "Core/ScanData.hh"

#include <qwt_plot_marker.h>

#include "Gui/PhaseSpectrogram.hh"
#include "Gui/CommonActions.hh"


#include <boost/math/special_functions/binomial.hpp>

class FftAnalysingAlgorithm
{
public:
  FftAnalysingAlgorithm(){}
  virtual void operator()(const boost::multi_array<float, 2>::iterator& iter,fftwf_complex* outputFrame,unsigned long long outputLength,double frequencyStep,double peakBackstep = 0.0) = 0;
  virtual FftAnalysingAlgorithm* clone() const = 0;
};

class FftToMagnitude : public FftAnalysingAlgorithm
{
public:
  void operator()(const boost::multi_array<float, 2>::iterator& iter,fftwf_complex* outputFrame,unsigned long long outputLength,double frequencyStep,double peakBackstep = 0.0)override;
  FftAnalysingAlgorithm* clone() const override;
};


class FftToPhaseAngle : public FftAnalysingAlgorithm
{
public:
  void operator()(const boost::multi_array<float, 2>::iterator& iter,fftwf_complex* outputFrame,unsigned long long outputLength,double frequencyStep,double peakBackstep = 0.0)override ;
  FftAnalysingAlgorithm* clone() const override;
};


class FftToPhaseAngleWithTrend : public FftAnalysingAlgorithm
{
public:
  void operator()(const boost::multi_array<float, 2>::iterator& iter,fftwf_complex* outputFrame,unsigned long long outputLength,double frequencyStep,double peakBackstep = 0.0)override;
  FftAnalysingAlgorithm* clone() const override;
};

class SpectorogramData : public QwtRasterData
{
  boost::multi_array<float, 2> data;
  double sampleRate;
  std::size_t step;
  std::size_t nfft;
  std::array<QwtInterval,3> m_intervals;
public:
  explicit SpectorogramData(boost::multi_array<float, 2>&& data, double sampleRate, std::size_t step, std::size_t nfft)
    : data(std::move(data)), sampleRate(sampleRate), step(step), nfft(nfft)
  { 
      //setInterval(Qt::XAxis, QwtInterval(0, (this->data.shape()[0] - 1) * step / sampleRate));
      //setInterval(Qt::YAxis, QwtInterval(0, (this->data.shape()[1] - 1) * sampleRate / double(nfft)));
      m_intervals.at(Qt::XAxis) = QwtInterval(0, (this->data.shape()[0] - 1) * step / sampleRate);
      m_intervals.at(Qt::YAxis) = QwtInterval(0, (this->data.shape()[1] - 1) * sampleRate / double(nfft));
      
    float maxValue = -std::numeric_limits<float>::max();
    float minValue = std::numeric_limits<float>::max();
    for (int i = 0; i < data.shape()[0]; i++) {
      for (int j = 0; j < data.shape()[1]; j++) {
        if ((data[i][j] != -std::numeric_limits<double>::infinity()) && (data[i][j] != std::numeric_limits<double>::infinity())) {
          minValue = std::min(minValue, data[i][j]);
          maxValue = std::max(maxValue, data[i][j]);
        }
      }
    }

    //setInterval(Qt::ZAxis, QwtInterval(minValue, maxValue));
    m_intervals.at(Qt::ZAxis) = QwtInterval(minValue, maxValue);
  }

  virtual double value(double x, double y) const override
  {
    std::size_t ix = x * sampleRate / step;
    std::size_t iy = y * nfft / sampleRate;

    return data[ix][iy];
  }
  virtual QwtInterval interval(Qt::Axis axis) const override {
      try {
          return m_intervals.at(axis);
      }
      catch (...) {
          return QwtInterval();
      }
  }

  const boost::multi_array<float, 2>* getData() {	  return (&data);}
};


class PeakSpectrogramAction : public ProgressedPlotItemAction
{
  Q_OBJECT
  ProcessingParameters processingParameters;
  FftAnalysingAlgorithm* analyser;
  QString title;
  bool isStoped;
public:
  explicit PeakSpectrogramAction(FftAnalysingAlgorithm* algorithm, const ProcessingParameters& processingParameters,QString title);
  virtual QString getTitle() const override;

  virtual void apply(const std::shared_ptr<uts::plotting::AbstractItemGroup>& group, 
                      const uts::plotting::PlotItemActionContext& context) override;

  Q_SLOT void stop() override
  {
	  isStoped = true;
  }
  Q_SLOT bool isAnalysisStoped()
  {
	  return isStoped;
  }

  Q_SIGNAL void analysisStart(int);
  Q_SIGNAL void analysisProgressed(int);
  Q_SIGNAL void analysisFinish();

};

class PeakSpectrogramDialog : public QDialog
{
  Q_OBJECT
public:
  PeakSpectrogramDialog(const std::shared_ptr<uts::plotting::AbstractItemGroup>& group, const uts::plotting::PlotItemActionContext& context,QWidget* parent = 0);

  std::shared_ptr<uts::plotting::AbstractCurve> getCurve() const;
  QString getCurveName() const;
private:
  std::shared_ptr<uts::plotting::AbstractItemGroup> group;
  QComboBox* curveBox;
};

class PeakSpectrogramPlot : public QWidget
{
  Q_OBJECT
  int length;
  SpectorogramData* data;
  QwtPlot* yPlot;
  QwtPlotCurve* curve;

public:
  PeakSpectrogramPlot(SpectorogramData* data, const QString& title,int length,std::vector<ColorStop> colorsList);
private:
  QwtPlotSpectrogram* plotSpectrogram;
  QwtPlotPicker* peakPicker;
  QwtPlotMarker* marker;
  QwtPlot* plot;
  QwtPlotZoomer* zoomer;
  QwtPlotZoomer* zoomerSpectrogramma;
  
  double currentViewYPoint;

  Q_SLOT void selectPoint(const QPointF& point);
  Q_SLOT void showColumnOnYPlot(float time);

  Q_SIGNAL void zoomerBaseChanged(QRectF);

};
