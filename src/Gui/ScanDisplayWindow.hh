/*
 * Gui/ScanDisplayWindow.hh
 */

#pragma once

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
#include <QTableView>
#include "Core/LineEncoding.hh"
#include "Core/ScanData.hh"
#include "Core/ScanFactory.hh"
#include "Core/ScanIO.hh"
#include "Gui/Saveable.hh"
#include "Gui/Loadable.hh"
#include "Gui/DefectsTable.hh"
#include "Core/ImageProcessing.hh"


#include "Gui/DefectClassificationTable.hh"


class ScanPlotDefectsMarker: public QwtPlotItem
{
	//отображает контуры дефектов поверх С-скана
	std::vector<Defect> defects;
	std::vector<Defect*> selectedDefects;
public:
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

	virtual void save(BackgroundTaskExecutor& taskExecutor) override;
	virtual void saveAs(BackgroundTaskExecutor& taskExecutor) override;
	virtual void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;

	void exportWave(const QString dirname);
	ProcessingParameters getProcessingParameters() const;
	void applyParameters(const ProcessingParameters& params, ScanFactory& factory);
	Q_SIGNAL void refreshScan(std::shared_ptr<Scan>& scan);
private:
	QString filename;
	std::shared_ptr<Scan> scan;
	int commonRangeNum;
	bool showCommonRange;
	ScanFactory* scanFactory;
	ProcessingParameters* processingParameters;

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
	ScanPlotDefectsMarker* defectsMarker;
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

	QWidget* infoWidget;
	QWidget* rangeSelector;
	QWidget* scanPlotHolder;
	QWidget* rightHolder;
	QWidget* bottomHolder;

	QScrollArea* scanScrollArea;

	void updateRangesPlot();
	void createColoredRangeSelector();
	void updateResidualsPlot();
	void updateDefectPointsPlot();
	void updateFixedColorDefectPointsPlot();
	Q_SLOT void updatePlot();
	Q_SLOT void updatePlotList();
	Q_SLOT void showPlots();
	Q_SLOT void selectRange(int idx, Extremum ex);

	QGridLayout* widgetLayout;
	void swapItemsByIndexes(int firstItemIndex, int secondItemIndex);
	Q_SLOT void setAdditionalGraphicsVisibility(bool isVisible);
	//Q_SLOT void onChangeFactorSpinbox();

	void updateRangeViewPoint();
	void updateResidualsViewPoint();
	void updateDefectsViewPoint();
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
