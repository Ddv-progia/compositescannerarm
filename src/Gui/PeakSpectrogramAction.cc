/*
 * Gui/PeakSpectrogramAction.cc
 */

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QMessageBox>
#include <QDataStream>
#include <QFile>
#include <qwt_plot.h>
#include <qwt_raster_data.h>
#include <qwt_series_data.h>
#include <qwt_scale_widget.h>
#include <qwt_picker_machine.h>

#include "Core/ScanAlgorithms.hh"
#include "Gui/PeakSpectrogramAction.hh"
#include "Gui/StandardColorMap.hh"

#include <QPushButton>
#include <QSplitter>
#include <qwt_scale_engine.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_zoomer.h>
#include <vtkVariantArray.h>
#include <QVector>
#include <UCL/RegressionAnalysis/LeastSquares.hh>
#include <vtkTable.h>
#include <ppl.h>
#include <time.h>


//#include <opencv\cv.h>
//#include <opencv\highgui.h>

void FftToMagnitude::operator()(boost::multi_array<float, 2>::iterator& iter, fftwf_complex* outputFrame, unsigned long long outputLength,
                                double frequencyStep, double peakBackstep)
{
  for (std::size_t i = 0; i < outputLength; i++)
    (*iter)[i] = std::sqrt(outputFrame[i][0] * outputFrame[i][0] + outputFrame[i][1] * outputFrame[i][1]); //спектр по амплитуде
}

FftAnalysingAlgorithm* FftToMagnitude::clone() const
{
  return new FftToMagnitude(*this);
}


void FftToPhaseAngle::operator()(boost::multi_array<float, 2>::iterator& iter, fftwf_complex* outputFrame, unsigned long long outputLength,
                                 double frequencyStep, double peakBackstep)
{
  std::vector<qreal> frequency;
  std::vector<qreal> values;
  auto k = 0;

  frequency.push_back(0);
  values.push_back(std::atan2(outputFrame[outputLength - 1][1], outputFrame[outputLength - 1][0]));

  for(int i = 1; i < outputLength; i++) {
    frequency.push_back(i * frequencyStep);
    auto value = std::atan2(outputFrame[outputLength - 1 - i][1], outputFrame[outputLength - 1 - i][0]);
    auto delta = value + 2 * k * M_PI - values.back();
    if(delta < -M_PI) {
      k++;
      value = value + 2 * k * M_PI;
    } else if(delta >= M_PI) {
      value += 2 * (k - 1) * M_PI;
    } else
      value += 2 * k * M_PI;
    values.push_back(value);
  }

  auto phaseParams = findCurveParams(values, frequency, 6);
  for(auto i = 0; i < phaseParams.size(); i++)
    (*iter)[i] = phaseParams[i];
}

FftAnalysingAlgorithm* FftToPhaseAngle::clone() const
{
  return new FftToPhaseAngle(*this);
}



void FftToPhaseAngleWithTrend::operator()(boost::multi_array<float, 2>::iterator& iter, fftwf_complex* outputFrame, unsigned long long outputLength,
    double frequencyStep, double peakBackstep)
{
  std::vector<qreal> frequency;
  std::vector<qreal> values;

  std::vector<double> originalValues;
  for(auto i = 0; i < outputLength; i++) {
    frequency.push_back(i * frequencyStep);
    //auto value = i*0.2+std::floor((i*0.2/2)/M_PI)*2*M_PI-M_PI;
    auto value = std::atan2(outputFrame[outputLength - 1 - i][1], outputFrame[outputLength - 1 - i][0]) + (frequency.back() * peakBackstep) * (2 * M_PI);
    originalValues.push_back(value - std::floor(std::abs(value) / (2 * M_PI)) * 2 * M_PI);

    //originalValues.push_back(std::atan2(outputFrame[outputLength-1-i][1],outputFrame[outputLength-1-i][0])+(frequency.back()*peakBackstep)*(2*M_PI));
  }
  values.push_back(originalValues[0]);
  auto trend = 0;
  for(int i = 1; i < outputLength; i++) {
    auto value = originalValues[i];
    auto dx = value - originalValues[i - 1];
    auto delta = dx + 2 * M_PI;

    if(std::abs(dx) + (trend) < std::abs(dx + 2 * M_PI)) {
      delta = dx;
    }
    if(std::abs(delta) >= std::abs(dx - 2 * M_PI) + (trend))
      delta = dx - 2 * M_PI;
    values.push_back(values.back() + delta);
  }
  auto phaseParams = findCurveParams(values, frequency, 3);
  for(auto i = 0; i < phaseParams.size(); i++)
    (*iter)[i] = phaseParams[i];
}

FftAnalysingAlgorithm* FftToPhaseAngleWithTrend:: clone() const
{
  return new FftToPhaseAngleWithTrend(*this);
}




PeakSpectrogramDialog::PeakSpectrogramDialog(const std::shared_ptr<uts::plotting::AbstractItemGroup>& group,
    const uts::plotting::PlotItemActionContext& context, QWidget* parent)
  : QDialog(context.parent), group(group)
{
  setWindowTitle("Спектрограмма по ударам");

  curveBox = new QComboBox;
  for (auto const & curve : group->getCurves()) {
    auto properties = uts::stylesheets::ObjectPropertySet(*context.stylesheet, "curve", curve->getClassName(), curve->getObjectName());
    curveBox->addItem(uts::stylesheets::getObjectTitle(properties, curve->getObjectName()));
  }

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  auto dialogLayout = new QVBoxLayout;
  dialogLayout->addWidget(curveBox);
  dialogLayout->addWidget(buttons);
  setLayout(dialogLayout);

  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

std::shared_ptr<uts::plotting::AbstractCurve> PeakSpectrogramDialog::getCurve() const
{
  return group->getCurves()[curveBox->currentIndex()];
}

//std::unique_ptr<QwtSeriesData<QPointF>> PeakSpectrogramDialog::getPoints() const
//{
//  auto points = group->getCurves()[curveBox->currentIndex()]->getPoints();
//  
//}

QString PeakSpectrogramDialog::getCurveName() const
{
  return curveBox->currentText();
}

PeakSpectrogramPlot::PeakSpectrogramPlot(SpectorogramData* data, const QString& title, int length, std::vector<ColorStop> colorsList): length(length),
  data(data)
{
  plot = new QwtPlot;
  setWindowTitle(title);
  plotSpectrogram = new QwtPlotSpectrogram;
  plotSpectrogram->setData(data);

  auto rightAxis = plot->axisWidget(QwtPlot::yRight);
  rightAxis->setColorBarEnabled(true);
  auto zInterval = plotSpectrogram->data()->interval(Qt::ZAxis);
  plotSpectrogram->setColorMap(new FixedColorMap(colorsList));
  plotSpectrogram->attach(plot);

  rightAxis->setColorMap(zInterval, new FixedColorMap(colorsList));
  plot->setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
  plot->enableAxis(QwtPlot::yRight);

  plot->replot();

  marker = new QwtPlotMarker;
  marker->setLineStyle(QwtPlotMarker::LineStyle::VLine);
  marker->setLinePen(QPen(Qt::black, 1, Qt::PenStyle::DashDotDotLine));
  marker->attach(plot);

  auto widgetLayout = new QVBoxLayout;
  widgetLayout->addWidget(plot);

  peakPicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, plot->canvas());

  peakPicker->setRubberBandPen(QColor(Qt::white));
  peakPicker->setStateMachine(new QwtPickerDragPointMachine);
  peakPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::MouseButton::LeftButton);
  peakPicker->setTrackerPen(QColor(Qt::white));

  // Создание графика зависимости величины от частоты
  auto columnPicker = new QwtPlotPicker(plot->canvas());
  columnPicker->setStateMachine(new QwtPickerClickPointMachine);
  columnPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::MouseButton::RightButton);

  yPlot = new QwtPlot;
  yPlot->setCanvasBackground(Qt::white);
  yPlot->enableAxis(QwtPlot::yLeft);
  yPlot->enableAxis(QwtPlot::xBottom);
  yPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
  yPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);

  curve = new QwtPlotCurve;
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  curve->setPen(QPen(Qt::black));
  curve->attach(yPlot);

  auto grid = new QwtPlotGrid;
  grid->setPen(QPen(Qt::DotLine));
  grid->attach(yPlot);

  zoomerSpectrogramma = new QwtPlotZoomer(plot->canvas(),true);
  zoomerSpectrogramma->setRubberBandPen(QPen(Qt::red));

  zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, yPlot->canvas());
  zoomer->setRubberBandPen(QPen(Qt::black));
  connect(columnPicker, SIGNAL(selected(const QPointF&)), SLOT(selectPoint(const QPointF&)));
  widgetLayout->addWidget(new QSplitter(Qt::Vertical));
  widgetLayout->addWidget(yPlot);
  widgetLayout->setStretch(0, 10);
  showColumnOnYPlot(0);

  setLayout(widgetLayout);
}



void PeakSpectrogramPlot::selectPoint(const QPointF& point)
{
  if(data->interval(Qt::XAxis).contains(point.x()))
    showColumnOnYPlot(point.x());
}

void PeakSpectrogramPlot::showColumnOnYPlot(float time)
{
  marker->setXValue(time);
  plot->replot();
  curve->detach();

  int nColumn = time / (data->interval(Qt::XAxis).width() / data->getData()->size());
  auto originalDataColumn = data->getData()->begin() + nColumn;
  auto size = originalDataColumn->size();
  auto frequencyStep = plotSpectrogram->data()->interval(Qt::YAxis).width() / size;
  QVector<QPointF> curveData;

  for(int i = 0; i < size; i++) {
    curveData.push_back(QPointF(frequencyStep * i, ((*originalDataColumn)[i])));
  }
  curve->setSamples(curveData);
  curve->attach(yPlot);

  yPlot->setAxisScale(QwtPlot::yLeft, curve->minYValue(), curve->maxYValue());
  yPlot->setAxisScale(QwtPlot::xBottom, curve->minXValue(), curve->maxXValue());

  yPlot->updateAxes();

  yPlot->replot();

  zoomer->setZoomBase(QRectF(QPointF(curve->minXValue(), curve->maxYValue()), QPointF(curve->maxXValue(), curve->minYValue())));
}



  PeakSpectrogramAction::PeakSpectrogramAction(FftAnalysingAlgorithm* algorithm, const ProcessingParameters& processingParameters,QString title)
	  : processingParameters(processingParameters),analyser(algorithm),title(title),isStoped(false)
  {
    connect(this, &PeakSpectrogramAction::analysisStart, this, &PeakSpectrogramAction::onStart);
    connect(this, &PeakSpectrogramAction::analysisProgressed, this, &PeakSpectrogramAction::onProgressed);
    connect(this, &PeakSpectrogramAction::analysisFinish, this, &PeakSpectrogramAction::onFinished);
  }

  QString PeakSpectrogramAction::getTitle() const
  {
     return title;
  }

  void PeakSpectrogramAction::apply(const std::shared_ptr<uts::plotting::AbstractItemGroup>& group, 
                      const uts::plotting::PlotItemActionContext& context)
  {

	  PeakSpectrogramDialog dlg(group, context);
	  if (dlg.exec() == QDialog::Accepted) {
		  auto curve = dlg.getCurve();
		  auto curvePoints = curve->getPoints();
    
      auto size = curvePoints->GetNumberOfRows();
      auto xs = curvePoints->GetColumn(0);
      auto ys = curvePoints->GetColumn(1);
      auto estimatedSampleRate = std::abs(double(size - 1) / (xs->GetVariantValue(size - 1).ToFloat() - xs->GetVariantValue(0).ToFloat()));
		  auto sampleRate = curve->getAttribute(uts::plotting::attribute::SAMPLE_RATE, estimatedSampleRate).toDouble();

      std::vector<float> curveData;
      for (std::size_t i = 0; i < size; i++) curveData.push_back(ys->GetVariantValue(i).ToFloat());

		  auto peaks = findPeaks(curveData.begin(), curveData.end(), sampleRate, processingParameters.peakMagnitudeLimit, processingParameters.peakBackstep, 
			processingParameters.peakForestep);
		  if (peaks.size() > 2) {
		    auto fftInputLength  = peaks[1].endIndex - peaks[1].beginIndex;
		    auto fftOutputLength = (fftInputLength - (fftInputLength & 1)) / 2 + 1;
		    boost::multi_array<float, 2> ffts(boost::extents[peaks.size() - 2][fftOutputLength],boost::fortran_storage_order());

		    boost::multi_array<std::complex<double>, 2> fftsc(boost::extents[peaks.size() - 2][fftOutputLength]);

		    auto inputFrame = static_cast<float*>(fftwf_malloc(fftInputLength * sizeof(float)));
		    auto outputFrame = static_cast<fftwf_complex*>(fftwf_malloc(fftOutputLength * sizeof(fftwf_complex)));
		    auto plan = fftwf_plan_dft_r2c_1d(fftInputLength, inputFrame, outputFrame, FFTW_ESTIMATE);

		    isStoped = false;
		    emit analysisStart(peaks.size()-2);
		    for(int peakIndex = 1; peakIndex<peaks.size()-1; peakIndex++){
			  for (std::size_t i = peaks[peakIndex].beginIndex; i < peaks[peakIndex].endIndex; i++) {
			    inputFrame[i - peaks[peakIndex].beginIndex] = curveData[i];
			  }

			  fftwf_execute(plan);
			   
			  (*analyser)(ffts.begin()+peakIndex - 1,outputFrame,fftOutputLength,sampleRate/fftInputLength,processingParameters.peakBackstep);

			  /*for (std::size_t i = 0; i < fftOutputLength; i++) 
			  //  fftsc[peakIndex - 1][i] = std::complex<double>(outputFrame[i][0],outputFrame[i][1]);*/
		
			  if(isAnalysisStoped()){
				  fftwf_destroy_plan(plan);
				  fftwf_free(outputFrame);
				  fftwf_free(inputFrame);
				  emit analysisFinish();
				  return;
			  }

			  QCoreApplication::processEvents();
			  emit analysisProgressed(peakIndex-1);
		    }
		    emit analysisFinish();
		    fftwf_destroy_plan(plan);
		    fftwf_free(outputFrame);
		    fftwf_free(inputFrame);

			//*******	
			//Нормализуем B-скан

			float meanData = 0;
			float sigmaData;
			float kvadro;
			for (auto j = 0; j < fftOutputLength; j++) {
				meanData = 0;
				sigmaData = 0;
				for (auto i = 0; i < peaks.size() - 2; i++) {
					
					meanData += (ffts[i][j] / peaks.size());
				}
				for (auto i = 0; i < peaks.size() - 2; i++) {
					kvadro = ffts[i][j] - meanData;
					sigmaData += (kvadro * kvadro) / (peaks.size() - 1);
				}
				sigmaData = std::sqrtf(sigmaData);
				for (auto i = 0; i < peaks.size() - 2; i++) {
					ffts[i][j] = ((ffts[i][j] - meanData) / (3 * sigmaData));
				}
			}
		   

		    auto plot = new PeakSpectrogramPlot(new SpectorogramData(std::move(ffts), sampleRate, fftInputLength, fftInputLength),
											    QString("Спектр - %1 - %2")
											    .arg(uts::stylesheets::getObjectTitle(uts::stylesheets::ObjectPropertySet(*context.stylesheet, "group", "", group->getName()), group->getName()))
											    .arg(dlg.getCurveName()),peaks.size()-2,processingParameters.colorStopsList);
			plot->setWindowFlags(Qt::Window);
		    plot->show();
		  }
	  }
	}