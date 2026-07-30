#pragma once

/*
 * Gui/ScanDisplayWindow.hh
 */


#include <memory>
#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <QtCore/QString>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollArea>
#include <QVTKOpenGLNativeWidget.h>
#include <QTableView>
#include <QVBoxLayout>

#include "Core/LineEncoding.hh"
#include "Core/ScanData.hh"
#include "Core/ScanFactory.hh"
#include "Core/ScanIO.hh"
#include <vtkEventQtSlotConnect.h>
#include "Gui/Saveable.hh"
#include "Gui/Loadable.hh"
#include "Gui/DefectsTable.hh"
#include "Core/ImageProcessing.hh"

#include "Gui/DefectClassificationTable.hh"

//*** new 2924
 //#include <vtkProperty.h>
 //#include <vtkRenderer.h>
 //std::mt19937
 #include <vtkGenericOpenGLRenderWindow.h>
 #include <vtkMapper.h>
 #include <vtkSphereSource.h>
 #include <vtkDiscretizableColorTransferFunction.h>
 #include <vtkDataSetMapper.h>
#include <vtkScalarBarWidget.h>
#include <vtkQuantizePolyDataPoints.h>
#include <cmath>
#include <cstdlib>
#include <random>


#include <sstream>
#include <vtkAbstractPicker.h>
#include <vtkActor2D.h>
#include <vtkCoordinate.h>
#include <vtkImageActor.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageMapper3D.h>
#include <vtkInteractorStyleImage.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVectorText.h>
#include <Gui/PeakAndBscanVTKView.hh>
#include <array>

#if VTK_VERSION_NUMBER >= 89000000000ULL
#define VTK890 1
#endif

//namespace myStyleSpace {
//
//	class MyStyle : public vtkInteractorStyleImage
//	{
//	public:
//		static MyStyle* New();
//		vtkTypeMacro(MyStyle, vtkInteractorStyleImage);
//
//		std::vector<vtkActor2D*> Numbers;
//
//		void OnLeftButtonDown() override
//		{
//			this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
//				this->Interactor->GetEventPosition()[1],
//				0, // always zero.
//				this->CurrentRenderer);
//			double picked[3];
//			this->Interactor->GetPicker()->GetPickPosition(picked);
//			this->AddNumber(picked);
//
//			// Forward events
//			vtkInteractorStyleImage::OnLeftButtonDown();
//
//			// this->Interactor->GetRenderWindow()->Render();
//			this->Interactor->Render();
//		}
//
//		void AddNumber(double p[3])
//		{
//			vtkNew<vtkNamedColors> colors;
//
//			std::cout << "Adding marker at " << p[0] << " " << p[1]; //<< std::endl;
//
//			// normally, with an image you would do
//			// double* s = image->GetSpacing();
//			// double* o = image->GetOrigin();
//			// p[0] = static_cast<int>( (p[0] - o[0]) / s[0] + 0.5 );
//			p[0] = static_cast<int>(p[0] + 0.5);
//			p[1] = static_cast<int>(p[1] + 0.5);
//
//			std::cout << " -> " << p[0] << " " << p[1] << std::endl;
//
//			// Convert the current number to a string
//			std::stringstream ss;
//			ss << Numbers.size();
//
//			// Create an actor for the text
//			vtkNew<vtkVectorText> textSource;
//			textSource->SetText(ss.str().c_str());
//
//			// get the bounds of the text
//			textSource->Update();
//			const double* bounds = textSource->GetOutput()->GetBounds();
//			// transform the polydata to be centered over the pick position
//			const double center[3] = { 0.5 * (bounds[1] + bounds[0]),
//								0.5 * (bounds[3] + bounds[2]), 0.0 };
//
//			vtkNew<vtkTransform> trans;
//			trans->Translate(-center[0], -center[1], 0);
//			trans->Translate(p[0], p[1], 0);
//
//			vtkNew<vtkTransformPolyDataFilter> tpd;
//			tpd->SetTransform(trans);
//			tpd->SetInputConnection(textSource->GetOutputPort());
//
//			// Create a mapper
//			vtkNew<vtkPolyDataMapper2D> mapper;
//			vtkNew<vtkCoordinate> coordinate;
//			coordinate->SetCoordinateSystemToWorld();
//			mapper->SetTransformCoordinate(coordinate);
//			mapper->SetInputConnection(tpd->GetOutputPort());
//
//			vtkNew<vtkActor2D> actor;
//			actor->SetMapper(mapper);
//			actor->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
//
//			this->CurrentRenderer->AddViewProp(actor);
//			this->Numbers.push_back(actor);
//		}
//	};
//	vtkStandardNewMacro(MyStyle);
//}

 //*** new 2924


class ScanPlotDefectsMarker: public QwtPlotItem
{
	//отображает контуры дефектов поверх С-скана
	std::vector<Defect> defects;
public:
	std::vector<Defect*> selectedDefects;
	ScanPlotDefectsMarker(const QwtText& title = QwtText("Defects marker"));

  virtual void draw (QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect) const override;
  void setDefects(std::vector<Defect> filteredDefects) {
		selectedDefects.clear();
		defects = filteredDefects;
	}
	virtual int rtti() const override;

	//рисует маску, используя переданный набор контуров, впоследствии маска используется в draw()
	QImage buildMaskImage() const;

	//выбор одного отдельного контура, выделение его на экране цветом
	void selectContour(const Defect& defect);
	void removeSelection() {
		selectedDefects.clear();
	}
};


class ScanDisplayWindow : public QWidget, public Saveable, public Loadable
{
	Q_OBJECT
public:
	explicit ScanDisplayWindow(const std::shared_ptr<Scan>& scan, QWidget* parent = 0);

	virtual void save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
	virtual void saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
	virtual void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;

	void exportWave(const QString dirname);
	ProcessingParameters getProcessingParameters() const;
	void doubleLines(const ProcessingParameters& params, ScanFactory& factory);
	void multiSizeLines(const ProcessingParameters& params, ScanFactory& factory,double newXSize, double newYSize);
	void doubleSizeLines(const ProcessingParameters& params, ScanFactory& factory,double newXSize = 2.0, double newYSize = 2.0);
	void applyParameters(const ProcessingParameters& params, ScanFactory& factory);
	void ShowNView(int n, bool needToShowOriginalView = true, bool needShowRandomizedData = false);
	Q_SIGNAL void refreshScan(std::shared_ptr<Scan>& scan);
	Q_SIGNAL void moveAlongSelectedDefect(const std::vector<Defect*>& defects, ::DefectSearchingParameters defParams);

	vtkNew<vtkDiscretizableColorTransferFunction> buildCTF(bool const& raduga, std::vector<ColorStop> colors);
	//void showQuantizedPoints();
	ScanFactory* scanFactory;
	std::mt19937 randEng;
	ScanPlotDefectsMarker* defectsMarker;


private:
	QPointer<PeakAndBscanVTKView> armVtkRenderWidget = new PeakAndBscanVTKView();
	//QVBoxLayout dockLayout;
	QPointer<QVBoxLayout > dockLayout = new QVBoxLayout();
	//QPointer<QVTKOpenGLNativeWidget> armVtkRenderWidget = new QVTKOpenGLNativeWidget();
	vtkSmartPointer<vtkEventQtSlotConnect> Connections;

	QPointer<QWidget > layoutContainer = new QWidget();
	//QPointer<QPushButton> randomizeButton = new QPushButton();

	std::vector<Defect> selectedDefects;

	QString filename;
	std::shared_ptr<Scan> scan;
	int commonRangeNum;
	bool showCommonRange;
	//ProcessingParameters* processingParameters;

	QComboBox* kindBox;
	QComboBox* plotBox;
	QwtPlot* scanPlot;
	QwtPlot* rowPlot;
	QwtPlot* columnPlot;
	PlotDefectsModel* plotDefectsModel;
	QTableView* table;
	QWidget* defectClassificationWindow;
	bool analyseRegion;

	QwtScaleWidget* scanLeftScale;
	QwtScaleWidget* scanTopScale;
	QwtScaleWidget* scanRightColorScale;

	QwtScaleWidget* rowLeftScale;
	QwtScaleWidget* rowBottomScale;

	QwtScaleWidget* columnTopScale;
	QwtScaleWidget* columnRightScale;

	QPointF currentViewPoint;
	QwtPlotMarker* xScanMarker;
	QwtPlotMarker* yScanMarker;
	QwtPlotMarker* xRowMarker;
	QwtPlotMarker* yColumnMarker;
	QwtPlotCurve* rowCurve;
	QwtPlotCurve* columnCurve;
	QwtPlotCurve* averageColumnCurve;
	QwtPlotCurve* averageColumnPolynomialCurve;

	QwtPlotCurve* redChannelRowCurve;
	QwtPlotCurve* greenChannelRowCurve;
	QwtPlotCurve* blueChannelRowCurve;

	QwtPlotCurve* redChannelColumnCurve;
	QwtPlotCurve* greenChannelColumnCurve;
	QwtPlotCurve* blueChannelColumnCurve;

	QLabel* columnLabel;
	QLabel* rowLabel;
	QLabel* xLabel;
	QLabel* yLabel;
	QLabel* valueLabel;
	QLabel* averValueLabel;

	QWidget* infoWidget;
	QWidget* rangeSelector;
	QWidget* scanPlotHolder;
	QWidget* rightHolder;
	QWidget* bottomHolder;

	QScrollArea* scanScrollArea;
	QScrollArea* commandScrollArea;

	void updateRangesPlot();
	void createColoredRangeSelector();
	void updateResidualsPlot();
	void updateDefectPointsPlot();
	void updateFixedColorDefectPointsPlot();
	//void Randomize(vtkSphereSource* sphere, vtkMapper* mapper, vtkGenericOpenGLRenderWindow* window, std::mt19937& randEng);
	Q_SLOT void updatePlot();
	Q_SLOT void updatePlotList();
	Q_SLOT void showPlots();
	Q_SLOT void selectRange(int idx, Extremum ex);

	QGridLayout* widgetLayout;
	QVBoxLayout* mainLayout;
	QWidget* mainWidget;
	void swapItemsByIndexes(int firstItemIndex, int secondItemIndex);
	Q_SLOT void setAdditionalGraphicsVisibility(bool isVisible);
	//Q_SLOT void onChangeFactorSpinbox();

	void updateRangeViewPoint();
	void updateResidualsViewPoint();
	void updateDefectsViewPoint();
	void SetPointInfoWidget(const int columnValue, const int rowValue, const double xValue, const double yValue, int signCount = 3, double valueValue = std::nan("1"));
	void updateViewPoint();
	std::vector<std::vector<float>> getPeaksFromRect(const QRectF& rect);
	QPoint specIndex(QPoint& viewPoint, std::size_t nRange = 0);
	void normalizeSpec(QPoint beginPoint, QPoint endPoint, std::vector<std::vector<RangeScanLine>>& spec);
	Q_SLOT void setViewPoint(const QPointF& newViewPoint);
	Q_SLOT void getRegionFrequencyRose(const QRectF& rect);
	Q_SLOT void normalizeRegion(const QRectF& rect);
	Q_SLOT void refreshWindow();

	int currentXSize() const;
	int currentYSize() const;
	double getXStartCoordinate() const;
	double getXFinalCoordinate() const;
	QPoint pointIndexes(const QPointF& p) const;
	QPointF pointFromIndexes(int ix, int iy) const;
	Q_SLOT void moveMarkers(int dx, int dy);
	Q_SLOT void selectContour(size_t n);
	Q_SLOT void setDefectMask();
	Q_SLOT void changeExtremums();
	Q_SLOT void changeExtremum(Extremum ex);
	Q_SLOT void getSelectedContour(const QItemSelection& selected, const QItemSelection& deselected);
	QTableView* getDefectTableView();

	Q_SLOT void createDefectsMarker();
	Q_SLOT void deleteDefectsMarker();
	
	Q_SLOT void togleWindowSize(bool isPressed);
	Q_SLOT void setDefectsVisible(bool isVisible);
	Q_SLOT void showUserRange();

	//черновая версия, нужен рефакторинг
	Q_SLOT void refreshClassificationParameters(std::vector<DefectType>& defects)
	{
		scan->parameters.defectClassification = defects;
	}

	Q_SLOT void moveAlongDefects();

	Q_SLOT void showClassificationTable(bool isVisible)
	{
		analyseRegion = isVisible;
		if(isVisible){
			defectClassificationWindow = new DefectClassificationWidget(scan);
			bool b = connect(defectClassificationWindow,SIGNAL(defectClassificationRefreshed(std::vector<DefectType>&)),SLOT(refreshClassificationParameters(std::vector<DefectType>&)));
			defectClassificationWindow->show();
		} else {
			defectClassificationWindow->hide();
			defectClassificationWindow->disconnect();
			defectClassificationWindow->deleteLater();
		}
	}


	////////////////////////////////
	Q_SIGNAL void selectRangeByIndex(int idx, Extremum ex);

	LineEncoding requestLineEncoding();
public slots:

	void slot_clicked(vtkObject*, unsigned long, void*, void*);

};

class ScanPlotEventFilter : public QObject
{
	Q_OBJECT
public:
	explicit ScanPlotEventFilter(QObject* parent = 0);
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

protected:
	Q_SIGNAL void moveMarkers(int dx, int dy);
	Q_SIGNAL void changeExtremums();
	Q_SIGNAL void shiftPressed();
	Q_SIGNAL void shiftReleased();
};
