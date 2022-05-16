/*
 * Gui/ScanDisplayWindow.cc
 */

#include <iterator>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <db_cxx.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_div.h>
#include <qwt_scale_engine.h>
#include <qwt_painter.h>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtGui/QKeyEvent>
#include <QBuffer>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QTableWidget>
#include <QPicture>
#include <UCL/PlotView/View.hh>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>

#include "Core/ConfigurationLocator.hh"
#include "Core/SaveScanTask.hh"
#include "Core/ScanAlgorithms.hh"
#include "Core/ScanDataPlots.hh"
#include "Core/ScanIO.hh"
#include "Core/Qwt/DoubleVectorVerticalSeriesData.hh"
#include "Core/Qwt/MultiArrayColorSliceSeriesData.hh"
#include "Core/Qwt/MultiArrayColorSliceVerticalSeriesData.hh"
#include "Core/Qwt/NormalizedRangeRasterData.hh"
#include "Core/Qwt/MultiArraySliceSeriesData.hh"
#include "Core/Qwt/MultiArraySliceVerticalSeriesData.hh"
#include "Core/Qwt/DefectPointsItem.hh"
#include "Core/Qwt/VerticalPolinomialSeriesData.hh"
#include "Core/Qwt/RangeRasterData.hh"
#include "Gui/PeakSpectrogramAction.hh"
#include "Gui/ScanDisplayWindow.hh"
#include "Gui/StandardColorMap.hh"
#include "Gui/ColoredRangeSelector.hh"
#include "Gui/FrequencyRose.hh"
#include "Core/ImageProcessing.hh"


QImage ScanPlotDefectsMarker::buildMaskImage() const
{
  QImage mask(plot()->size(), QImage::Format::Format_ARGB32_Premultiplied);
  if(defects.empty()) return mask;

  auto xInterval = plot()->axisInterval(xAxis());
  auto yInterval = plot()->axisInterval(yAxis());

  auto pxPerX = xInterval.width() / plot()->width();
  auto pxPerY = yInterval.width() / plot()->height();

  //маску рисуем на прозрачном фоне черным контуром

  QPainter* painter = new QPainter;
  painter->begin(&mask);
  mask.fill(Qt::transparent);
  painter->setPen(QPen(Qt::black));
  painter->setBrush(QBrush(Qt::transparent));

  //перевод метрических координат в экранные с последующей отрисовкой контуров
  for(auto & contour : defects) {
    QPolygon polygon;
    for(auto & point : contour.contour) {
      auto scanPoint = QPoint((point.x() - xInterval.minValue()) / pxPerX, plot()->height() - (point.y() - yInterval.minValue()) / pxPerY);
      polygon.push_back(scanPoint);
    }
    painter->drawPolygon(polygon);
  }
  painter->setPen(QPen(Qt::magenta, 3));
  for(auto & contour : selectedDefects) {
    QPolygon polygon;
    for(auto & point : contour->contour) {
      auto scanPoint = QPoint((point.x() - xInterval.minValue()) / pxPerX, plot()->height() - (point.y() - yInterval.minValue()) / pxPerY);
      polygon.push_back(scanPoint);
    }
    painter->drawPolygon(polygon);
  }

  painter->end();

  return mask;
}


void ScanPlotDefectsMarker::selectContour(const Defect& defect)
{
  for(auto & def : defects) {
    if(defect.contour == def.contour) {
      selectedDefects.push_back(&def);
      return;
    }
  }
}


ScanPlotDefectsMarker::ScanPlotDefectsMarker(const QwtText& title): QwtPlotItem(title)
{
  setRenderHint( QwtPlotItem::RenderAntialiased, true );
  setZ(10);
}

void ScanPlotDefectsMarker::draw (QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const
{
  auto image = buildMaskImage();
  //маштабирование маски с отрисовкой. возможно,вместо этого стоит вызывать перестроение внутри функции
  painter->setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
  QwtPainter::drawPixmap(painter, canvasRect, QPixmap::fromImage(image.scaled(canvasRect.size().toSize(),
                         Qt::AspectRatioMode::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
}

int ScanPlotDefectsMarker::rtti() const
{
  return QwtPlotItem::Rtti_PlotUserItem;
}



ScanDisplayWindow::ScanDisplayWindow(const std::shared_ptr<Scan>& scan, QWidget* parent)
  : QWidget(parent), scan(scan), currentViewPoint(QPointF(0, 0))
{
  this->setWindowTitle(QString::fromStdString(scan->scanName));
  showCommonRange = false;
  commonRangeNum = 0;
  
  // Область выбора вида
  kindBox = new QComboBox;
  kindBox->addItem("Диапазоны");
  kindBox->addItem("Вычеты модели по столбцам");
  kindBox->addItem("Дефектные точки (пропорциональный цвет)");
  kindBox->addItem("Дефектные точки (фиксированный цвет)");

  plotBox = new QComboBox;

  auto additionalPlotsButton = new QPushButton("Детальный график");
  additionalPlotsButton->setCheckable(true);
  auto plotsButton = new QPushButton("Графики");
  auto resizeToWindowButton = new QPushButton("Сохранять пропорции");
  resizeToWindowButton->setCheckable(true);
  auto defectsButton = new QPushButton("Отображать дефекты");
  defectsButton->setCheckable(true);

  auto defectClassificationButton = new QPushButton("Классификатор");
  defectClassificationButton->setCheckable(true);

  auto commandLayout = new QHBoxLayout;

  commandLayout->addWidget(kindBox);
  commandLayout->addWidget(plotBox);
  commandLayout->addStretch();

  commandLayout->addStretch();
  commandLayout->addWidget(additionalPlotsButton);
  commandLayout->addWidget(plotsButton);
  commandLayout->addWidget(resizeToWindowButton);
  commandLayout->addWidget(defectsButton);
  commandLayout->addWidget(defectClassificationButton);

  // Графики
  auto scanPlotEventFilter = new ScanPlotEventFilter(this);

  scanPlot = new QwtPlot;
  scanPlot->enableAxis(QwtPlot::yLeft, false);
  scanPlot->enableAxis(QwtPlot::xBottom, false);
  scanPlot->plotLayout()->setCanvasMargin(-1);
  scanPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
  scanPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);
  scanPlot->installEventFilter(scanPlotEventFilter);

  rowPlot = new QwtPlot;
  rowPlot->setCanvasBackground(Qt::white);
  rowPlot->enableAxis(QwtPlot::yLeft, false);
  rowPlot->enableAxis(QwtPlot::xBottom, false);
  rowPlot->plotLayout()->setCanvasMargin(-1);
  rowPlot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);

  columnPlot = new QwtPlot;
  columnPlot->setCanvasBackground(Qt::white);
  columnPlot->enableAxis(QwtPlot::xBottom, false);
  columnPlot->enableAxis(QwtPlot::yLeft, false);
  columnPlot->plotLayout()->setCanvasMargin(-1);
  columnPlot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);

  // Маркеры
  xScanMarker = new QwtPlotMarker;
  xScanMarker->setLineStyle(QwtPlotMarker::VLine);
  xScanMarker->setLinePen(QPen(Qt::DashLine));
  xScanMarker->attach(scanPlot);

  yScanMarker = new QwtPlotMarker;
  yScanMarker->setLineStyle(QwtPlotMarker::HLine);
  yScanMarker->setLinePen(QPen(Qt::DashLine));
  yScanMarker->attach(scanPlot);

  xRowMarker = new QwtPlotMarker;
  xRowMarker->setLineStyle(QwtPlotMarker::VLine);
  xRowMarker->setLinePen(QPen(Qt::DashLine));
  xRowMarker->attach(rowPlot);

  yColumnMarker = new QwtPlotMarker;
  yColumnMarker->setLineStyle(QwtPlotMarker::HLine);
  yColumnMarker->setLinePen(QPen(Qt::DashLine));
  yColumnMarker->attach(columnPlot);

  // Кривые
  rowCurve = new QwtPlotCurve;
  rowCurve->attach(rowPlot);

  columnCurve = new QwtPlotCurve;
  columnCurve->attach(columnPlot);

  averageColumnCurve = new QwtPlotCurve;
  averageColumnCurve->attach(columnPlot);
  averageColumnCurve->setPen(QPen(Qt::red));

  averageColumnPolynomialCurve = new QwtPlotCurve;
  averageColumnPolynomialCurve->attach(columnPlot);
  averageColumnPolynomialCurve->setPen(QPen(Qt::blue));

  redChannelRowCurve = new QwtPlotCurve;
  redChannelRowCurve->attach(rowPlot);
  redChannelRowCurve->setPen(QPen(Qt::red));

  greenChannelRowCurve = new QwtPlotCurve;
  greenChannelRowCurve->attach(rowPlot);
  greenChannelRowCurve->setPen(QPen(Qt::green));

  blueChannelRowCurve = new QwtPlotCurve;
  blueChannelRowCurve->attach(rowPlot);
  blueChannelRowCurve->setPen(QPen(Qt::blue));

  redChannelColumnCurve = new QwtPlotCurve;
  redChannelColumnCurve->attach(columnPlot);
  redChannelColumnCurve->setPen(QPen(Qt::red));

  greenChannelColumnCurve = new QwtPlotCurve;
  greenChannelColumnCurve->attach(columnPlot);
  greenChannelColumnCurve->setPen(QPen(Qt::green));

  blueChannelColumnCurve = new QwtPlotCurve;
  blueChannelColumnCurve->attach(columnPlot);
  blueChannelColumnCurve->setPen(QPen(Qt::blue));

  // Сетки графиков
  auto rowGrid = new QwtPlotGrid;
  rowGrid->setPen(QPen(Qt::DotLine));
  rowGrid->attach(rowPlot);

  auto columnGrid = new QwtPlotGrid;
  columnGrid->setPen(QPen(Qt::DotLine));
  columnGrid->attach(columnPlot);

  // Информационная панель
  columnLabel = new QLabel;
  columnLabel->setFrameShadow(QFrame::Sunken);
  columnLabel->setFrameShape(QFrame::Box);

  rowLabel = new QLabel;
  rowLabel->setFrameShadow(QFrame::Sunken);
  rowLabel->setFrameShape(QFrame::Box);

  xLabel = new QLabel;
  xLabel->setFrameShadow(QFrame::Sunken);
  xLabel->setFrameShape(QFrame::Box);

  yLabel = new QLabel;
  yLabel->setFrameShadow(QFrame::Sunken);
  yLabel->setFrameShape(QFrame::Box);

  valueLabel = new QLabel;
  valueLabel->setFrameShadow(QFrame::Sunken);
  valueLabel->setFrameShape(QFrame::Box);

  auto infoLayout = new QFormLayout;
  infoLayout->addRow("Столбец", columnLabel);
  infoLayout->addRow("Строка", rowLabel);
  infoLayout->addRow("X", xLabel);
  infoLayout->addRow("Y", yLabel);
  infoLayout->addRow("Уровень", valueLabel);
  infoLayout->setContentsMargins(5, 5, 5, 5);
  infoLayout->setSpacing(5);

  auto pointPicker = new QwtPlotPicker(scanPlot->canvas());
  pointPicker->setStateMachine(new QwtPickerClickPointMachine);
  pointPicker->setTrackerMode(QwtPicker::ActiveOnly);
  pointPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::MouseButton::LeftButton);

  auto regionPicker = new QwtPlotPicker(scanPlot->canvas());
  regionPicker->setRubberBand(QwtPicker::RectRubberBand);
  regionPicker->setStateMachine(new  QwtPickerDragRectMachine);
  regionPicker->setTrackerMode(QwtPicker::AlwaysOff);
  regionPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton, Qt::ShiftModifier);

  auto regionRosePicker = new QwtPlotPicker(scanPlot->canvas());
  regionRosePicker->setRubberBand(QwtPicker::RectRubberBand);
  regionRosePicker->setStateMachine(new  QwtPickerDragRectMachine);
  regionRosePicker->setTrackerMode(QwtPicker::AlwaysOff);
  regionRosePicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);


 // Шкалы графиков
  scanLeftScale = new QwtScaleWidget;
  scanTopScale = new QwtScaleWidget(QwtScaleDraw::TopScale);
  scanRightColorScale = new QwtScaleWidget(QwtScaleDraw::RightScale);
  scanRightColorScale->setColorBarEnabled(true);

  rowLeftScale = new QwtScaleWidget;
  rowBottomScale = new QwtScaleWidget(QwtScaleDraw::BottomScale);

  columnTopScale = new QwtScaleWidget(QwtScaleDraw::TopScale);
  columnRightScale = new QwtScaleWidget(QwtScaleDraw::RightScale);

  table = new QTableView;

  // Компоновка графиков
  auto scanPlotLayout = new QGridLayout;
  scanScrollArea = new QScrollArea;
  scanPlotHolder = new QWidget;
  scanPlotLayout->addWidget(scanTopScale, 0, 1);
  scanPlotLayout->addWidget(scanLeftScale, 1, 0, Qt::AlignRight);
  scanPlotLayout->addWidget(scanPlot, 1, 1);
  scanPlotHolder->setLayout(scanPlotLayout);
  scanScrollArea->setWidget(scanPlotHolder);
  scanScrollArea->setWidgetResizable(true);

  rightHolder = new QWidget;
  auto rightLayout = new QGridLayout;
  rightLayout->addWidget(columnTopScale, 0, 0);
  rightLayout->addWidget(columnRightScale, 1, 1);
  rightLayout->addWidget(columnPlot, 1, 0);
  rightHolder->setLayout(rightLayout);

  bottomHolder = new QWidget;
  auto bottomLayout = new QGridLayout;
  bottomLayout->addWidget(rowBottomScale, 1, 1);
  bottomLayout->addWidget(rowLeftScale, 0, 0, Qt::AlignRight);
  bottomLayout->addWidget(rowPlot, 0, 1);
  bottomLayout->addWidget(table, 2, 1);
  bottomHolder->setLayout(bottomLayout);

  widgetLayout = new QGridLayout;
  widgetLayout->setContentsMargins(0, 0, 0, 0);
  widgetLayout->setSpacing(0);

  infoWidget = new QWidget;
  infoWidget->setLayout(infoLayout);
  rangeSelector = new QWidget;

  widgetLayout->addLayout(commandLayout, 0, 0);
  widgetLayout->addWidget(rangeSelector, 0, 2);
  widgetLayout->addWidget(scanScrollArea, 3, 0);
  widgetLayout->addWidget(scanRightColorScale, 3, 1);
  widgetLayout->addWidget(rightHolder, 3, 2);
  widgetLayout->addWidget(bottomHolder, 4, 0);
  widgetLayout->addWidget(infoWidget, 4, 2);

  widgetLayout->setColumnStretch(0, 9);
  widgetLayout->setColumnStretch(2, 1);
  widgetLayout->setRowStretch(3, 1);
  setLayout(widgetLayout);

  createColoredRangeSelector();

  connect(plotsButton, SIGNAL(clicked()), this, SLOT(showPlots()));
  connect(kindBox, SIGNAL(activated(int)), this, SLOT(updatePlotList()));

  connect(pointPicker, SIGNAL(selected(const QPointF&)), this, SLOT(setViewPoint(const QPointF&)));
  connect(regionPicker, SIGNAL(selected(const QRectF&)), this, SLOT(normalizeRegion(const QRectF&)));
  connect(regionRosePicker, SIGNAL(selected(const QRectF&)), this, SLOT(getRegionFrequencyRose(const QRectF&)));
  connect(scanPlotEventFilter, SIGNAL(moveMarkers(int, int)), this, SLOT(moveMarkers(int, int)));
  connect(scanPlotEventFilter, SIGNAL(changeExtremums()), this, SLOT(changeExtremums()));

  connect(additionalPlotsButton, SIGNAL(toggled(bool)), this, SLOT(setAdditionalGraphicsVisibility(bool)));
  connect(defectsButton, SIGNAL(toggled(bool)), this, SLOT(setDefectsVisible(bool)));
  connect(resizeToWindowButton, SIGNAL(toggled(bool)), this, SLOT(togleWindowSize(bool)));
  connect(plotBox, SIGNAL(activated(int)), this, SLOT(showUserRange()));
  setAdditionalGraphicsVisibility(false);
  createDefectsMarker();
  setDefectsVisible(false);
  updatePlotList();

  plotBox->setCurrentIndex(scan->currentRange);

  ///////////
  
  analyseRegion = false;
  connect(defectClassificationButton,SIGNAL(toggled(bool)),SLOT(showClassificationTable(bool)));
}

std::vector<std::vector<float>> ScanDisplayWindow::getPeaksFromRect(const QRectF& rect)
{
  std::vector<std::vector<float>> samples;
  auto beginPoint = pointIndexes(rect.topLeft());
  auto endPoint = pointIndexes(rect.bottomRight());

  std::size_t startIndex = scan->normalizedRanges.begin()->beginIndex;
  std::size_t stopIndex = scan->normalizedRanges.begin()->endIndex;
  std::size_t step = scan->normalizedRanges.begin()->step;

  for(auto row = beginPoint.y(); row <= endPoint.y(); row++) {

    for(auto column = beginPoint.x(); column <= endPoint.x(); column++) {
      std::vector<float> peakSamples;
      auto sampleStart = startIndex + step * column;
      auto sampleStop = startIndex + step * (column + 1);
      auto sample = scan->lines[row - 1].samples.begin() + sampleStart;
      for(; sampleStart < sampleStop; sampleStart++) {
        peakSamples.push_back(*sample);
        if(sample != scan->lines[row - 1].samples.end() - 1)
          sample++;
      }
      samples.push_back(peakSamples);
    }
  }
  return samples;
}


void ScanDisplayWindow::selectContour(size_t n )
{
  auto defect = plotDefectsModel->currentDefects().at(n - 1);
  defectsMarker->selectContour(defect);
  scanPlot->replot();
}
void ScanDisplayWindow::setDefectMask()
{
  defectsMarker->setDefects(plotDefectsModel->currentDefects());
}


void ScanDisplayWindow::changeExtremum(Extremum ex) //*******
{
    switch (ex) {
        default:
        case Extremum::Max: {
            for (auto& range : scan->commonNormalizedRanges) {
                range.view = range.maxView;
                range.extremum = Extremum::Max;
            }
            break;
        }
        case Extremum::Aver: {
            for (auto& range : scan->commonNormalizedRanges) {
                range.view = range.averView;
                range.extremum = Extremum::Aver;
            }
            break;
        }
        case Extremum::Min: {
            for (auto& range : scan->commonNormalizedRanges) {
                range.view = range.minView;
                range.extremum = Extremum::Min;
            }
            break;
        }
    }
}

void ScanDisplayWindow::changeExtremums()
{
  if(scan->commonNormalizedRanges.front().extremum == Extremum::Max) {
    for(auto & range : scan->commonNormalizedRanges) {
      range.view = range.minView;
      range.extremum = Extremum::Min;
    }
  } else {
    for(auto & range : scan->commonNormalizedRanges) {
      range.view = range.maxView;
      range.extremum = Extremum::Max;
    }
  }
}

void ScanDisplayWindow::showUserRange()
{
  showCommonRange = false;
  updatePlot();
}

QPoint ScanDisplayWindow::specIndex(QPoint& viewPoint, std::size_t nRange)
{
  // из индексов в точки спектра
  double sample;
  int line = viewPoint.y();
  double xSize = scan->normalizedRanges.front().view.shape()[0] / 3;
  int size = scan->normalizedSpec[line][nRange].samples.size();

  sample = size / xSize;
  sample *= viewPoint.x() / 3;

  return(QPoint(std::min(size, static_cast<int>(sample)), line));
}

void ScanDisplayWindow::normalizeSpec(QPoint beginPoint, QPoint endPoint, std::vector<std::vector<RangeScanLine>>& spec)
{
  namespace ba = boost::accumulators;
  if(beginPoint == endPoint)
    return;

  scan->parameters.specNormalization.clear();
  scan->parameters.specNormalization.resize(spec.begin()->size());

  //для всех диапазонов
  for(auto nRange = 0; nRange < spec.begin()->size(); nRange++) {
    ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::variance>> acc;

    auto begin = specIndex(beginPoint, nRange);
    auto end = specIndex(endPoint, nRange);
    if (begin.x() > end.x()) {
      auto temp = begin.x();
      begin.setX(end.x());
      end.setX(temp);
    }
    if (begin.y() > end.y()) {
      auto temp = begin.y();
      begin.setY(end.y());
      end.setY(temp);
    }

    for(auto row = begin.y(); row <= end.y(); row++) {
      if (end.x() >= spec[row][nRange].samples.size() || end.y() >= spec.size()) {
        end.setX(spec[row][nRange].samples.size() - 1);
        end.setY(spec.size() - 1);
      }

      for(auto column = begin.x(); column <= end.x(); column++) {
        acc(spec[row][nRange].samples[column]);
      }
    }
    auto sigma = std::sqrt(ba::variance(acc));
    auto average = ba::mean(acc);

	scan->parameters.specNormalization[nRange].sigma = sigma;
	scan->parameters.specNormalization[nRange].average = average;

	//будет выполнено при построении скана
    //for(auto & line : spec) {
    //  for(auto & sample : line[nRange].samples) {
    //    if(sample != 0)
    //      sample = (sample - average) / (3 * sigma);
    //  }
    //}
  }
  scan->processingStage = ScanProcessingStage::DirectionNormalized;
  refreshWindow();
}

void ScanDisplayWindow::refreshWindow()
{
  emit refreshScan(scan);
  QWidget* win = this;
  while (win && ! dynamic_cast<QMdiSubWindow*>(win)) win = win->parentWidget();
  if (win) win->deleteLater();
}


void ScanDisplayWindow::normalizeRegion(const QRectF& rect)
{
  QPoint beginPoint, endPoint;
  beginPoint = pointIndexes(rect.topLeft());
  endPoint = pointIndexes(rect.bottomRight());
  if (beginPoint.x() > endPoint.x()){
    auto temp = beginPoint.x();
    beginPoint.setX(endPoint.x());
    endPoint.setX(temp);
  }
  if (beginPoint.y() > endPoint.y()){
    auto temp = beginPoint.y();
    beginPoint.setY(endPoint.y());
    endPoint.setY(temp);
  }

  if(endPoint.y() >= scan->normalizedRanges.begin()->view.shape()[1])  endPoint.setY(scan->normalizedRanges.begin()->view.shape()[1] - 1);
  if(endPoint.x() >= scan->normalizedRanges.begin()->view.shape()[0])  endPoint.setX(scan->normalizedRanges.begin()->view.shape()[0] - 1);
  if(beginPoint.y() >= scan->normalizedRanges.begin()->view.shape()[1])  beginPoint.setY(scan->normalizedRanges.begin()->view.shape()[1] - 1);
  if(beginPoint.x() >= scan->normalizedRanges.begin()->view.shape()[0])  beginPoint.setX(scan->normalizedRanges.begin()->view.shape()[0] - 1);

  /* for(int i = 1;i<6;i++) 
      normalizeSpec(QPoint(beginPoint.x()/i,beginPoint.y()/i),QPoint(endPoint.x()/i,endPoint.y()/i),scan->normalizedSpec);*/
  normalizeSpec(beginPoint, endPoint, scan->spec);
}

void ScanDisplayWindow::getRegionFrequencyRose(const QRectF& rect)
{
  QPoint beginPoint, endPoint;
  std::vector<std::pair<FrequencyRange, double>> points;
  beginPoint = pointIndexes(rect.topLeft());
  endPoint = pointIndexes(rect.bottomRight());
  if (beginPoint.x() > endPoint.x()){
    auto temp = beginPoint.x();
    beginPoint.setX(endPoint.x());
    endPoint.setX(temp);
  }
  if (beginPoint.y() > endPoint.y()){
    auto temp = beginPoint.y();
    beginPoint.setY(endPoint.y());
    endPoint.setY(temp);
  }

  if(endPoint.y() >= scan->normalizedRanges.begin()->view.shape()[1])  endPoint.setY(scan->normalizedRanges.begin()->view.shape()[1] - 1);
  if(endPoint.x() >= scan->normalizedRanges.begin()->view.shape()[0])  endPoint.setX(scan->normalizedRanges.begin()->view.shape()[0] - 1);
  if(beginPoint.y() >= scan->normalizedRanges.begin()->view.shape()[1])  beginPoint.setY(scan->normalizedRanges.begin()->view.shape()[1] - 1);
  if(beginPoint.x() >= scan->normalizedRanges.begin()->view.shape()[0])  beginPoint.setX(scan->normalizedRanges.begin()->view.shape()[0] - 1);

  beginPoint = specIndex(beginPoint);
  endPoint = specIndex(endPoint);

  if(analyseRegion){
	  auto classificator = new DefectClassificator(scan->parameters.defectClassification,scan->normalizedSpec);
	  try{
		  auto closeness = classificator->findFieldCloseness(beginPoint,endPoint);
		  auto controlParams = classificator->getControlParams(beginPoint,endPoint);
		  auto normControlParams = classificator->normalizeControlParams(controlParams);
		  auto defects = classificator->normalizeParams(scan->parameters.defectClassification);
		  auto residuals = classificator->getResiduals(defects,normControlParams);

		  auto closenessTable = new QTableWidget(scan->parameters.defectClassification.back().params.size()+2,scan->parameters.defectClassification.size()*2+2);
		  QStringList header;
		  header<<"Исследуемая область";
		  header<<"Исследуемая область(норм.)";
          for (auto& defect : scan->parameters.defectClassification) {
              header << QString::fromStdString(defect.name) + ":Коэф.";
              header << QString::fromStdString(defect.name) + ":Знач.";
          }
		  closenessTable->setHorizontalHeaderLabels(header);
		  for(auto nParam = 0; nParam<normControlParams.size(); nParam++){
			  auto param = new QTableWidgetItem(QString::number(controlParams[nParam]));
			  auto normParam = new QTableWidgetItem(QString::number(normControlParams[nParam]));
			  param->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
			  normParam->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
			  closenessTable->setItem(nParam,0,param);
			  closenessTable->setItem(nParam,1,normParam);
			  for(auto nDef = 0; nDef< defects[0].size(); nDef++){
				  auto kItem = new QTableWidgetItem(QString::number(scan->parameters.defectClassification[nDef].params[nParam].koeff));
				  kItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
				  closenessTable->setItem(nParam,2*nDef+2,kItem);
				  auto vItem = new QTableWidgetItem(QString::number(residuals[nParam][nDef]));
				  vItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
				  closenessTable->setItem(nParam,2*nDef+3,vItem);
			  }
		  }
		  auto nMin = 0;
		  for(auto nDef = 0; nDef<scan->parameters.defectClassification.size(); nDef++){
			  auto item = new QTableWidgetItem(QString::number(closeness[nDef]));
			  item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
			  if(closeness[nDef] < closeness[nMin])	nMin = nDef;
			  closenessTable->setItem(closenessTable->rowCount()-1,2*nDef+3,item);
		  }
		  closenessTable->item(closenessTable->rowCount()-1,2*nMin+3)->setBackgroundColor(QColor(Qt::yellow));
		  closenessTable->resizeColumnsToContents();
		  closenessTable->show();
	  }catch(...){}
	  delete classificator;
  }

  RangedMultiArray  rangedPlots;
  for(auto sNum = 0; sNum < scan->normalizedSpec.front().size(); sNum++) {
    DDArray plot(boost::extents[endPoint.y() - beginPoint.y() + 1][endPoint.x() - beginPoint.x() + 1]);
    for(int row = beginPoint.y(); row <= endPoint.y(); row++) {
	  for(int column = beginPoint.x(); column<=endPoint.x() && column< scan->normalizedSpec[row][sNum].samples.size(); column++) {
        plot[row - beginPoint.y()][column - beginPoint.x()] = scan->normalizedSpec[row][sNum].samples[column];
      }
    }
    rangedPlots.push_back(std::make_pair(scan->normalizedSpec.front()[sNum].range, plot));
  }

  FrequencyRoseWidget* rose = new FrequencyRoseWidget(rangedPlots, scan->parameters.colorStopsList, this);
  rose->show();
}

void ScanDisplayWindow::swapItemsByIndexes(int firstItemIndex, int secondItemIndex)
{
  int row1, column1, rowSpan1, columnSpan1, row2, column2, rowSpan2, columnSpan2;

  QLayoutItem* first = layout()->itemAt(firstItemIndex);
  QLayoutItem* second = layout()->itemAt(secondItemIndex);
  widgetLayout->getItemPosition(firstItemIndex, &row1, &column1, &rowSpan1, &columnSpan1);
  widgetLayout->getItemPosition(secondItemIndex, &row2, &column2, &rowSpan2, &columnSpan2);

  widgetLayout->addWidget(first->widget(), row2, column2, rowSpan2, columnSpan2);
  widgetLayout->addWidget(second->widget(), row1, column1, rowSpan1, columnSpan1);
}

void ScanDisplayWindow::setAdditionalGraphicsVisibility(bool isVisible)
{
  rightHolder->setVisible(isVisible);
  bottomHolder->setVisible(table->isVisible() ? true : isVisible);
  rowBottomScale->setVisible(isVisible);
  rowLeftScale->setVisible(isVisible);
  rowPlot->setVisible(isVisible);

  swapItemsByIndexes(layout()->indexOf(infoWidget), layout()->indexOf(rightHolder));
}

LineEncoding ScanDisplayWindow::requestLineEncoding()
{
  auto encodingBox = new QComboBox;
  encodingBox->addItems(QStringList() << "IEEE 754" << "PCM 16 бит" << "Windows Media Audio Lossless");
  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
  auto dialogLayout = new QVBoxLayout;
  dialogLayout->addWidget(encodingBox);
  dialogLayout->addWidget(buttons);
  QDialog dlg;
  dlg.setLayout(dialogLayout);
  connect(buttons, SIGNAL(accepted()), &dlg, SLOT(accept()));

  if (dlg.exec() == QDialog::Accepted)
    return static_cast<LineEncoding>(encodingBox->currentIndex());
  else
    return LineEncoding::Float;
}

void ScanDisplayWindow::save(BackgroundTaskExecutor& taskExecutor)
{
  if (filename.isEmpty()) {
    saveAs(taskExecutor);
  } else {
    try {
      taskExecutor.enqueue(new SaveScanTask(filename, *scan, requestLineEncoding()));
    } catch (DbException& exc) {
      QMessageBox::critical(this, "Ошибка", exc.what());
    }
  }
}

void ScanDisplayWindow::saveAs(BackgroundTaskExecutor& taskExecutor)
{
  auto fn = QFileDialog::getSaveFileName(this, "Сохранение скана", QString(), "Сканы (*.csp)");
  if (! fn.isEmpty()) {
    try {
      filename = fn;
      taskExecutor.enqueue(new SaveScanTask(filename, *scan, requestLineEncoding()));
    } catch (DbException& exc) {
      QMessageBox::critical(this, "Ошибка", exc.what());
    }
  }
}

void ScanDisplayWindow::createColoredRangeSelector()
{
  std::vector<float> mins, maxs, avers;

  double max = 0.0;
  double min = 0.0;

  min = scan->commonNormalizedRanges.begin()->min;
  max = scan->commonNormalizedRanges.begin()->max;

  for(auto & range : scan->commonNormalizedRanges) {
    if(range.min < min) min = range.min;
    if(range.max > max) max = range.max;

    mins.push_back(range.min);
    maxs.push_back(range.max);
    avers.push_back(range.aver);
  }
  QwtColorMap* colorMap = new FixedColorMap(scan->parameters.colorStopsList);//(min,max);
  auto commonRanges = constructCommonRanges();
  if(scan->parameters.shouldNormalize)
    commonRanges.push_back(FrequencyRange{ commonRanges.front().from, commonRanges.back().to });
  auto selector = new ColoredRangeSelector(maxs, mins, avers, commonRanges, colorMap);
  auto layout = new QHBoxLayout;
  layout->setContentsMargins(1, 1, 1, 1);

  layout->addWidget(selector);

  rangeSelector->setLayout(layout);
  connect(selector, SIGNAL(selectRange(int, Extremum)), SLOT(selectRange(int, Extremum)));
  connect(plotBox, SIGNAL(activated(int)), selector, SLOT(leave()));
}

void ScanDisplayWindow::updateRangesPlot()
{
  if (scan->parameters.colorStopsList.empty())
    return;

  auto max = scan->parameters.colorStopsList.begin()->val;
  auto min = scan->parameters.colorStopsList.begin()->val;

  std::for_each(scan->parameters.colorStopsList.begin(), scan->parameters.colorStopsList.end(),
  [&](ColorStop stop) {
    if(stop.val > max) max = stop.val;
    if(stop.val < min) min = stop.val;
  });

  auto idx = plotBox->currentIndex();
  if (idx < 0) return;
  
  scan->currentRange = idx;
  auto* normalizedRanges = &scan->normalizedRanges;
  if(showCommonRange) {
    normalizedRanges = &scan->commonNormalizedRanges;
    idx = commonRangeNum;
  }
  else{
      //switch (scan->normalizedRanges[idx].extremum){
      switch (scan->parameters.extremumOfRanges[idx]) {
      case Extremum::Max:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].maxView;
          break;
      case Extremum::Min:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].minView;
          break;
      case Extremum::Aver:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].averView;
          break;
      }
  }
  auto spec = new QwtPlotSpectrogram;
  spec->setData(new NormalizedRangeRasterData((*normalizedRanges)[idx]));
  //spec->setData(new RangeRasterData(scan->ranges, idx));
  if (scan->parameters.defectRendering.fixedColorScale)
    spec->setColorMap(new FixedColorMap(scan->parameters.colorStopsList));
  else
    spec->setColorMap(new StandardColorMap);

  spec->attach(scanPlot);

  QwtColorMap* colorMap;
  QwtInterval zInterval = spec->data()->interval(Qt::ZAxis);
  if(scan->parameters.defectRendering.fixedColorScale) {
    colorMap = new FixedColorMap(scan->parameters.colorStopsList);//(min,max);
    scanRightColorScale->setColorMap(QwtInterval(min, max), colorMap);
    scanPlot->setAxisScale(QwtPlot::yRight, min, max);
  } else {
    colorMap = new StandardColorMap;
    scanRightColorScale->setColorMap(zInterval, colorMap);
    scanPlot->setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
  }
  scanPlot->updateAxes();
  scanRightColorScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::yRight));
}

void ScanDisplayWindow::selectRange(int idx, Extremum ex)
{
  showCommonRange = true;
//*******
  this->changeExtremum(ex);
  //if(ex != scan->commonNormalizedRanges.front().extremum)
    //this->changeExtremums(); 
//*******

  commonRangeNum = idx;

  scanPlot->replot();
  updatePlot();

  rowPlot->updateAxes();
  rowLeftScale->setScaleDiv(rowPlot->axisScaleDiv(QwtPlot::yLeft));
  rowBottomScale->setScaleDiv(rowPlot->axisScaleDiv(QwtPlot::xBottom));

  columnPlot->updateAxes();
  columnTopScale->setScaleDiv(columnPlot->axisScaleDiv(QwtPlot::xBottom));
  columnRightScale->setScaleDiv(columnPlot->axisScaleDiv(QwtPlot::yLeft));

  rowPlot->replot();
  columnPlot->replot();
}


void  ScanDisplayWindow::getSelectedContour(const QItemSelection& selected, const QItemSelection& deselected)
{
  defectsMarker->removeSelection();
  if(selected.isEmpty()) return;
  for(auto & index : selected.front().indexes()) {
    auto defectNum = table->model()->index(index.row(), 0, index.parent()).data().toInt();
    auto defect = plotDefectsModel->currentDefects().at(defectNum - 1);
    defectsMarker->selectContour(defect);
  }
  scanPlot->replot();
}

QTableView*  ScanDisplayWindow::getDefectTableView()
{
  return table;
}

void ScanDisplayWindow::createDefectsMarker()
{
  defectsMarker = new ScanPlotDefectsMarker;
  defectsMarker->attach(scanPlot);
  defectsMarker->hide();
  plotDefectsModel = new PlotDefectsModel(scan);
  table->setModel(new DefectsViewTableModel(plotDefectsModel->currentDefects()));
  table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  table->setAttribute(Qt::WA_DeleteOnClose, true);
  connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
          SLOT(getSelectedContour(const QItemSelection&, const QItemSelection&)));
  connect(plotDefectsModel, SIGNAL(defectSelected(const Defect&)), SLOT(selectContour(const Defect&)));
  connect(table, SIGNAL(destroyed()), SLOT(deleteDefectsMarker()));
  setDefectMask();
  scanPlot->replot();
}
void ScanDisplayWindow::deleteDefectsMarker()
{
  scanPlot->detachItems(defectsMarker->rtti());
  scanPlot->replot();
}

void ScanDisplayWindow::togleWindowSize(bool isPressed)
{
  if(isPressed) {
    auto width = scanPlot->width();
    auto height = scanPlot->height();
    auto xWidth = scanPlot->axisInterval(Qt::ZAxis).width();
    auto yWidth = scanPlot->axisInterval(Qt::XAxis).width();
    auto pw = width / xWidth;
    auto ph = height / yWidth;
    auto p = std::max(pw, ph);
    scanScrollArea->setWidgetResizable(false);
    scanPlotHolder->resize(xWidth * p + scanLeftScale->width() + scanPlotHolder->layout()->spacing(),
                           yWidth * p + scanTopScale->height() + scanPlotHolder->layout()->spacing());

  } else {
    scanScrollArea->setWidgetResizable(true);
  }

  scanPlot->setAxisScale(Qt::XAxis, scanPlot->axisInterval(Qt::XAxis).minValue(), scanPlot->axisInterval(Qt::XAxis).maxValue());
  scanPlot->setAxisScale(Qt::ZAxis, scanPlot->axisInterval(Qt::ZAxis).minValue(), scanPlot->axisInterval(Qt::ZAxis).maxValue());
  scanPlot->updateAxes();
  scanLeftScale->setScaleDiv(scanPlot->axisScaleDiv(Qt::XAxis));
  scanTopScale->setScaleDiv(scanPlot->axisScaleDiv(Qt::ZAxis));
  scanPlot->replot();
}
void ScanDisplayWindow::setDefectsVisible(bool isVisible)
{
  defectsMarker->setVisible(isVisible);
  bottomHolder->setVisible(rightHolder->isVisible() ? true : isVisible);
  table->setVisible(isVisible);
  scanPlot->replot();
}


void ScanDisplayWindow::updateResidualsPlot()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  auto spec = new QwtPlotSpectrogram;
  spec->setData(new NormalizedRangeRasterData(scan->rangesResiduals[idx]));

  spec->setColorMap(new FixedColorMap(scan->parameters.colorStopsList));
  spec->attach(scanPlot);

  auto xInterval = spec->data()->interval(Qt::XAxis);
  scanPlot->setAxisScale(QwtPlot::xBottom, xInterval.minValue(), xInterval.maxValue());

  auto zInterval = spec->data()->interval(Qt::ZAxis);
  scanRightColorScale->setColorMap(zInterval, new FixedColorMap(scan->parameters.colorStopsList));
  scanPlot->setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue());
  scanPlot->updateAxes();
  scanRightColorScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::yRight));
}

void ScanDisplayWindow::updateDefectPointsPlot()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  auto item = new DefectPointsItem(scan->renderedDefectPoints[idx]);
  item->attach(scanPlot);
}

void ScanDisplayWindow::updateFixedColorDefectPointsPlot()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  auto item = new DefectPointsItem(scan->renderedDefectPointsFixedColor[idx]);
  item->attach(scanPlot);
}


void ScanDisplayWindow::updatePlot()
{
  scanPlot->detachItems(QwtPlotItem::Rtti_PlotSpectrogram);
  scanPlot->detachItems(DefectPointsItem::Rtti_DefectPointsItem);

  switch (kindBox->currentIndex()) {
  case 0:
    updateRangesPlot();
    scanRightColorScale->show();
    xScanMarker->setLinePen(Qt::black, 0, Qt::DashLine);
    yScanMarker->setLinePen(Qt::black, 0, Qt::DashLine);
    break;
  case 1:
    updateResidualsPlot();
    scanRightColorScale->show();
    xScanMarker->setLinePen(Qt::black, 0, Qt::DashLine);
    yScanMarker->setLinePen(Qt::black, 0, Qt::DashLine);
    break;
  case 2:
    updateDefectPointsPlot();
    scanRightColorScale->hide();
    xScanMarker->setLinePen(Qt::white, 0, Qt::DashLine);
    yScanMarker->setLinePen(Qt::white, 0, Qt::DashLine);
    break;
  case 3:
    updateFixedColorDefectPointsPlot();
    scanRightColorScale->hide();
    xScanMarker->setLinePen(Qt::white, 0, Qt::DashLine);
    yScanMarker->setLinePen(Qt::white, 0, Qt::DashLine);
    break;
  }

  scanLeftScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::yLeft));
  scanTopScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::xBottom));
  updateViewPoint();

  scanPlot->replot();
  rowPlot->replot();
  columnPlot->replot();

}

void ScanDisplayWindow::updatePlotList()
{
  plotBox->clear();
  int i = 0;
  switch (kindBox->currentIndex()) {
  default:
      for (auto& r : scan->parameters.ranges) {
          auto index = plotBox->count();
          QString strExtremum = " Aver";
          QString path = "icons/button_average.ico";
          if (scan->parameters.extremumOfRanges.size()>i) {
              if (scan->parameters.extremumOfRanges[i] == ::Extremum::Min) {
                  path = "icons/button_min.ico";
                  strExtremum = " Min";
              }
              else if (scan->parameters.extremumOfRanges[i] == ::Extremum::Max) {
                  path = "icons/button_max.ico";
                  strExtremum = " Max";
              }
          }
          plotBox->addItem(QString("%1 -- %2").arg(r.from).arg(r.to) + strExtremum);
          QIcon icon(path);
          plotBox->setItemIcon(index, icon);
          i++;
      }
    break;
  case 2:
  case 3:
    for (auto & d : scan->parameters.defectPoints) plotBox->addItem(QString::fromStdString(d.title));
    break;
  }
  updatePlot();
}

void ScanDisplayWindow::updateRangeViewPoint()
{
  int idx = plotBox->currentIndex();
  auto* range = &scan->normalizedRanges[idx];
  if(showCommonRange)
    range = &scan->commonNormalizedRanges[commonRangeNum];

  auto all = boost::multi_array<float, 2>::index_range();


  valueLabel->setText(QString::number(range->view[pointIndexes(currentViewPoint).x()][pointIndexes(currentViewPoint).y()], 'f', 3));

  rowCurve->setData(new MultiArraySliceSeriesData(range->view[boost::indices[all][pointIndexes(currentViewPoint).y()]], range->startCoordinate,
                    range->finalCoordinate));
  columnCurve->setData(new MultiArraySliceVerticalSeriesData(range->view[boost::indices[pointIndexes(currentViewPoint).x()][all]],
                       range->lineCoordinates));

  averageColumnCurve->setData(new DoubleVectorVerticalSeriesData(scan->averageColumns[idx], range->lineCoordinates));
  if (scan->parameters.columnModelOrder > 0) {
    averageColumnPolynomialCurve->setData(new VerticalPolinomialSeriesData(scan->averageColumnPolyniomials[idx].coefficients,
                                          range->lineCoordinates.front(), range->lineCoordinates.back()));
  } else {
    averageColumnPolynomialCurve->setData(0);
  }

  redChannelRowCurve->setData(0);
  greenChannelRowCurve->setData(0);
  blueChannelRowCurve->setData(0);
  redChannelColumnCurve->setData(0);
  greenChannelColumnCurve->setData(0);
  blueChannelColumnCurve->setData(0);
}

void ScanDisplayWindow::updateResidualsViewPoint()
{
  auto idx = plotBox->currentIndex();

  auto all = boost::multi_array<float, 2>::index_range();
  auto& range = scan->rangesResiduals[idx];

  valueLabel->setText(QString::number(range.view[pointIndexes(currentViewPoint).x()][pointIndexes(currentViewPoint).y()], 'f', 3));

  rowCurve->setData(new MultiArraySliceSeriesData(range.view[boost::indices[all][pointIndexes(currentViewPoint).y()]], range.startCoordinate,
                    range.finalCoordinate));
  columnCurve->setData(new MultiArraySliceVerticalSeriesData(range.view[boost::indices[pointIndexes(currentViewPoint).x()][all]],
                       range.lineCoordinates));
  averageColumnCurve->setData(new DoubleVectorVerticalSeriesData(scan->averageColumns[idx], range.lineCoordinates));
  if (scan->parameters.columnModelOrder > 0) {
    averageColumnPolynomialCurve->setData(new VerticalPolinomialSeriesData(scan->averageColumnPolyniomials[idx].coefficients,
                                          range.lineCoordinates.front(), range.lineCoordinates.back()));
  } else {
    averageColumnPolynomialCurve->setData(0);
  }

  averageColumnCurve->setData(0);
  averageColumnPolynomialCurve->setData(0);
  redChannelRowCurve->setData(0);
  greenChannelRowCurve->setData(0);
  blueChannelRowCurve->setData(0);
  redChannelColumnCurve->setData(0);
  greenChannelColumnCurve->setData(0);
  blueChannelColumnCurve->setData(0);
}

void ScanDisplayWindow::updateDefectsViewPoint()
{
  auto idx = plotBox->currentIndex();

  auto all = boost::multi_array<float, 2>::index_range();
  auto& defects = scan->rawDefectPoints[idx];
  auto currentRow = defects.view[boost::indices[all][pointIndexes(currentViewPoint).y()]];
  auto currentColumn = defects.view[boost::indices[pointIndexes(currentViewPoint).x()][all]];

  valueLabel->setText("");

  redChannelRowCurve->setData(new MultiArrayColorSliceSeriesData(currentRow, defects.startCoordinate, defects.finalCoordinate, &RgbColor::red));
  greenChannelRowCurve->setData(new MultiArrayColorSliceSeriesData(currentRow, defects.startCoordinate, defects.finalCoordinate, &RgbColor::green));
  blueChannelRowCurve->setData(new MultiArrayColorSliceSeriesData(currentRow, defects.startCoordinate, defects.finalCoordinate, &RgbColor::blue));
  redChannelColumnCurve->setData(new MultiArrayColorSliceVerticalSeriesData(currentColumn, defects.sampleRate, &RgbColor::red));
  greenChannelColumnCurve->setData(new MultiArrayColorSliceVerticalSeriesData(currentColumn, defects.sampleRate, &RgbColor::green));
  blueChannelColumnCurve->setData(new MultiArrayColorSliceVerticalSeriesData(currentColumn, defects.sampleRate, &RgbColor::blue));

  rowCurve->setData(0);
  columnCurve->setData(0);
  averageColumnCurve->setData(0);
  averageColumnPolynomialCurve->setData(0);
}

void ScanDisplayWindow::updateViewPoint()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  xScanMarker->setXValue(currentViewPoint.x());
  yScanMarker->setYValue(currentViewPoint.y());
  xRowMarker->setXValue(currentViewPoint.x());
  yColumnMarker->setYValue(currentViewPoint.y());

  auto ip = pointIndexes(currentViewPoint);
  columnLabel->setText(QString::number(ip.x()));
  rowLabel->setText(QString::number(ip.y()));
  xLabel->setText(QString::number(currentViewPoint.x(), 'f', 3));
  yLabel->setText(QString::number(currentViewPoint.y(), 'f', 3));

  switch (kindBox->currentIndex()) {
  case 0:
    updateRangeViewPoint();
    break;
  case 1:
    updateResidualsViewPoint();
    break;
  case 2:
  case 3:
    updateDefectsViewPoint();
    break;
  }

  rowPlot->updateAxes();
  rowLeftScale->setScaleDiv(rowPlot->axisScaleDiv(QwtPlot::yLeft));
  rowBottomScale->setScaleDiv(rowPlot->axisScaleDiv(QwtPlot::xBottom));

  columnPlot->updateAxes();
  columnTopScale->setScaleDiv(columnPlot->axisScaleDiv(QwtPlot::xBottom));
  columnRightScale->setScaleDiv(columnPlot->axisScaleDiv(QwtPlot::yLeft));
}

void ScanDisplayWindow::setViewPoint(const QPointF& newViewPoint)
{
  auto currentIndexes = pointIndexes(newViewPoint);
  currentViewPoint = pointFromIndexes(currentIndexes.x(), currentIndexes.y() - 1);
  updateViewPoint();
  scanPlot->replot();
  rowPlot->replot();
  columnPlot->replot();
}


void ScanDisplayWindow::showPlots()
{
  auto model = std::make_shared<ScanDataRangesModel>(scan);
  auto plotView = new uts::plotting::PlotCollectionView;
  //FIX
  //plotView->setStylesheetProvider(std::make_shared<uts::plotting::CascadingStylesheetFileProvider>(getConfigurationPathname("RangesStylesheet.xml")));
  plotView->setModel(model);



  auto magnitudeAction = std::make_shared<PeakSpectrogramAction>(new FftToMagnitude, scan->parameters,
                         "Спектрограмма по ударам (амплитуда)...");
  auto phaseAction = std::make_shared<PeakSpectrogramAction>(new FftToPhaseAngleWithTrend, scan->parameters,
                     "Спектрограмма по ударам (фаза)...");

  plotView->addPlotItemAction(magnitudeAction);
#ifndef USE_TREND_VERSION
  plotView->addPlotItemAction(std::make_shared<PeakSpectrogramAction>(new FftToFaseAngle, scan->parameters,
                              "Спектрограмма по ударам (фаза)..."));
#else
  plotView->addPlotItemAction(phaseAction);
#endif

  plotView->setWindowFlags(Qt::Window);
  plotView->setAttribute(Qt::WA_DeleteOnClose);
  plotView->setWindowTitle("Собранная деталь");
  plotView->showMaximized();
  plotView->show();
}

void ScanDisplayWindow::exportWave(const QString dirname)
{
  saveToWaveDirectory(dirname, *scan);
}

ProcessingParameters ScanDisplayWindow::getProcessingParameters() const
{
  return scan->parameters;
}

void ScanDisplayWindow::applyParameters(const ProcessingParameters& params, ScanFactory& factory)
{
  if(params.smoothingPointsCount != scan->parameters.smoothingPointsCount)
    scan->processingStage = ScanProcessingStage::PeaksDetected;
  else if(params.shouldNormalize != scan->parameters.shouldNormalize)
    scan->processingStage = ScanProcessingStage::DirectionNormalized;
  else if(params.defectRendering.fixedColorScale != scan->parameters.defectRendering.fixedColorScale)
    scan->processingStage = ScanProcessingStage::LinesAligned;
  else
    scan->processingStage = ScanProcessingStage::RawDataObtained;
  factory.startNewScan(params);
  for (auto const & l : scan->lines) factory.addRangeScanLine(l);
  refreshWindow();
}

int ScanDisplayWindow::currentXSize() const
{
  switch (kindBox->currentIndex()) {
  case 0:
    return scan->normalizedRanges.front().view.shape()[0];
  case 1:
    return scan->rangesResiduals.front().view.shape()[0];
  case 2:
  case 3:
    return scan->rawDefectPoints.front().view.shape()[0];
  default:
    return -1;
  }
}

int ScanDisplayWindow::currentYSize() const
{
  switch (kindBox->currentIndex()) {
  case 0:
    return scan->normalizedRanges.front().view.shape()[1];
  case 1:
    return scan->rangesResiduals.front().view.shape()[1];
  case 2:
  case 3:
    return scan->rawDefectPoints.front().view.shape()[1];
  default:
    return -1;
  }
}

double ScanDisplayWindow::getXStartCoordinate() const
{
  switch (kindBox->currentIndex()) {
  case 0:
    return scan->normalizedRanges.front().startCoordinate;
  case 1:
    return scan->rangesResiduals.front().startCoordinate;
  case 2:
  case 3:
    return scan->rawDefectPoints.front().startCoordinate;
  default:
    return 0;
  }
}

double ScanDisplayWindow::getXFinalCoordinate() const
{
  switch (kindBox->currentIndex()) {
  case 0:
    return scan->normalizedRanges.front().finalCoordinate;
  case 1:
    return scan->rangesResiduals.front().finalCoordinate;
  case 2:
  case 3:
    return scan->rawDefectPoints.front().finalCoordinate;
  default:
    return 0;
  }
}

QPoint ScanDisplayWindow::pointIndexes(const QPointF& p) const
{
  int xsize = currentXSize();
  auto xStart = getXStartCoordinate();
  auto xFinal = getXFinalCoordinate();

  if (xsize < 0)
      return QPoint(0, 0);

  auto yiter = (scan->normalizedRanges.front().lineCoordinates.back() > scan->normalizedRanges.front().lineCoordinates.front())
               ? boost::lower_bound(scan->normalizedRanges.front().lineCoordinates, p.y())
               : boost::lower_bound(scan->normalizedRanges.front().lineCoordinates, p.y(), std::greater<double>());
  auto iy = std::distance(scan->normalizedRanges.front().lineCoordinates.begin(), yiter);
  auto ix = static_cast<int>(std::floor((p.x() - xStart) * xsize / std::abs(xFinal - xStart) + 0.5));
  return QPoint(std::max(0, std::min(ix, xsize - 1)), iy);
}

QPointF ScanDisplayWindow::pointFromIndexes(int ix, int iy) const
{
  int xsize = currentXSize();
  auto xStart = getXStartCoordinate();
  auto xFinal = getXFinalCoordinate();

  if (xsize < 0) return QPoint(0, 0);

  return QPointF(std::abs(xFinal - xStart) * double(ix) / double(xsize) + xStart,
                 scan->normalizedRanges.front().lineCoordinates[iy]);
}

void ScanDisplayWindow::moveMarkers(int dx, int dy)
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  int xsize = currentXSize();
  int ysize = currentYSize();
  if (xsize < 0 || ysize < 0) return;

  auto sampleRate = scan->normalizedRanges.front().sampleRate;

  auto xStart = getXStartCoordinate();
  auto xFinal = getXFinalCoordinate();
  int ix = pointIndexes(currentViewPoint).x();
  int iy = pointIndexes(currentViewPoint).y();

  ix = std::max(0, std::min(ix + dx, xsize));
  iy = std::max(0, std::min(iy + dy, ysize));

  if (iy < scan->normalizedRanges.front().lineCoordinates.size()) {
    currentViewPoint = pointFromIndexes(ix, iy);
    updateViewPoint();
    scanPlot->replot();
    rowPlot->replot();
    columnPlot->replot();
  }
}

ScanPlotEventFilter::ScanPlotEventFilter(QObject* parent)
  : QObject(parent)
{ }

bool ScanPlotEventFilter::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent*>(event);

    if (keyEvent->modifiers() == Qt::NoModifier)
      switch (keyEvent->key()) {
      case Qt::Key_Down:
        emit moveMarkers(0, -1);
        return true;
      case Qt::Key_Up:
        emit moveMarkers(0, 1);
        return true;
      case Qt::Key_Left:
        emit moveMarkers(-1, 0);
        return true;
      case Qt::Key_Right:
        emit moveMarkers(1, 0);
        return true;

      }
    else if (keyEvent->modifiers() == Qt::ShiftModifier) {
      if(keyEvent->key() == Qt::Key_Shift) {
        if(!keyEvent->isAutoRepeat())
          emit shiftPressed();
        return true;
      }
    }
    return true;
  } else if (event->type() == QEvent::KeyRelease) {
    auto keyEvent = static_cast<QKeyEvent*>(event);
    if(keyEvent->key() == Qt::Key_Shift) {
      if(!keyEvent->isAutoRepeat())
        emit shiftReleased();
      else keyEvent->ignore();
      return true;
    }
    return true;
  }

  return QObject::eventFilter(watched, event);
}
