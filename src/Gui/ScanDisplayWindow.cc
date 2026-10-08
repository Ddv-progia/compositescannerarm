#pragma once

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
#include <QSplitter>
#include <QSettings>
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
#include <UCL/Stylesheets/Cascade.hh>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>

#include "Core/ConfigurationLocator.hh"
#include "Core/ImageProcessing.hh"
#include "Core/LoadScanTask.hh"
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
#include "Gui/EditorWindow.hh"
#include "Gui/PeakAndBscanVTKView.hh"

//new 2024
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>

#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSuperEllipsoid.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>

#include <iostream>
#include <string> 
//***

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
//#include <vtkProperty.h>
//#include <vtkRenderer.h>
//#include <vtkSphereSource.h>

#include <QApplication>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
//#include <QPushButton>
//#include <QVBoxLayout>
#include <QVTKOpenGLNativeWidget.h>
#include <cmath>

//end new 2024


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

void ScanDisplayWindow::slot_clicked(vtkObject* ob, unsigned long lo, void* vo, void* vi)
{
    auto inter = static_cast<vtkRenderWindowInteractor*>(ob);
    auto b = inter->GetEventPosition();
    std::cout << "Clicked. " << b[0] << " " << b[1] << " " << b[2] << b[3] << std::endl;

    auto iren = this->armVtkRenderWidget->genericOpenGLRenderWindow->GetInteractor();
    auto a = iren->GetEventPosition();
    std::cout << "Clicked. " << a[0] << " " << a[1] << " " << a[2] << a[3] << std::endl;
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
  kindBox->addItem("Точки неоднородности (пропорциональный цвет)");
  kindBox->addItem("Точки неоднородности (фиксированный цвет)");
  //kindBox->addItem("Дефектные точки (пропорциональный цвет)");
  //kindBox->addItem("Дефектные точки (фиксированный цвет)");

  plotBox = new QComboBox;

  auto outlineSelectedDefectsButton = new QPushButton("Обвести неоднородность");
  outlineSelectedDefectsButton ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto additionalPlotsButton = new QPushButton("Детальный график");
  additionalPlotsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  additionalPlotsButton->setCheckable(true);
  auto plotsButton = new QPushButton("Графики");
  plotsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  plotsButton->setCheckable(false);
  auto resizeToWindowButton = new QPushButton("Сохранять пропорции");
  resizeToWindowButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  resizeToWindowButton->setCheckable(true);
  auto defectsButton = new QPushButton("Отображать неоднородности");
  defectsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  defectsButton->setCheckable(true);

  auto defectClassificationButton = new QPushButton("Классификатор");
  defectClassificationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  defectClassificationButton->setCheckable(true);

  auto commandLayout = new QHBoxLayout;

  commandLayout->addWidget(kindBox);
  commandLayout->addWidget(plotBox);
  commandLayout->insertStretch(2,3);

  //commandLayout->addStretch();
  commandLayout->addWidget(outlineSelectedDefectsButton);
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
  //scanPlot->plotLayout()->setCanvasMargin(-1);
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



  averValueGlobalLabel = new QLabel;
  averValueGlobalLabel->setFrameShadow(QFrame::Sunken);
  averValueGlobalLabel->setFrameShape(QFrame::Box);

  maxValueGlobalLabel = new QLabel;
  maxValueGlobalLabel->setFrameShadow(QFrame::Sunken);
  maxValueGlobalLabel->setFrameShape(QFrame::Box);

  minValueGlobalLabel = new QLabel;
  minValueGlobalLabel->setFrameShadow(QFrame::Sunken);
  minValueGlobalLabel->setFrameShape(QFrame::Box);

  averValueLabel = new QLabel;
  averValueLabel->setFrameShadow(QFrame::Sunken);
  averValueLabel->setFrameShape(QFrame::Box);

  maxValueLabel = new QLabel;
  maxValueLabel->setFrameShadow(QFrame::Sunken);
  maxValueLabel->setFrameShape(QFrame::Box);

  minValueLabel = new QLabel;
  minValueLabel->setFrameShadow(QFrame::Sunken);
  minValueLabel->setFrameShape(QFrame::Box);

  auto infoLayout = new QFormLayout;
  infoLayout->addRow("Столбец", columnLabel);
  infoLayout->addRow("Строка", rowLabel);
  infoLayout->addRow("X", xLabel);
  infoLayout->addRow("Y", yLabel);
  infoLayout->addRow("Уровень", valueLabel);
  auto vLayoutDummy = new QVBoxLayout();
  vLayoutDummy->addSpacing(10);
  auto vLayoutDummy2 = new QVBoxLayout();
  vLayoutDummy2->addSpacing(10);
  infoLayout->addRow(vLayoutDummy);
  infoLayout->addRow("Макс.", maxValueLabel);
  infoLayout->addRow("Мин.", minValueLabel);
  infoLayout->addRow("Среднее", averValueLabel);
  infoLayout->addRow(vLayoutDummy2);
  infoLayout->addRow("Макс. Глобал", maxValueGlobalLabel);
  infoLayout->addRow("Мин. Глобал", minValueGlobalLabel);
  infoLayout->addRow("Среднее Глобал", averValueGlobalLabel);
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
  regionPicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton);

  auto regionRosePicker = new QwtPlotPicker(scanPlot->canvas());
  regionRosePicker->setRubberBand(QwtPicker::RectRubberBand);
  regionRosePicker->setStateMachine(new  QwtPickerDragRectMachine);
  regionRosePicker->setTrackerMode(QwtPicker::AlwaysOff);
  regionRosePicker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::RightButton, Qt::ShiftModifier);


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

  mainLayout = new QVBoxLayout;
  mainWidget = new QWidget;

  widgetLayout = new QGridLayout;
  widgetLayout->setContentsMargins(2, 2, 2, 2);
  widgetLayout->setSpacing(1);

  infoWidget = new QWidget;
  infoWidget->setLayout(infoLayout);
  rangeSelector = new QWidget;

  commandScrollArea = new QScrollArea;
  commandScrollArea->setFrameStyle(1);
  commandScrollArea->setLayout(commandLayout);
  commandScrollArea->setWidgetResizable(true);

  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
  
  //armVtkRenderWidget->setRenderWindow(genericOpenGLRenderWindow.Get());
  //armVtkRenderWidget->setRenderWindow(renderWindow.Get());


  // connect the buttons
  //QObject::connect(randomizeButton, &QPushButton::released, this, &ScanDisplayWindow::Randomize);
  vtkNew<vtkEventQtSlotConnect> slotConnector;
  this->armVtkRenderWidget->Connections = slotConnector;
#if VTK890
  this->armVtkRenderWidget->Connections->Connect(
      this->armVtkRenderWidget->genericOpenGLRenderWindow->GetInteractor(),
      vtkCommand::LeftButtonPressEvent, this,
      SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));


  //this->armVtkRenderWidget->Connections->Connect(
  //    this->armVtkRenderWidget->genericOpenGLRenderWindow->GetInteractor(),
  //    vtkCommand::LeftButtonReleaseEvent, this,
  //    SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
  ////this->armVtkRenderWidget->Connections->Connect(
  ////    this->armVtkRenderWidget->genericOpenGLRenderWindow->GetInteractor(),
  ////    vtkCommand::LeftButtonPressEvent, this,
  ////    SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
#else
  this->armVtkRenderWidget->Connections->Connect(
      //QVTKOpenGLNativeWidget
      this->armVtkRenderWidget->RenderWindow()->GetInteractor(),
      vtkCommand::LeftButtonPressEvent, this,
      SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
#endif
  dockLayout->addWidget(armVtkRenderWidget);
  layoutContainer->setLayout(dockLayout);
  armVtkRenderWidget->showQuantizedPoints(scan);
  //armVtkRenderWidget->showNView(scan, 2);
  ////commandScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  widgetLayout->addWidget(commandScrollArea, 0, 0);
  //widgetLayout->addLayout(commandLayout, 0, 0);
  widgetLayout->addWidget(rangeSelector, 0, 2);

  widgetLayout->addWidget(scanScrollArea, 3, 0);
  widgetLayout->addWidget(scanRightColorScale, 3, 1);
  widgetLayout->addWidget(rightHolder, 3, 2);
  widgetLayout->addWidget(bottomHolder, 4, 0);
  widgetLayout->addWidget(infoWidget, 4, 2);
  //widgetLayout->addWidget(layoutContainer, 5, 0, 1, 2);

  widgetLayout->setColumnStretch(0, 9);
  widgetLayout->setColumnStretch(2, 1);
  widgetLayout->setRowStretch(3, 5);
  widgetLayout->setRowStretch(4, 1);
  //widgetLayout->setRowStretch(5, 1);
  

  //auto splitter = new QSplitter(Qt::Vertical);
  //splitter->addWidget(mainWidget);
  //splitter->addWidget(armVtkRenderWidget);
  //splitter->setStretchFactor(0, 5);
  //splitter->setStretchFactor(1, 1);

  setLayout(widgetLayout);


  createColoredRangeSelector();

  connect(plotsButton, SIGNAL(clicked()), this, SLOT(showPlots()));
  connect(kindBox, SIGNAL(activated(int)), this, SLOT(updatePlotList()));
  connect(kindBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePlotGeometryOnCurrentIndexChanged(int)));
  kindBox->setCurrentIndex(0);
  connect(pointPicker, SIGNAL(selected(const QPointF&)), this, SLOT(setViewPoint(const QPointF&)));
  connect(regionPicker, SIGNAL(selected(const QRectF&)), this, SLOT(normalizeRegion(const QRectF&)));
  connect(regionRosePicker, SIGNAL(selected(const QRectF&)), this, SLOT(getRegionFrequencyRose(const QRectF&)));
  connect(scanPlotEventFilter, SIGNAL(moveMarkers(int, int)), this, SLOT(moveMarkers(int, int)));
  connect(scanPlotEventFilter, SIGNAL(changeExtremums()), this, SLOT(changeExtremums()));

  connect(outlineSelectedDefectsButton, SIGNAL(clicked()), this, SLOT (moveAlongDefects()));

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
    defectsMarker->setDefects( plotDefectsModel->currentDefects() );
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
        case Extremum::Diff: {
            for (auto& range : scan->commonNormalizedRanges) {
                range.view = range.diffView;
                range.extremum = Extremum::Diff;
            }
            break;
        }
        case Extremum::DiffOnTable: {
            for (auto& range : scan->commonNormalizedRanges) {
                range.view = range.diffOnTableView;
                range.extremum = Extremum::DiffOnTable;
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

void ScanDisplayWindow::moveAlongDefects()
{
    const auto defs = defectsMarker->selectedDefects;
    DefectSearchingParameters params = scan->parameters.defectSearching;
    emit moveAlongSelectedDefect(defs, params);
    return Q_SLOT void();
}

QPoint ScanDisplayWindow::specIndex(QPoint& viewPoint, std::size_t nRange)
{
  // из индексов в точки спектра
  double sample;
  int line = viewPoint.y();
  double xSize = scan->normalizedRanges.front().view.val.shape()[0] / 3;
  int size = scan->normalizedSpec[line][nRange].samples.size();

  sample = size / xSize;
  sample *= viewPoint.x() / 3;

  return(QPoint(std::min(size, static_cast<int>(sample)), line));  //TODO не нужен /3 (в xSize И в sample *= )??? //TODO в min( должно быть xSize !?
}

void ScanDisplayWindow::normalizeSpec(QPoint beginPoint, QPoint endPoint, std::vector<std::vector<RangeScanLine>>& spec)
{//***//***//
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
        if ((spec.size()>row)&&(spec[row].size()>nRange)) {
            //if (end.x() >= spec[row][nRange].samples.size() || end.y() >= spec.size()) 
            //{
            //    end.setX(spec[row][nRange].samples.size() - 1);
            //    end.setY(spec.size() - 1);
            //}

            if (end.x() >= spec[row][nRange].samples.size()) 
            {
                end.setX(spec[row][nRange].samples.size() - 1);
            }
            if (end.y() >= spec.size()) 
            {
                end.setY(spec.size() - 1);
            }

            for (auto column = begin.x(); column <= end.x(); column++) {
                acc(spec[row][nRange].samples[column]);
            }
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
  //scan->processingStage = ScanProcessingStage::DirectionNormalized;
  scan->processingStage = ScanProcessingStage::SpecNormalized;
  //scan->processingStage = ScanProcessingStage::SpectreRestructuredOnPeaksCoordinates;
  refreshWindow();
}

void ScanDisplayWindow::refreshWindow()
{
  emit refreshScan(scan);
  QWidget* win = this;
  while (win && ! dynamic_cast<QMdiSubWindow*>(win)) win = win->parentWidget();
  if (win) win->deleteLater();
}

void ScanDisplayWindow::normalizeRegionExternalStart(const QRect& rect) {
    normalizeRegionIntIndex(rect, false);
}

void ScanDisplayWindow::normalizeRegionIntIndex(const QRect& rect, const bool needAddRegion)
{
  QPoint beginPoint, endPoint;
  beginPoint = rect.topLeft();
  endPoint = rect.bottomRight();

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

  if(endPoint.y() >= scan->normalizedRanges.begin()->view.val.shape()[1])
      endPoint.setY(scan->normalizedRanges.begin()->view.val.shape()[1] - 1);
  if(endPoint.x() >= scan->normalizedRanges.begin()->view.val.shape()[0])
      endPoint.setX(scan->normalizedRanges.begin()->view.val.shape()[0] - 1);
  if(beginPoint.y() >= scan->normalizedRanges.begin()->view.val.shape()[1])
      beginPoint.setY(scan->normalizedRanges.begin()->view.val.shape()[1] - 1);
  if(beginPoint.x() >= scan->normalizedRanges.begin()->view.val.shape()[0])
      beginPoint.setX(scan->normalizedRanges.begin()->view.val.shape()[0] - 1);

  ///* for(int i = 1;i<6;i++) 
  //    normalizeSpec(QPoint(beginPoint.x()/i,beginPoint.y()/i),QPoint(endPoint.x()/i,endPoint.y()/i),scan->normalizedSpec);*/
  //normalizeSpec(beginPoint, endPoint, scan->spec);
  if ((scan->parameters.specNormalizationIJRect.size()>0) &&
      (scan->parameters.specNormalizationIJRect.front().xLowLeft == scan->parameters.specNormalizationIJRect.front().xTopRight) &&
      (scan->parameters.specNormalizationIJRect.front().yLowLeft == scan->parameters.specNormalizationIJRect.front().yTopRight)) {
      scan->parameters.specNormalizationIJRect.erase(scan->parameters.specNormalizationIJRect.begin());
  }
  if (needAddRegion) {
      RectForNormalization rectForNormalization = { beginPoint.x(), beginPoint.y() , 0.0, endPoint.x(), endPoint.y(), 0.0 };
      scan->parameters.specNormalizationIJRect.push_back(rectForNormalization);
  }
  //this->applyParameters(scan->parameters,*this->scanFactory);
  normalizeSpec(beginPoint, endPoint, scan->normalizedSpec);
}

void ScanDisplayWindow::normalizeRegion(const QRectF& rect, const bool needAddRegion)
{
  QPoint beginPoint, endPoint;
  beginPoint = pointIndexes(rect.bottomLeft());
  endPoint = pointIndexes(rect.topRight());
  //normalizeRegionIntIndex(QRect(beginPoint, endPoint), needAddRegion);
  //return;
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

  if (endPoint.y() >= scan->normalizedRanges.begin()->view.val.shape()[1])  endPoint.setY(scan->normalizedRanges.begin()->view.val.shape()[1] - 1);
  if (endPoint.x() >= scan->normalizedRanges.begin()->view.val.shape()[0])  endPoint.setX(scan->normalizedRanges.begin()->view.val.shape()[0] - 1);
  if (beginPoint.y() < 0)  beginPoint.setY(0);
  if (beginPoint.x() < 0)  beginPoint.setX(0);

  ///* for(int i = 1;i<6;i++) 
  //    normalizeSpec(QPoint(beginPoint.x()/i,beginPoint.y()/i),QPoint(endPoint.x()/i,endPoint.y()/i),scan->normalizedSpec);*/
  //normalizeSpec(beginPoint, endPoint, scan->spec);
  if ((scan->parameters.specNormalizationIJRect.size()>0)&&
      (scan->parameters.specNormalizationIJRect.front().xLowLeft == scan->parameters.specNormalizationIJRect.front().xTopRight)&&
      (scan->parameters.specNormalizationIJRect.front().yLowLeft == scan->parameters.specNormalizationIJRect.front().yTopRight)) {
      scan->parameters.specNormalizationIJRect.erase(scan->parameters.specNormalizationIJRect.begin());
  }
  if (needAddRegion) {
      RectForNormalization rectForNormalization = { beginPoint.x(), beginPoint.y() , 0.0, endPoint.x(), endPoint.y(), 0.0 };
      scan->parameters.specNormalizationIJRect.push_back(rectForNormalization);
  }
  //this->applyParameters(scan->parameters,*this->scanFactory);
  normalizeSpec(beginPoint, endPoint, scan->normalizedSpec);
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

  if(endPoint.y() >= scan->normalizedRanges.begin()->view.val.shape()[1])  endPoint.setY(scan->normalizedRanges.begin()->view.val.shape()[1] - 1);
  if(endPoint.x() >= scan->normalizedRanges.begin()->view.val.shape()[0])  endPoint.setX(scan->normalizedRanges.begin()->view.val.shape()[0] - 1);
  if(beginPoint.y() >= scan->normalizedRanges.begin()->view.val.shape()[1])  beginPoint.setY(scan->normalizedRanges.begin()->view.val.shape()[1] - 1);
  if(beginPoint.x() >= scan->normalizedRanges.begin()->view.val.shape()[0])  beginPoint.setX(scan->normalizedRanges.begin()->view.val.shape()[0] - 1);

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
		  //closenessTable->item(closenessTable->rowCount()-1,2*nMin+3)->setBackground(QColor(Qt::yellow));
          auto localBrush = closenessTable->item(closenessTable->rowCount() - 1, 2 * nMin + 3)->background();
          localBrush.setColor(QColor(Qt::yellow));
		  closenessTable->item(closenessTable->rowCount()-1,2*nMin+3)->setBackground(localBrush);
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

  FrequencyRoseWidget* rose = new FrequencyRoseWidget(rangedPlots, scan->parameters.colorStopsList, scan->parameters.factorForFrequencyRoseWidget, this);
  rose->factorSpinBox->setValue(scan->parameters.factorForFrequencyRoseWidget);
  rose->show();
  scan->parameters.factorForFrequencyRoseWidget = rose->factorSpinBox->value();
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

void ScanDisplayWindow::load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
{
    this->loadLastOpenDir();
    auto pathnames = QFileDialog::getOpenFileNames(this, "Открыть", lastOpenDir, "Все файлы сканера (*.js *.csp)");
    if (pathnames.isEmpty()) return;

    bool newPartCreated = false;
    for (auto& pathname : pathnames) {
        QString normalizedSuffix = QFileInfo(pathname).suffix().toLower();
        if (normalizedSuffix == "js") {
            auto ew = new EditorWindow(pathname, this);
            ew->setAttribute(Qt::WA_DeleteOnClose);
            mdiArea->addSubWindow(ew);
            ew->showMaximized();
        }
        else if (normalizedSuffix == "csp") {
            try {
                taskExecutor.enqueue(new LoadScanTask(pathname, *scanFactory, processingParameters, false));
            }
            catch (DbException& exc) {
                QMessageBox::critical(this, "Ошибка", exc.what());
            }
            catch (uts::Exception& exc) {
                auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
                if (msg) {
                    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
                }
                else {
                    QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
                }
            }
        }
    }

    lastOpenDir = QFileInfo(pathnames.back()).dir().path();
    //settings->setValue(QString::fromUtf8("lastOpenDir"), lastOpenDir);
    this->saveLastOpenDir();
}

void ScanDisplayWindow::save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
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

void ScanDisplayWindow::saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
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
  std::vector<float> mins, maxs, avers, diffs, diffOnTable;

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
    diffs.push_back(range.diff);
    diffOnTable.push_back(range.diff*2);
  }
  QwtColorMap* colorMap = new FixedColorMap(scan->parameters.colorStopsList);//(min,max);
  auto commonRanges = constructCommonRanges();
  if(scan->parameters.shouldNormalize)
    commonRanges.push_back(FrequencyRange{ commonRanges.front().from, commonRanges.back().to });
  auto selector = new ColoredRangeSelector(maxs, mins, avers, diffs, diffOnTable, commonRanges, colorMap);
  auto layout = new QHBoxLayout;
  layout->setContentsMargins(1, 1, 1, 1);
  
  layout->addWidget(selector);

  rangeSelector->setLayout(layout);
  connect(selector, SIGNAL(selectRange(int, Extremum)), SLOT(onSelectRange(int, Extremum)));
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
  auto extremum = scan->parameters.extremumOfRanges[idx];
  if(showCommonRange) {
    normalizedRanges = &scan->commonNormalizedRanges;
    idx = commonRangeNum;
  }
  else{
      //switch (scan->normalizedRanges[idx].extremum){
      //switch (scan->parameters.extremumOfRanges[idx]) {
      switch (extremum) {
      case Extremum::Max:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].maxView;
          break;
      case Extremum::Min:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].minView;
          break;
      case Extremum::Aver:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].averView;
          break;
      case Extremum::Diff:
          scan->normalizedRanges[idx].view = scan->normalizedRanges[idx].diffView;
          break;
      }
  }
  auto spec = new QwtPlotSpectrogram;
  if (scan->parameters.shouldNormalize) {
    spec->setData(new NormalizedRangeRasterData((*normalizedRanges)[idx]));
  }
  else {
    spec->setData(new RangeRasterData(scan->ranges, idx));
  }
  //spec->setRenderThreadCount(0);
  spec->setRenderThreadCount(3);

  if (scan->parameters.defectRendering.fixedColorScale)
    spec->setColorMap(new FixedColorMap(scan->parameters.colorStopsList));
  else
    spec->setColorMap(new StandardColorMap);
  spec->attach(scanPlot);
  //****************10/07/2025
 //// A color bar on the right axis
 // auto colorMap2 = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
 // colorMap2->addColorStop(0.1, Qt::cyan);
 // colorMap2->addColorStop(0.6, Qt::green);
 // colorMap2->addColorStop(0.95, Qt::yellow);

 // QwtScaleWidget* rightAxis = scanPlot->axisWidget(QwtPlot::yLeft);
 // rightAxis->setTitle("Intensity");
 // rightAxis->setColorBarEnabled(true);
 // QwtInterval interval = spec->data()->interval(Qt::ZAxis);
 // //rightAxis->setColorMap(d_data->range(), colorMap);
 // rightAxis->setColorMap(interval, colorMap2);

 // scanPlot->setAxisScale(QwtPlot::yLeft, interval.minValue(), interval.maxValue());
 // scanPlot->enableAxis(QwtPlot::yLeft);

 // //plotLayout()->setAlignCanvasToScales(true);
 // scanPlot->replot();

  //****************10/07/2025

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
  armVtkRenderWidget->SetPolyDataSource(idx, armVtkRenderWidget->pointSource, normalizedRanges);
  //armVtkRenderWidget->inputMapper->GetLookupTable()->SetRange(armVtkRenderWidget->pointSource->GetScalarRange());
  armVtkRenderWidget->inputMapper->UseLookupTableScalarRangeOn();
  armVtkRenderWidget->inputMapper->SetScalarRange(armVtkRenderWidget->pointSource->GetScalarRange());
  armVtkRenderWidget->inputMapper->Modified();
  armVtkRenderWidget->inputMapper->Update();
  //armVtkRenderWidget->renderWindow->Render();
  
  //vtkRendererCollection* renderers = armVtkRenderWidget->genericOpenGLRenderWindow->GetRenderers();
  //auto numberOfItems = renderers->GetNumberOfItems();
  //renderers->InitTraversal();
  //for (auto i = 0; i < numberOfItems; ++i)
  //{
  //    renderers->GetNextItem()->Render();
  //}
}

void ScanDisplayWindow::onSelectRange(int idx, Extremum ex)
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
  auto selin = selected.indexes();
  QSet<int> rowSet;
  //QSet<int> uniqueSet = originalList.toSet();
  for (auto& index : selin) {

      rowSet.insert(index.row());
  }
  for(auto & index : rowSet) {
    auto defectNum = table->model()->index(index, 0 ).data().toInt();
    auto defect = plotDefectsModel->currentDefects().at(defectNum - 1);
    defectsMarker->selectContour(defect);
  }
  //for(auto & index : selected.front().indexes()) {
  //  auto defectNum = table->model()->index(index.row(), 0, index.parent()).data().toInt();
  //  auto defect = plotDefectsModel->currentDefects().at(defectNum - 1);
  //  defectsMarker->selectContour(defect);
  //}
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

  armVtkRenderWidget->SetPolyDataSource(idx, armVtkRenderWidget->pointSource, &(scan->rangesResiduals));
  armVtkRenderWidget->renderWindow->Render();

}

void ScanDisplayWindow::updateDefectPointsPlot()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;
  auto defectPoints = scan->renderedDefectPoints[idx];
  auto item = new DefectPointsItem(scan->renderedDefectPoints[idx]);

  item->attach(scanPlot);

  // недописано //TODO 
  //armVtkRenderWidget->createImageDataFromDefectsView(armVtkRenderWidget->imageDefectsView, &defectPoints);
  //auto va = scan->scanArm.parameters.headAndScanCollectorParameters.currentNumArea;
  //auto he = scan->scanArm.parameters.headAndScanCollectorParameters.height;
  //auto wi = scan->scanArm.parameters.headAndScanCollectorParameters.width;
  //auto xDim = defectPoints.view.shape()[0];
  //auto yDim = defectPoints.view.shape()[1];
  //double xSpace = double(wi) / xDim;
  //double ySpace = double(he) / yDim;
  ////double xSpace = double(wi) / double(xDim);
  ////double ySpace = double(he) / double(yDim);
  //armVtkRenderWidget->imageDefectsView->SetSpacing(xSpace, ySpace, 1);
  ////armVtkRenderWidget->imageDefectsView->SetSpacing(wi/ xDim, he / yDim, 1);
  ////armVtkRenderWidget->imageDefectsView->SetDimensions(xDim, yDim, 1);
  ////auto df = armVtkRenderWidget->imageDefectsView->GetDimensions();
  //armVtkRenderWidget->axial->GetMapper()->SetInputData(armVtkRenderWidget->imageDefectsView);
  //armVtkRenderWidget->axial->SetDisplayExtent(0, xDim-1, 0, yDim-1, 0, 0);
  //armVtkRenderWidget->axial->SetInputData(armVtkRenderWidget->imageDefectsView);
  //armVtkRenderWidget->axial->ForceOpaqueOn();
  //vtkNew<vtkTransform> transform1a;
  //transform1a->PostMultiply();
  ////transform1a->Translate(xDim, 0.0, 0.0);
  //transform1a->Translate(wi, 0.0, 0.0);
  //armVtkRenderWidget->axial->SetUserTransform(transform1a);
  //armVtkRenderWidget->axial->Modified();
  //armVtkRenderWidget->axial->Update();
  //armVtkRenderWidget->leftRenderer->AddActor(armVtkRenderWidget->axial);
  ////armVtkRenderWidget->inputMapper->UseLookupTableScalarRangeOff();
  ////armVtkRenderWidget->inputMapper->SetScalarRange(armVtkRenderWidget->pointSource->GetScalarRange());
  //////armVtkRenderWidget->inputMapper->CreateDefaultLookupTable();
  //////armVtkRenderWidget->inputMapper->GetLookupTable()->SetRange(0,255*256*256);
  ////armVtkRenderWidget->inputMapper->Modified();
  ////armVtkRenderWidget->inputMapper->Update();
  ////auto num = armVtkRenderWidget->renderWindow->GetRenderers()->GetNumberOfItems();
  //vtkRendererCollection* renderers = armVtkRenderWidget->renderWindow->GetRenderers();
  //if (renderers->GetNumberOfItems() < 1)
  //{
  //    return;
  //}
  //renderers->InitTraversal();
  ////vtkRenderer* ren0 = renderers->GetNextItem();
  ////// Bottom item.
  ////vtkRenderer* ren1 = renderers->GetNextItem();
  //vtkRenderer* ren2;
  //while (ren2 = renderers->GetNextItem() )
  //{
  //    try
  //    {
  //        ren2->Render();
  //    }
  //    catch (const std::exception&)
  //    {
  //        //Do nothing&
  //    }
  //};
  // end недописано
}

void ScanDisplayWindow::updateFixedColorDefectPointsPlot()
{
  auto idx = plotBox->currentIndex();
  if (idx < 0) return;

  auto item = new DefectPointsItem(scan->renderedDefectPointsFixedColor[idx]);
  item->attach(scanPlot);
}


//vtkNew<vtkDiscretizableColorTransferFunction> ScanDisplayWindow::buildCTF(bool const& raduga, std::vector<ColorStop> colors)
//{
//
//    vtkNew<vtkDiscretizableColorTransferFunction> ctf;
//
//    ctf->SetColorSpaceToRGB();
//    ctf->SetScaleToLinear();
//    ctf->SetNanColor(0.5, 0.5, 0.5);
//    //ctf->SetBelowRangeColor(0.0, 0.0, 0.0);
//
//    if (colors.size() > 0) {
//        int ans = std::stoi(colors[0].color.substr(1), 0, 16);
//        double r = ((ans >> 16) & 0xff ) / 255;
//        double b = (ans & 0xff) / 255;
//        double g = ((ans >> 8) & 0xff)/255;
//        ctf->SetAboveRangeColor(r, g, b);
//        ans = std::stoi(colors.at(colors.size()-1).color.substr(1), 0, 16);
//        r = ((ans >> 16) & 0xff) / 255;
//        b = (ans & 0xff )/ 255;
//        g = ((ans >> 8) & 0xff )/ 255;
//        ctf->SetBelowRangeColor(r, g, b);
//    }
//    else {
//
//        ctf->SetAboveRangeColor(1.0, 1.0, 1.0);
//        ctf->SetBelowRangeColor(0.0, 0.0, 0.0);
//    }
//        ctf->UseAboveRangeColorOn();
//        ctf->UseBelowRangeColorOn();
//
//    if (raduga)
//    {
//        ctf->AddRGBPoint(-1.0, 1.0, 0.0, 0.0);                 // Red
//        ctf->AddRGBPoint(-2.0 / 3.0, 1.0, 128.0 / 255.0, 0.0); // Orange #ff8000
//        ctf->AddRGBPoint(-1.0 / 3.0, 1.0, 1.0, 0.0);           // Yellow
//        ctf->AddRGBPoint(0.0, 0.0, 1.0, 0.0);                  // Green  #00ff00
//        ctf->AddRGBPoint(1.0 / 3.0, 0.0, 1.0, 1.0);            // Cyan
//        ctf->AddRGBPoint(2.0 / 3.0, 0.0, 0.0, 1.0);            // Blue
//        ctf->AddRGBPoint(1.0, 128.0 / 255.0, 0.0, 1.0);        // Violet #8000ff
//        ctf->SetNumberOfValues(7);
//    }
//    else {
//        for (auto colorPair : colors) {
//            int ans = stoi(colorPair.color.substr(1), 0, 16);
//            auto r = (ans >> 16) & 0xff;
//            auto b = ans & 0xff;
//            auto g = (ans >> 8) & 0xff;
//            //int i = c.toUInt();
//            //unsigned int x;
//            //std::stringstream ss;
//            //ss << std::hex << colorPair.color.substr(1);
//            //ss >> x;
//            //// output it as a signed type
//            //std::cout << static_cast<int>(x) << std::endl;
//            //auto r = (x >> 16) & 0xff;
//            //auto b = x & 0xff;
//            //auto g = (x>>8) & 0xff;
//            ctf->AddRGBPoint(colorPair.val, r/255.0,g/255.0,b/255.0);
//        }
//        ctf->SetNumberOfValues(colors.size());
//    }
//    ctf->DiscretizeOff();
//    return ctf;
//}
//
//void ScanDisplayWindow::showQuantizedPoints()
//{
//    vtkNew<vtkNamedColors> colors;
//
//    //vtkNew<vtkPointSource> pointSource;
//    //pointSource->SetNumberOfPoints(100);
//    //pointSource->Update();
//    vtkNew<vtkPolyData> pointSource;
//    vtkNew<vtkPolyData> peakRtPolyData;
//    //pointSource->DeepCopy(sphere);
//    
//    ///******* готовим точки пиков
//    int numOfPoints = 0;
//    double t = 0;
//    double x, y,z;
//
//
//    vtkSmartPointer<vtkPoints> peakRtpoints = vtkSmartPointer<vtkPoints>::New();
//    vtkSmartPointer<vtkCellArray> peakRtcells = vtkSmartPointer<vtkCellArray>::New();
//    vtkSmartPointer<vtkFloatArray> peakValueArr = vtkSmartPointer<vtkFloatArray>::New();
//    peakValueArr->SetName("value");
//    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::min>> akumRt;
//    {
//        for (auto& line : scan->scanArm.rtPeaks) {
//            vtkSmartPointer<vtkPolyLine> peakRtPolyLine = vtkSmartPointer<vtkPolyLine>::New();
//            for (int j= 0; j < line.size(); j++) {
//                for (auto peak : line.at(j)) {
//                    auto idOfPoint = peakRtpoints->InsertNextPoint(peak.x, peak.y, peak.z+50.0 );
//                    peakRtPolyLine->GetPointIds()->InsertNextId(idOfPoint);
//                    auto averIndexOfPeak = (peak.endIndex + peak.beginIndex) / 2;
//                    auto averValue = scan->scanArm.sound.samples.at(averIndexOfPeak);
//                    peakValueArr->InsertNextTuple1(averValue);
//                    akumRt(averValue);
//                }
//            }
//            peakRtcells->InsertNextCell(peakRtPolyLine);
//        }
//        //stageProgressed();
//    }
//    double minValueRt = boost::accumulators::min(akumRt);
//    double maxValueRt = boost::accumulators::max(akumRt);
//
//    peakRtPolyData->SetPoints(peakRtpoints);
//    peakRtPolyData->SetLines (peakRtcells );
//    peakRtPolyData->SetVerts (peakRtcells );
//    peakRtPolyData->GetPointData()->SetScalars(peakValueArr);
//    peakRtPolyData->Modified();
//
//    vtkNew<vtkPolyData> trajectoryPolyData;
//    vtkSmartPointer<vtkPoints> pointsTrajectory = vtkSmartPointer<vtkPoints>::New();
//    vtkSmartPointer<vtkCellArray> cellsTrajectory = vtkSmartPointer<vtkCellArray>::New();
//    vtkSmartPointer<vtkFloatArray> scalarsTrajectory = vtkSmartPointer<vtkFloatArray>::New();
//    scalarsTrajectory->SetName("Y");
//    vtkSmartPointer<vtkPolyLine> trajectoryPolyLine = vtkSmartPointer<vtkPolyLine>::New();
//    for (auto elem : scan->scanArm.trajectory.pos)
//    {
//                auto idOfPoint = pointsTrajectory->InsertNextPoint(elem.x, elem.y, elem.z -50.0);
//                trajectoryPolyLine->GetPointIds()->InsertNextId(idOfPoint);
//
//    };
//    cellsTrajectory->InsertNextCell(trajectoryPolyLine);
//    trajectoryPolyData->SetPoints(pointsTrajectory);
//    trajectoryPolyData->SetLines(cellsTrajectory);
//    trajectoryPolyData->SetVerts(cellsTrajectory);
//    trajectoryPolyData->Modified();
//
//
//
//    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
//    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
//    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
//    scalars->SetName("range");
//    auto nRanges = scan->normalizedSpec.begin()->size();
//    //for (int nRange = 0; nRange < nRanges; nRange++) {
//    int nRange = 0;
//    {
//        for (auto& line : scan->normalizedSpec) {
//            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
//            if (line.size() > nRange) {
//                //for (auto& pointValue : line[nRange].samples)
//                for (int i = 0; i < line[nRange].samples.size(); i++) {
//                    auto pointValue = line[nRange].samples[i];
//                    auto peak = line[nRange].peaks[i];
//                    auto idOfPoint = points->InsertNextPoint(peak.x, peak.y,peak.z);
//                    polyLine->GetPointIds()->InsertNextId(idOfPoint);
//                    scalars->InsertNextTuple1(pointValue);
//                    //scalars->InsertNextTuple1(peak.y);
//                }
//            }
//            cells->InsertNextCell(polyLine);
//        }
//        //stageProgressed();
//    }
//
//    pointSource->SetPoints(points);
//    pointSource->SetLines (cells );
//    //pointSource->SetVerts (cells );
//    //pointSource->Modified();
//    pointSource->GetPointData()->SetScalars(scalars);
//    pointSource->GetCellData()->SetScalars(scalars);
//    pointSource->Modified();
//
//    std::cout << "There are " << pointSource->GetNumberOfPoints() << " points."
//        << std::endl;
//    ///******* end готовим точки пиков
//
//    vtkNew<vtkQuantizePolyDataPoints> quantizeFilter;
//    //quantizeFilter->SetInputConnection(pointSource->GetOutputPort());
//    quantizeFilter->SetInputData(pointSource);
//    quantizeFilter->SetQFactor(scan->parameters.headAndScanCollectorParameters.currentNumArea);
//    //quantizeFilter->SetQFactor(.1);
//    quantizeFilter->Update();
//
//    vtkNew<vtkQuantizePolyDataPoints> quantizeFilterRt;
//    quantizeFilterRt->SetInputData(peakRtPolyData);
//    quantizeFilterRt->SetQFactor(scan->parameters.headAndScanCollectorParameters.currentNumArea);
//    quantizeFilterRt->Update();
//
//    //vtkPolyData* quantized = quantizeFilter->GetOutput();
//    //std::cout << "There are " << quantized->GetNumberOfPoints()
//    //    << " quantized points." << std::endl;
//    //for (vtkIdType i = 0; i < pointSource->GetNumberOfPoints(); i++)
//    //{
//    //    double pOrig[3];
//    //    double pQuantized[3];
//    //    pointSource->GetPoint( i, pOrig);
//    //    if (i <= quantized->GetNumberOfPoints()) {
//    //        quantized->GetPoints()->GetPoint(i, pQuantized);
//    //        std::cout << "Point " << i << " : (" << pOrig[0] << ", " << pOrig[1] << ", "
//    //            << pOrig[2] << ")" << " (" << pQuantized[0] << ", "
//    //            << pQuantized[1] << ", " << pQuantized[2] << ")" << std::endl;
//    //    }
//    //    else {
//    //        std::cout << "Point " << i << " : (" << pOrig[0] << ", " << pOrig[1] << ", "
//    //            << pOrig[2] << ")" << " ( no point)" << std::endl;
//    //    }
//    //}
//
//    //double radius = 0.02;
//    double radius = scan->parameters.headAndScanCollectorParameters.currentNumArea/2;
//    vtkNew<vtkSphereSource> sphereSource;
//    sphereSource->SetRadius(radius);
//
//    auto params = scan->parameters;
//    
//    //vtkNew<vtkLookupTable> lookupTable;
//    //lookupTable->SetNumberOfTableValues(params.colorStopsList.size());
//    //int i = 0;
//    //double minValue;
//    //double maxValue;
//
//    //boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::min>> akum;
//    //for (auto colorPair : params.colorStopsList)
//    //{
//    //    akum(colorPair.val);
//    //}
//    //minValue = boost::accumulators::min(akum);
//    //maxValue = boost::accumulators::max(akum);
//
//    //for (auto colorPair : params.colorStopsList)
//    //{
//    //    int ans = stoi(colorPair.color.substr(1), 0, 16);
//    //    auto r = (ans >> 16) & 0xff;
//    //    auto b = ans & 0xff;
//    //    auto g = (ans >> 8) & 0xff;
//
//    //    lookupTable->SetTableValue(i, r/255.0,g / 255.0,b / 255.0,1);
//    //    i++;
//    //}
//    //lookupTable->SetRampToLinear();
//    //lookupTable->SetTableRange(minValue, maxValue);
//    //lookupTable->Build();
//
//    vtkNew<vtkGlyph3DMapper> inputMapper;
//    inputMapper->SetInputData(pointSource);
//    //inputMapper->SetScalarRange(
//    //    pointSource->GetPointData()->GetScalars()->GetRange()[0],
//    //    pointSource->GetPointData()->GetScalars()->GetRange()[1]);
//    inputMapper->SetLookupTable(buildCTF(false , params.colorStopsList));
//    inputMapper->SetSourceConnection(sphereSource->GetOutputPort());
//    inputMapper->ScalarVisibilityOn();
//    inputMapper->InterpolateScalarsBeforeMappingOff();
//    //inputMapper->UseLookupTableScalarRangeOn();
//    //inputMapper->SetScalarModeToUseCellData();
//    //inputMapper->SetScalarModeToUsePointData();
//    //inputMapper->MapScalars(0.4);
//    inputMapper->ScalingOff();
//    //inputMapper->SetLookupTable(lookupTable);
//    inputMapper->Update();
//    vtkNew<vtkActor> inputActor;
//    inputActor->SetMapper(inputMapper);
//    //inputActor->GetProperty()->SetColor(colors->GetColor3d("Orchid").GetData());
//
//
//    vtkNew<vtkScalarBarActor> scalarBar;
//    scalarBar->SetLookupTable(inputMapper->GetLookupTable());
//    //scalarBar->SetLookupTable(lookupTable);
//    scalarBar->SetTitle("Range");
//    //scalarBar->SetNumberOfLabels(4);
//    scalarBar->UnconstrainedFontSizeOn();
//    scalarBar->DragableOn();
//    scalarBar->DrawFrameOn();
//    //scalarBar->SetPosition(0, 50);
//
//
//    //vtkNew<vtkPolyDataMapper> polyMapper;
//    //polyMapper->SetInputData(pointSource);
//    //polyMapper->Update();
//    //vtkNew<vtkActor> polyActor;
//    //polyActor->SetMapper(polyMapper);
//    //polyActor->GetProperty()->EdgeVisibilityOn();
//    //polyActor->GetProperty()->SetEdgeColor(colors->GetColor3d("Black").GetData());
//
//    vtkNew<vtkGlyph3DMapper> quantizedMapper;
//    quantizedMapper->SetInputConnection(quantizeFilter->GetOutputPort());
//    quantizedMapper->SetSourceConnection(sphereSource->GetOutputPort());
//    quantizedMapper->ScalarVisibilityOff();
//    quantizedMapper->ScalingOff();
//
//    vtkNew<vtkActor> quantizedActor;
//    quantizedActor->SetMapper(quantizedMapper);
//    quantizedActor->GetProperty()->SetColor( colors->GetColor3d("DodgerBlue").GetData());
//
//    vtkNew<vtkPolyDataMapper> trajectoryMapper; 
//    trajectoryMapper->SetInputData(trajectoryPolyData);
//    //trajectoryMapper->ScalarVisibilityOff();
//    //trajectoryMapper->ScalingOff();
//
//    vtkNew<vtkActor> trajectoryActor;
//    trajectoryActor->SetMapper(trajectoryMapper);
//    trajectoryActor->GetProperty()->SetColor( colors->GetColor3d("Orange").GetData());
//    trajectoryActor->GetProperty()->SetLineWidth(scan->parameters.headAndScanCollectorParameters.currentNumArea);
//
//    //***
//
//    vtkNew<vtkParametricSuperEllipsoid> parametricSuperEllipsoid;
//    parametricSuperEllipsoid->SetN1(0.2);
//    parametricSuperEllipsoid->SetN2(0.2);
//    parametricSuperEllipsoid->SetXRadius(radius);
//    parametricSuperEllipsoid->SetYRadius(radius);
//    parametricSuperEllipsoid->SetZRadius(radius/3);
//
//
//    vtkSmartPointer<vtkParametricFunctionSource> parametricFunctionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
//    parametricFunctionSource->SetParametricFunction(parametricSuperEllipsoid);
//    parametricFunctionSource->SetUResolution(11);
//    parametricFunctionSource->SetVResolution(11);
//    parametricFunctionSource->SetWResolution(11);
//    parametricFunctionSource->Update();
//
//
//    vtkNew<vtkSphereSource> sphereSourceRt;
//    sphereSourceRt->SetRadius(radius);
//
//    vtkNew<vtkGlyph3DMapper> inputMapperRt;
//    //inputMapper->SetInputConnection(pointSource->GetOutputPort());
//    inputMapperRt->SetInputData(peakRtPolyData);
//    //inputMapperRt->SetSourceConnection(sphereSourceRt->GetOutputPort());
//    inputMapperRt->SetSourceConnection(parametricFunctionSource->GetOutputPort());
//    //auto tableRt = buildCTF(false, params.colorStopsList);
//    //tableRt->SetRange(minValueRt, maxValueRt);
//    //inputMapperRt->SetLookupTable(tableRt);
//    inputMapperRt->SetRange(peakRtPolyData->GetScalarRange());
//    inputMapperRt->CreateDefaultLookupTable();
//    //inputMapperRt->SetUseLookupTableScalarRange(1);
//    inputMapperRt->ScalarVisibilityOn();
//    inputMapperRt->ScalingOff();
//    inputMapperRt->Update();
//
//    vtkNew<vtkScalarBarActor> scalarBarRt;
//    scalarBarRt->SetLookupTable(inputMapperRt->GetLookupTable());
//    scalarBarRt->SetTitle("RT Amplitude");
//    scalarBarRt->UnconstrainedFontSizeOn();
//    scalarBarRt->DragableOn();
//    scalarBarRt->DrawFrameOn();
//    scalarBarRt->SetOrientationToHorizontal();
//
//    vtkNew<vtkActor> inputActorRt;
//    inputActorRt->SetMapper(inputMapperRt);
//    inputActorRt->GetProperty()->SetColor(colors->GetColor3d("Green").GetData());
//    
//    vtkNew<vtkGlyph3DMapper> quantizedMapperRt;
//    quantizedMapperRt->SetInputConnection(quantizeFilterRt->GetOutputPort());
//    quantizedMapperRt->SetSourceConnection(parametricFunctionSource->GetOutputPort());
//    quantizedMapperRt->ScalarVisibilityOff();
//    quantizedMapperRt->ScalingOff();
//
//    vtkNew<vtkActor> quantizedActorRt;
//    quantizedActorRt->SetMapper(quantizedMapperRt);
//    quantizedActorRt->GetProperty()->SetColor(colors->GetColor3d("Aquamarine").GetData());
////***
//
//    // Define viewport ranges.
//    // (xmin, ymin, xmax, ymax)
//    double leftViewport[4] = { 0.0, 0.0, 0.5, 1.0 };
//    double rightViewport[4] = { 0.5, 0.0, 1.0, 1.0 };
//
//    // Setup both renderers.
//    vtkNew<vtkRenderer> leftRenderer;
//    renderWindow->AddRenderer(leftRenderer);
//    leftRenderer->SetViewport(leftViewport);
//    leftRenderer->SetBackground(colors->GetColor3d("Bisque").GetData());
//
//    vtkNew<vtkRenderer> rightRenderer;
//    renderWindow->AddRenderer(rightRenderer);
//    rightRenderer->SetViewport(rightViewport);
//    rightRenderer->SetBackground(colors->GetColor3d("PaleTurquoise").GetData());
//
//   
//    renderer->AddActor(inputActor);
//    renderer->AddActor(inputActorRt);
//    renderer->AddActor2D(scalarBar);
//    renderer->AddActor2D(scalarBarRt); 
//    
//    leftRenderer->AddActor(inputActor);
//    leftRenderer->AddActor(inputActorRt);
//    //leftRenderer->AddActor2D(scalarBar);
//    //leftRenderer->AddActor2D(scalarBarRt);
//    //leftRenderer->AddActor(polyActor);
//    leftRenderer->AddActor(trajectoryActor);
//    rightRenderer->AddActor(quantizedActor);
//    rightRenderer->AddActor(quantizedActorRt);
//
//    leftRenderer->ResetCamera();
//    leftRenderer->ResetCameraClippingRange();
//
//
//    rightRenderer->SetActiveCamera(leftRenderer->GetActiveCamera());
//
//    scalarBarWidgetRt->SetInteractor(interactor);
//    scalarBarWidgetRt->SetScalarBarActor(scalarBarRt);
//    scalarBarWidgetRt->On();
//
//    scalarBarWidget->SetInteractor(interactor);
//    scalarBarWidget->SetScalarBarActor(scalarBar);
//    scalarBarWidget->On();
//
//    renderWindow->SetSize(640, 360);
//    renderWindow->SetWindowName("QuantizePolyDataPoints");
//    interactor->SetRenderWindow(renderWindow);
//
//    renderWindow->Render();
//    interactor->Start();
//}
//
void ScanDisplayWindow::updatePlot()
{
  scanPlot->detachItems(QwtPlotItem::Rtti_PlotSpectrogram);
  scanPlot->detachItems(DefectPointsItem::Rtti_DefectPointsItem);
  auto indx = kindBox->currentIndex();
  updatePlotGeometryOnCurrentIndexChanged(indx);
  switch (indx) {
  case 0:
    updateRangesPlot();
    scanRightColorScale->show();
    xScanMarker->setLinePen(Qt::black, 1, Qt::DashLine);
    yScanMarker->setLinePen(Qt::black, 1, Qt::DashLine);
    break;
  case 1:
    updateResidualsPlot();
    scanRightColorScale->show();
    xScanMarker->setLinePen(Qt::white, 1, Qt::DashLine);
    yScanMarker->setLinePen(Qt::white, 1, Qt::DashLine);
    break;
  case 2:
    updateDefectPointsPlot();
    scanRightColorScale->hide();
    xScanMarker->setLinePen(Qt::green, 1, Qt::DashLine);
    yScanMarker->setLinePen(Qt::green, 1, Qt::DashLine);
    break;
  case 3:
    updateFixedColorDefectPointsPlot();
    scanRightColorScale->hide();
    xScanMarker->setLinePen(Qt::yellow, 1, Qt::DashLine);
    yScanMarker->setLinePen(Qt::yellow, 1, Qt::DashLine);
    break;
  }

  scanLeftScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::yLeft));
  scanTopScale->setScaleDiv(scanPlot->axisScaleDiv(QwtPlot::xBottom));
  updateViewPoint();

  scanPlot->replot();
  rowPlot->replot();
  columnPlot->replot();
  //armVtkRenderWidget->showQuantizedPoints(scan);
}

void ScanDisplayWindow::updatePlotGeometryOnCurrentIndexChanged(int index)
{
    //return currentXSize_;
    //switch (kindBox->currentIndex()) {
    switch (index) {
    case 0:
        currentXSize_ =  scan->normalizedRanges.front().view.val.shape()[0];
        currentYSize_ = scan->normalizedRanges.front().view.val.shape()[1];
        xStartCoordinate = scan->normalizedRanges.front().startCoordinate;
        xFinalCoordinate = scan->normalizedRanges.front().finalCoordinate;
        yStartCoordinate = scan->normalizedRanges.front().lineCoordinates.front();
        yFinalCoordinate = scan->normalizedRanges.front().finalLineCoordinates.front();
        break;
    case 1:
        currentXSize_ = scan->rangesResiduals.front().view.val.shape()[0];
        currentYSize_ = scan->rangesResiduals.front().view.val.shape()[1];
        xStartCoordinate = scan->rangesResiduals.front().startCoordinate;
        xFinalCoordinate = scan->rangesResiduals.front().finalCoordinate;
        yStartCoordinate = scan->rangesResiduals.front().lineCoordinates.front();
        yFinalCoordinate = scan->rangesResiduals.front().finalLineCoordinates.size() > 0 ? scan->rangesResiduals.front().finalLineCoordinates.front() : scan->rangesResiduals.front().lineCoordinates.front();
        //yFinalCoordinate = scan->rangesResiduals.front().finalLineCoordinates.front();
        break;
    case 2:
    case 3:
        currentXSize_ = scan->rawDefectPoints.front().view.shape()[0];
        currentYSize_ = scan->rawDefectPoints.front().view.shape()[1];
        xStartCoordinate = scan->rawDefectPoints.front().startCoordinate;
        xFinalCoordinate = scan->rawDefectPoints.front().finalCoordinate;
        yStartCoordinate = scan->rawDefectPoints.front().lineCoordinates.front();
        yFinalCoordinate = scan->rawDefectPoints.front().finalLineCoordinates.size() > 0 ? scan->rawDefectPoints.front().finalLineCoordinates.front() : scan->rawDefectPoints.front().lineCoordinates.front();
        //yFinalCoordinate = scan->rawDefectPoints.front().finalLineCoordinates.front();
        break;
    default:
        currentXSize_ = -1;
        currentYSize_ = -1;
        xStartCoordinate = std::numeric_limits<double>::quiet_NaN();
        xFinalCoordinate = std::numeric_limits<double>::quiet_NaN();
        yStartCoordinate = std::numeric_limits<double>::quiet_NaN();
        yFinalCoordinate = std::numeric_limits<double>::quiet_NaN();
    }
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
              else if (scan->parameters.extremumOfRanges[i] == ::Extremum::Diff) {
                  path = "icons/button_diff.ico";
                  strExtremum = " Diff";
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

  int indexX;
  int indexY;
  try {
      indexX = pointIndexes(currentViewPoint).x();
      indexY = pointIndexes(currentViewPoint).y();
  }
  catch (uts::Exception& exc) {
      auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
      if (msg) {
          QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
      }
      else {
          QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
      }
  }
  catch (...) {
          //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
        QString str = QString::fromUtf8("Ошибка при получении pointIndexes(currentViewPoint).    x() или y()\r\n");
        QMessageBox::critical(this, "Ошибка", str+QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
  }
  QString qStringValue = "";
  try {
      if ((!range->view.val.empty()) && (!range->view.val[indexX].empty())) {
          auto value = range->view.val[indexX][indexY];
          qStringValue = QString::number(value, 'f', 3);
      }
  }
  catch (uts::Exception& exc) {
      auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
      if (msg) {
          QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
      }
      else {
          QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
      }
  }
  catch (...) {
      //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
      QString str = QString::fromUtf8("Ошибка при получении range->view[indexX][indexY]\r\n");
      QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
  }

  valueLabel->setText(qStringValue);

  QString qStringAverGlobalValue = "";
try {
    if (std::isfinite( range->aver)) {
        qStringAverGlobalValue = QString::number(range->aver, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->aver\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

averValueGlobalLabel->setText(qStringAverGlobalValue);

QString qStringMinGlobalValue = "";
try {
    if (std::isfinite(range->min)) {
        qStringMinGlobalValue = QString::number(range->min, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->min\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

minValueGlobalLabel->setText(qStringMinGlobalValue);


QString qStringMaxGlobalValue = "";
try {
    if (std::isfinite(range->max)) {
        qStringMaxGlobalValue = QString::number(range->max, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->aver\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

maxValueGlobalLabel->setText(qStringMaxGlobalValue);

QString qStringAverValue = "";
try {
    if (std::isfinite(range->view.aver)) {
        qStringAverValue = QString::number(range->view.aver, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->aver\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

averValueLabel->setText(qStringAverValue);

QString qStringMinValue = "";
try {
    if (std::isfinite(range->view.min)) {
        qStringMinValue = QString::number(range->view.min, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->min\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

minValueLabel->setText(qStringMinValue);


QString qStringMaxValue = "";
try {
    if (std::isfinite(range->view.max)) {
        qStringMaxValue = QString::number(range->view.max, 'f', 3);
    }
}
catch (uts::Exception& exc) {
    auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
    if (msg) {
        QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    }
    else {
        QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
    }
}
catch (...) {
    //QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
    QString str = QString::fromUtf8("Ошибка при получении range->aver\r\n");
    QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
}

maxValueLabel->setText(qStringMaxValue);


  rowCurve->setData(new MultiArraySliceSeriesData(range->view.val[boost::indices[all][pointIndexes(currentViewPoint).y()]], range->startCoordinate,
                    range->finalCoordinate));
  try {
      columnCurve->setData(new MultiArraySliceVerticalSeriesData(range->view.val[boost::indices[pointIndexes(currentViewPoint).x()][all]],
          range->lineCoordinates));
  }
  catch (...) {
      QString str = QString::fromUtf8("Ошибка при columnCurve->setData\r\n");
      QMessageBox::critical(this, "Ошибка", str + QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
  }
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

  valueLabel->setText(QString::number(range.view.val[pointIndexes(currentViewPoint).x()][pointIndexes(currentViewPoint).y()], 'f', 3));

  rowCurve->setData(new MultiArraySliceSeriesData(range.view.val[boost::indices[all][pointIndexes(currentViewPoint).y()]], range.startCoordinate,
                    range.finalCoordinate));
  columnCurve->setData(new MultiArraySliceVerticalSeriesData(range.view.val[boost::indices[pointIndexes(currentViewPoint).x()][all]],
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

//void ScanDisplayWindow::SetPointInfoWidget(QPoint& ip)

void ScanDisplayWindow::SetPointInfoWidget(const int columnValue, const int rowValue, const double xValue, const double yValue, int signCount, double valueValue)
{
    //columnLabel->setText(QString::number(ip.x()));
    //rowLabel->setText(QString::number(ip.y()));
    //xLabel->setText(QString::number(currentViewPoint.x(), 'f', 3));
    //yLabel->setText(QString::number(currentViewPoint.y(), 'f', 3));
    columnLabel->setText(QString::number(columnValue));
    rowLabel->setText(QString::number(rowValue));
    xLabel->setText(QString::number(xValue, 'f', signCount));
    yLabel->setText(QString::number(yValue, 'f', signCount));
    if (isnan(valueValue)) {
        valueLabel->setText("nan");
    }
    else {
        if (isinf(valueValue)) {
            valueLabel->setText("inf");
        }
        else valueLabel->setText(QString::number(valueValue, 'f', signCount));
    }
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
  SetPointInfoWidget(ip.x(), ip.y(), currentViewPoint.x(), currentViewPoint.y());

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
  auto stylesheet = uts::stylesheets::cascadingStylesheetFromFile(Configuration::getConfigurationPathname("etc/RangesStylesheet.xml"));
  plotView->setStylesheet(stylesheet);
  plotView->setModel(model);



  auto magnitudeAction = std::make_shared<PeakSpectrogramAction>(new FftToMagnitude, scan->parameters,
                         "Спектрограмма по ударам (амплитуда)...");
  auto phaseAction = std::make_shared<PeakSpectrogramAction>(new FftToPhaseAngleWithTrend, scan->parameters,
                     "Спектрограмма по ударам (фаза)...");
  auto phaseActionNoTrend = std::make_shared<PeakSpectrogramAction>(new FftToPhaseAngle, scan->parameters,
                     "Спектрограмма по ударам NoTrend (фаза)...");

  plotView->addPlotItemAction(magnitudeAction);
#ifndef USE_TREND_VERSION
  plotView->addPlotItemAction(std::make_shared<PeakSpectrogramAction>(new FftToFaseAngle, scan->parameters,
                              "Спектрограмма по ударам (фаза)..."));
#else
  plotView->addPlotItemAction(phaseAction);
#endif
  plotView->addPlotItemAction(phaseActionNoTrend);

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

void ScanDisplayWindow::setProcessingParameters(ProcessingParameters parametersIn)
{
  scan->parameters = parametersIn;
}

/// <summary>
/// Увеличиваем отсканированную область до заданных размеров (newXSize и newYSize), размеры в мм
/// </summary>
/// <param name="params"></param>
/// <param name="factory"></param>
void ScanDisplayWindow::multiSizeLines(const ProcessingParameters& params, ScanFactory& factory, double newXSize, double newYSize)
{
        scan->processingStage = ScanProcessingStage::RawDataObtained;
        scan->parameters = params;
        factory.startNewScan(params);
        ::std::vector< ::SourceScanLine > liness;
        bool isFirst = false;
        double minLineCoord;
        double maxLineCoord;
        if (scan->lines.back().lineCoordinate != scan->lines.front().lineCoordinate) {
            minLineCoord = scan->lines.front().lineCoordinate;
            maxLineCoord = scan->lines.front().lineCoordinate;
        }
        else {
            return;
        }
        for (auto const& l : scan->lines) {
            liness.push_back(l);
            if (l.lineCoordinate > maxLineCoord) maxLineCoord = l.lineCoordinate;
            if (l.finalLineCoordinate > maxLineCoord) maxLineCoord = l.finalLineCoordinate;
            if (l.lineCoordinate < minLineCoord) minLineCoord = l.lineCoordinate;
            if (l.finalLineCoordinate < minLineCoord) minLineCoord = l.finalLineCoordinate;
            
            auto lsize = l.finalCoordinate - l.startCoordinate; // размер строки в мм по оси X
            auto stepCount = int(newXSize / std::abs(lsize)); //сколько целых раз старая строка помещается в новой

            for (auto i = 0; i < stepCount -1; i++) {
                std::copy(std::begin(l.samples), std::end(l.samples), std::back_inserter(liness.back().samples));
            }

            auto ostatokDliny = newXSize - std::abs(lsize) * stepCount;
            int adderCount = int(ostatokDliny * l.samples.size() / std::abs(lsize)); // количество звуковых тиков, чтобы "закрыть" остаток линии
            std::copy(std::begin(l.samples), std::begin(l.samples)+ adderCount, std::back_inserter(liness.back().samples));


            if (lsize > 0) {
                liness.back().finalCoordinate = l.startCoordinate + newXSize;
            }
            else {
                liness.back().startCoordinate = l.finalCoordinate + newXSize;
            }
            liness.back().timestampEnd = liness.back().timestampEnd * (newXSize);
            //factory.addRangeScanLine(liness.back());
        }
        auto sizeYOfLines = maxLineCoord - minLineCoord;
        auto deltaLines = sizeYOfLines / liness.size();
        double baseCoord = minLineCoord;
        while (baseCoord <= newYSize) {
            for (auto l : liness) {
                baseCoord = baseCoord + deltaLines;
                if (baseCoord > newYSize) break;
                l.lineCoordinate = baseCoord;
                l.finalLineCoordinate = baseCoord;
                factory.addRangeScanLine(l);
            }
        }
        refreshWindow();
}

/// <summary>
/// Кратно увеличиваем отсканированную область (в newXSize раз и в newYSize раз)
/// </summary>
/// <param name="params"></param>
/// <param name="factory"></param>
void ScanDisplayWindow::doubleSizeLines(const ProcessingParameters& params, ScanFactory& factory, double newXSize, double newYSize)
{
        scan->processingStage = ScanProcessingStage::RawDataObtained;
        scan->parameters = params;
        factory.startNewScan(params);
        ::std::vector< ::SourceScanLine > liness;
        bool isFirst = false;
        double minLineCoord;
        double maxLineCoord;
        if (scan->lines.back().lineCoordinate != scan->lines.front().lineCoordinate) {
            minLineCoord = scan->lines.front().lineCoordinate;
            maxLineCoord = scan->lines.front().lineCoordinate;
        }
        for (auto const& l : scan->lines) {
            liness.push_back(l);
            if (l.lineCoordinate > maxLineCoord) maxLineCoord = l.lineCoordinate;
            if (l.finalLineCoordinate > maxLineCoord) maxLineCoord = l.finalLineCoordinate;
            if (l.lineCoordinate < minLineCoord) minLineCoord = l.lineCoordinate;
            if (l.finalLineCoordinate < minLineCoord) minLineCoord = l.finalLineCoordinate;

            for (auto i = 0; i < newXSize-1; i++) {
                std::copy(std::begin(l.samples), std::end(l.samples), std::back_inserter(liness.back().samples));
            }
            liness.back().finalCoordinate = liness.back().finalCoordinate * (newXSize);
            liness.back().timestampEnd = liness.back().timestampEnd * (newXSize);
            factory.addRangeScanLine(liness.back());
        }
        auto sizeOfLines = maxLineCoord - minLineCoord;
        auto deltaLines = sizeOfLines / liness.size();
        double baseCoord = std::max(scan->lines.back().lineCoordinate, scan->lines.front().lineCoordinate);
        for (auto i = 0; i < newYSize-1; i++) {
            for (auto l : liness) {
                baseCoord = baseCoord + deltaLines;
                l.lineCoordinate = baseCoord;
                l.finalLineCoordinate = baseCoord;
                factory.addRangeScanLine(l);
            }
        }
        refreshWindow();
}

void ScanDisplayWindow::doubleLines(const ProcessingParameters& params, ScanFactory& factory)
{
    scan->processingStage = ScanProcessingStage::RawDataObtained;
    scan->parameters = params;
    factory.startNewScan(params);
    ::std::vector< ::SourceScanLine > liness;
    bool isFirst = false;
    double minLineCoord;
    double maxLineCoord;
    if (scan->lines.back().lineCoordinate != scan->lines.front().lineCoordinate) {
        minLineCoord = scan->lines.front().lineCoordinate;
        maxLineCoord = scan->lines.front().lineCoordinate;
    }
    for (auto const& l : scan->lines) {
        factory.addRangeScanLine(l);
        liness.push_back(l);
        if (l.lineCoordinate > maxLineCoord) maxLineCoord = l.lineCoordinate;
        if (l.finalLineCoordinate > maxLineCoord) maxLineCoord = l.finalLineCoordinate;
        if (l.lineCoordinate < minLineCoord) minLineCoord = l.lineCoordinate;
        if (l.finalLineCoordinate < minLineCoord) minLineCoord = l.finalLineCoordinate;
    }
    auto sizeOfLines = maxLineCoord - minLineCoord;
    auto deltaLines = sizeOfLines / liness.size();
    double baseCoord = std::max(scan->lines.back().lineCoordinate, scan->lines.front().lineCoordinate);
    for (auto l : liness) {
        baseCoord = baseCoord + deltaLines;
        l.lineCoordinate = baseCoord;
        l.finalLineCoordinate= baseCoord;
        factory.addRangeScanLine(l);
    }
    refreshWindow();

}
void ScanDisplayWindow::applyParameters(const ProcessingParameters& params, ScanFactory& factory)
{
  // new start commented
  //if(params.smoothingPointsCount != scan->parameters.smoothingPointsCount)
  //  scan->processingStage = ScanProcessingStage::PeaksDetected;
  //else if(params.shouldNormalize != scan->parameters.shouldNormalize)
  //  scan->processingStage = ScanProcessingStage::DirectionNormalized;
  //else if(params.defectRendering.fixedColorScale != scan->parameters.defectRendering.fixedColorScale)
  //  scan->processingStage = ScanProcessingStage::LinesAligned;
  //else
  //  scan->processingStage = ScanProcessingStage::RawDataObtained;
  // new end commented
  scan->processingStage = ScanProcessingStage::RawDataObtained; // new вместо commented
  scan->parameters = params;
  factory.startNewScan(params);  //rem : 04_09_2025
  for (auto const & l : scan->lines) factory.addRangeScanLine(l);
  refreshWindow();
}

void ScanDisplayWindow::ShowNView(int n, bool needToShowOriginalView, bool needShowRandomizedData)
{
        armVtkRenderWidget->showNView(scan, n, needToShowOriginalView, needShowRandomizedData);
}

int ScanDisplayWindow::currentXSize() const
{
    return currentXSize_;
  //switch (kindBox->currentIndex()) {
  //case 0:
  //  return scan->normalizedRanges.front().view.val.shape()[0];
  //case 1:
  //  return scan->rangesResiduals.front().view.val.shape()[0];
  //case 2:
  //case 3:
  //  return scan->rawDefectPoints.front().view.shape()[0];
  //default:
  //  return -1;
  //}
}

int ScanDisplayWindow::currentYSize() const
{
    return currentYSize_;
  //switch (kindBox->currentIndex()) {
  //case 0:
  //  return scan->normalizedRanges.front().view.val.shape()[1];
  //case 1:
  //  return scan->rangesResiduals.front().view.val.shape()[1];
  //case 2:
  //case 3:
  //  return scan->rawDefectPoints.front().view.shape()[1];
  //default:
  //  return -1;
  //}
}

double ScanDisplayWindow::getYStartCoordinate() const
{
    return yStartCoordinate;
}

double ScanDisplayWindow::getYFinalCoordinate() const
{
    return yFinalCoordinate;
}

double ScanDisplayWindow::getXStartCoordinate() const
{
  return xStartCoordinate;
  //switch (kindBox->currentIndex()) {
  //case 0:
  //  return scan->normalizedRanges.front().startCoordinate;
  //case 1:
  //  return scan->rangesResiduals.front().startCoordinate;
  //case 2:
  //case 3:
  //  return scan->rawDefectPoints.front().startCoordinate;
  //default:
  //  return 0;
  //}
}

double ScanDisplayWindow::getXFinalCoordinate() const
{
  return xFinalCoordinate;
  //switch (kindBox->currentIndex()) {
  //case 0:
  //  return scan->normalizedRanges.front().finalCoordinate;
  //case 1:
  //  return scan->rangesResiduals.front().finalCoordinate;
  //case 2:
  //case 3:
  //  return scan->rawDefectPoints.front().finalCoordinate;
  //default:
  //  return 0;
  //}
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
