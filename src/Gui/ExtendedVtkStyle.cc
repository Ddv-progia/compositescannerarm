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

//new 2024
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkGlyph3DMapper.h>
#include <vtkLookupTable.h>

#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkSphereSource.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSuperEllipsoid.h>

#include <iostream>
#include <string> 
//***

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
//#include <vtkGenericOpenGLRenderWindow.h>
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
#include <cmath>

//end new 2024


