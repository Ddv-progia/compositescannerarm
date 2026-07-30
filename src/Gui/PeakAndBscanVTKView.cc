#pragma once

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
#include "Gui/PeakAndBscanVTKView.hh"

//new 2024
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkContextActor.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkImageMapToColors.h>
#include <vtkImageGridSource.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageMapper.h>
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
#include <vtkRectilinearGrid.h>
//#include <vtkDoubleArray.h>

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>

#include <vtkCubeAxesActor2D.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkTextProperty.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>

#include <vector>
#include <cmath>
#include <cstdlib>

#include <ctime>


#include <iostream>
#include <string> 
//***

#include <vtkContextView.h>
#include <vtkChartMatrix.h>
//#include <vtkVector2i.h>
#include <vtkChart.h>
#include <vtkPlotPoints.h>
#include <vtkPlot.h>

#include <vtkFloatArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
//#include <vtkProperty.h>
//#include <vtkRenderer.h>
//#include <vtkSphereSource.h>

#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkRenderer.h>
#include <vtkTable.h>
#include <vtkVersion.h>

#if VTK_VERSION_NUMBER >= 90220220630ULL
#define VTK_HAS_SETCOLORF 1
#endif
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


//vtkButtonWidget2D_2::vtkButtonWidget2D_2()
//{
//}
//
//void vtkButtonWidget2D_2::createAndPlace(vtkRenderWindowInteractor* interactor, vtkRenderer* renderer, const char* fileName,
//    double* x, double  xSpace, double  y, double size)
//{
//    vtkNew<vtkNamedColors> colorNamed;
//    atext->SetText("XYZ");
//    vtkNew<vtkPolyDataMapper> textMapper;
//    textMapper->SetInputConnection(atext->GetOutputPort());
//    //vtkNew<vtkFollower> textActor;
//    textActor->SetMapper(textMapper);
//    textActor->SetScale(0.02, 0.02, 0.02);
//    textActor->AddPosition(0.1, 0.1, 0.0);
//    textActor->GetProperty()->SetColor(colorNamed->GetColor3d("Peacock").GetData());
//    textActor->GetProperty()->SetAmbientColor(colorNamed->GetColor3d("Peacock").GetData());
//
//    vtkNew<vtkProp3DButtonRepresentation> buttonRepresentation;
//    buttonRepresentation->SetNumberOfStates(1);
//    buttonRepresentation->SetButtonProp(0, textActor);
//    buttonRepresentation->FollowCameraOn();
//
//    vtkSmartPointer<vtkPNGReader> reader = vtkSmartPointer<vtkPNGReader>::New();
//    reader->SetFileName(fileName);
//    reader->Update();
//    vtkSmartPointer<vtkImageData> image1;
//    image1 = reader->GetOutput();
//    //vtkSmartPointer<vtkImageData> image1 = vtkSmartPointer<vtkImageData>::New();
//    //image1 = reader->GetOutput();
//
//
//    //vtkSmartPointer<vtkImageData> image2;
//    //reader->SetFileName(fileName);
//    //reader->Update();
//    //image2 = reader->GetOutput();
//
//    buttonRepresentation2D->SetNumberOfStates(1);
//    buttonRepresentation2D->SetButtonTexture(0, image1);
//    //buttonRepresentation2D->SetButtonTexture(1, image2);
//
//    this->SetInteractor(interactor);
//    //buttonWidgetX->SetRepresentation(buttonRepresentation);
//    this->SetRepresentation(buttonRepresentation2D);
//
//    vtkNew<vtkCoordinate> upperRight;
//    upperRight->SetCoordinateSystemToNormalizedDisplay();
//    upperRight->SetValue(*x, y);
//    double bds[6];
//    bds[0] = upperRight->GetComputedDisplayValue(renderer)[0];
//    bds[1] = bds[0] + size;
//    bds[2] = upperRight->GetComputedDisplayValue(renderer)[1];
//    bds[3] = bds[2] + size;
//    bds[4] = bds[5] = 0.0;
//
//    vtkNew<vtkCoordinate> screenValue;
//    screenValue->SetCoordinateSystemToDisplay();
//    screenValue->SetValue(bds[1], bds[2]);
//    screenValue->SetCoordinateSystemToNormalizedDisplay();
//
//    auto a = screenValue->GetComputedValue(renderer)[0];
//    //double coordinates[3];
//    //coordinates[0] = *x;
//    //coordinates[1] = y;
//    //coordinates[2] = 0;
//
//    //renderer->SetWorldPoint(coordinates);
//    //renderer->WorldToView();
//    //renderer->GetViewPoint(coordinates);
//
//    //*x = coordinates[0];
//    //y = coordinates[1];
//
//    //renderer->ViewportToNormalizedViewport(*x, y);
//
//    //vtkViewport->ViewportToNormalizedViewport(*x, y);
//    *x = *x + xSpace;
//
//    buttonRepresentation2D->SetPlaceFactor(1);
//    buttonRepresentation2D->PlaceWidget(bds);
//
//}
void CreateColorImage(vtkImageData* image, unsigned int xDim = 20, unsigned int yDim = 20)
{
    vtkNew<vtkNamedColors> colors;

    std::array<unsigned char, 3> drawColor1{ 0, 0, 0 };
    //std::array<unsigned char, 3> drawColor2{ 0, 0, 0 };
    auto color1 = colors->GetColor3ub("HotPink").GetData();
    //auto color2 = colors->GetColor3ub("Chartreuse").GetData();
    for (auto i = 0; i < 3; ++i)
    {
        drawColor1[i] = color1[i];
        //drawColor2[i] = color2[i];
    }
   

    image->SetDimensions(xDim, yDim, 1);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

    for (unsigned int x = 0; x < xDim; x++)
    {
        for (unsigned int y = 0; y < yDim; y++)
        {
            auto pixel =
                static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
            if (x < xDim / 2)
            {
                for (auto i = 0; i < 3; ++i)
                {
                    pixel[i] = drawColor1[i];
                }
            }
        }
    }
    image->Modified();
}


vtkIdType PeakAndBscanVTKView::paintPointNumber(double xin, double yin, double zin,
    vtkRectilinearGrid* grid)
{
    int b[3];          // аналог b = [0,0,0]
    double c[3];   // аналог c = [0.0,0.0,0.0]

    double point[3] = { xin, yin, zin };

    int a = grid->ComputeStructuredCoordinates(point, b, c);
    if (a > 0)
    {
        double pcoord[3];
        grid->GetPoint(b[0], b[1], b[2], pcoord);
        return grid->ComputePointId(b);
    }

    return -1; // аналог None
}

// Основная функция
vtkDoubleArray* PeakAndBscanVTKView::paintCircle(vtkRectilinearGrid* grid,
    vtkDoubleArray* scalars,
    double r, double val,
    double dx, double dy,
    double dz)
{
    int dims[3];
    grid->GetDimensions(dims);

    double xSize = dims[0] * dx;
    double ySize = dims[1] * dy;

    double mindXY = dx;
    if (dy < dx)
        mindXY = dy;

    double x = 2 * r + (std::rand() % 101) * (xSize - 2 * r) / 100.0;
    double y = 2 * r + (std::rand() % 101) * (ySize - 2 * r) / 100.0;
    double z = 0.0;

    int counter = static_cast<int>(r / mindXY);

    for (int i = 0; i < counter; i++)
    {
        double yl = i * mindXY;

        for (int j = 0; j < counter; j++)
        {
            double xl = j * mindXY;

            double dist = std::sqrt(xl * xl + yl * yl);

            if (dist <= r)
            {
                vtkIdType idp;

                // 4 квадранта
                idp = paintPointNumber(x + xl, y + yl, z, grid);
                if (idp != -1)
                    scalars->SetValue(idp, val);

                idp = paintPointNumber(x + xl, y - yl, z, grid);
                if (idp != -1)
                    scalars->SetValue(idp, val);

                idp = paintPointNumber(x - xl, y + yl, z, grid);
                if (idp != -1)
                    scalars->SetValue(idp, val);

                idp = paintPointNumber(x - xl, y - yl, z, grid);
                if (idp != -1)
                    scalars->SetValue(idp, val);
            }
        }
    }

    return scalars;
}

vtkDoubleArray* PeakAndBscanVTKView::paintCirclePure(vtkRectilinearGrid* grid, vtkDoubleArray* scalars, double r,
    double val, double dx, double dy, double dz)
{
    int dims[3];
    grid->GetDimensions(dims);

    double xSize = dims[0] * dx;
    double ySize = dims[1] * dy;

    double x = 2 * r + (std::rand() % 101) * (xSize - 2 * r) / 100.0;
    double y = 2 * r + (std::rand() % 101) * (ySize - 2 * r) / 100.0;
    double z = 0.0;

    // Список точек (аналог Python list)
    std::vector<std::pair<double, double>> px = {
        {x - 4 * r / 5,y},{x - 3 * r / 5,y},{x - 2 * r / 5,y},{x - 1 * r / 5,y},{x,y},{x - 1 * r / 5,y},{x + 2 * r / 5,y},{x + 3 * r / 5,y},{x + 4 * r / 5,y},

        {x - 3 * r / 5,y + 1 * r / 5},{x - 2 * r / 5,y + 1 * r / 5},{x - 2 * r / 5,y + 1 * r / 5},{x,y + 1 * r / 5},
        {x - 2 * r / 5,y + 1 * r / 5},{x + 2 * r / 5,y + 1 * r / 5},{x + 3 * r / 5,y + 1 * r / 5},

        {x - 3 * r / 5,y + 2 * r / 5},{x - 2 * r / 5,y + 2 * r / 5},{x - 1 * r / 5,y + 2 * r / 5},{x,y + 2 * r / 5},
        {x + 1 * r / 5,y + 2 * r / 5},{x + 2 * r / 5,y + 2 * r / 5},{x + 3 * r / 5,y + 2 * r / 5},

        {x - 3 * r / 5,y + 3 * r / 5},{x - 2 * r / 5,y + 3 * r / 5},{x - 1 * r / 5,y + 3 * r / 5},{x,y + 3 * r / 5},
        {x + 1 * r / 5,y + 3 * r / 5},{x + 2 * r / 5,y + 3 * r / 5},{x + 3 * r / 5,y + 3 * r / 5},

        {x - 3 * r / 5,y - 1 * r / 5},{x - 2 * r / 5,y - 1 * r / 5},{x - 2 * r / 5,y - 1 * r / 5},{x,y - 1 * r / 5},
        {x - 2 * r / 5,y - 1 * r / 5},{x + 2 * r / 5,y - 1 * r / 5},{x + 3 * r / 5,y - 1 * r / 5},

        {x - 3 * r / 5,y - 2 * r / 5},{x - 2 * r / 5,y - 2 * r / 5},{x - 1 * r / 5,y - 2 * r / 5},{x,y - 2 * r / 5},
        {x + 1 * r / 5,y - 2 * r / 5},{x + 2 * r / 5,y - 2 * r / 5},{x + 3 * r / 5,y - 2 * r / 5},

        {x - 3 * r / 5,y - 3 * r / 5},{x - 2 * r / 5,y - 3 * r / 5},{x - 1 * r / 5,y - 3 * r / 5},{x,y - 3 * r / 5},
        {x + 1 * r / 5,y - 3 * r / 5},{x + 2 * r / 5,y - 3 * r / 5},{x + 3 * r / 5,y - 3 * r / 5}
    };

    for (const auto& p : px)
    {
        int ijk[3];
        double pcoords[3];

        double point[3] = { p.first, p.second, z };

        int inside = grid->ComputeStructuredCoordinates(point, ijk, pcoords);

        if (inside)
        {
            double worldPoint[3];
            grid->GetPoint(grid->ComputePointId(ijk), worldPoint);

            double dist = std::sqrt(
                (worldPoint[0] - x) * (worldPoint[0] - x) +
                (worldPoint[1] - y) * (worldPoint[1] - y)
            );

            if (dist <= r)
            {
                vtkIdType idp = grid->ComputePointId(ijk);
                scalars->SetValue(idp, val);
            }
        }
    }

    return scalars;
}

void PeakAndBscanVTKView::constructPeakAndBscanVTKView()
{
    //showNView(scan, 3);
    return; 
    
    //polyDataSource->SetOutput(sphere);
    //mapper->SetInputConnection(polyDataSource->GetOutputPort());
    //mapper->SetScalarModeToUsePointData();
    //mapper->ColorByArrayComponent("range", 0);
    //mapper->Update();

    //actor->SetMapper(mapper);
    //actor->GetProperty()->SetEdgeVisibility(true);
    //actor->GetProperty()->SetRepresentationToSurface();

    //renderer->AddActor(actor);

    //quantizeFilter->SetInputConnection(polyDataSource->GetOutputPort());
    //quantizeFilter->SetQFactor(.1);
    //quantizeFilter->Update();

    //mapperQuantized->SetInputConnection(quantizeFilter->GetOutputPort());
    //mapperQuantized->SetScalarModeToUsePointData();
    //mapperQuantized->ColorByArrayComponent("range", 0);
    //mapperQuantized->Update();

    //actorQuantized->SetMapper(mapperQuantized);
    //actorQuantized->GetProperty()->SetEdgeVisibility(true);
    //actorQuantized->GetProperty()->SetRepresentationToSurface();

    //renderer->AddActor(actorQuantized);

    //genericOpenGLRenderWindow->AddRenderer(renderer);

    ////QPointer<QVTKOpenGLNativeWidget> widget = new QVTKOpenGLNativeWidget(...);
    //this->setRenderWindow(genericOpenGLRenderWindow.Get());
    renderWindowInteractor->SetRenderWindow(genericOpenGLRenderWindow);
    //renderWindowInteractor = genericOpenGLRenderWindow->GetInteractor();
    //renderWindowInteractor = renderWindow->GetInteractor();
    //renderWindowInteractor->SetRenderWindow(genericOpenGLRenderWindow);
    vtkNew<MyStyle2> style;
    renderWindowInteractor->SetInteractorStyle(style);
    //interactor->SetInteractorStyle(style);
    style->SetDefaultRenderer(renderer);
    style->SetCurrentRenderer(renderer);
    //renderWindowInteractor->Start();

    renderWindow->SetSize(640, 360);
    renderWindow->SetWindowName("QuantizePolyDataPoints");
    //vtkNew<MyStyle2> interactorStyle;
    ////interactorStyle->SetDefaultRenderer(renderer);
    ////interactorStyle->SetCurrentRenderer(renderer);
    //interactor->SetInteractorStyle(interactorStyle);
    //interactor->SetRenderWindow(renderWindow);

    tableAll->AddColumn(arrXAll);
    tableAll->AddColumn(arrYAll);
    arrXAll->SetName("X Axis");
    arrYAll->SetName("Y Ax");

    tablePeak->AddColumn(arrX);
    tablePeak->AddColumn(arrP);
    arrX->SetName("X Axis");
    arrP->SetName("Peak");

    // Set up a 2D scene, add an XY chart to it.
    view->GetRenderWindow()->SetSize(1280, 1024);
    view->GetRenderWindow()->SetWindowName("PeaksView");

    view->GetScene()->AddItem(matrix);
    matrix->SetSize(vtkVector2i(1, 2));
    matrix->SetGutter(vtkVector2f(50.0, 70.0));



    int numPoints =2;
    //float inc = 1;
    float inc = 7.5 / (numPoints - 1);
    tablePeak->SetNumberOfRows(numPoints);
    tablePeak->SetValue(0, 0, 0);
    tablePeak->SetValue(0, 1, 0);
    tablePeak->SetValue(1, 0, 1);
    tablePeak->SetValue(1, 1, 1);
    auto numOfAllRows = 0;
    tableAll->SetNumberOfRows(numOfAllRows + numPoints);
    vtkChart* chart;
    vtkPlot*  plot;

    chart = matrix->GetChart(vtkVector2i(0, 0));
    plot = chart->AddPlot(vtkChart::LINE);
    //plot = chart->AddPlot(vtkChart::POINTS);
    plot->SetInputData(tableAll, 0, 1);
#if VTK_HAS_SETCOLORF
    plot->GetXAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("white").GetData());
    plot->GetYAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("white").GetData());
#else
    plot->GetXAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
    plot->GetYAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
#endif
    plot->SetColor(colors->GetColor3ub("dark_orange").GetRed(),
        colors->GetColor3ub("dark_orange").GetGreen(),
        colors->GetColor3ub("dark_orange").GetBlue(), 255);

    //plot = chart->AddPlot(vtkChart::LINE);
    plot = chart->AddPlot(vtkChart::POINTS);
    plot->SetInputData(tablePeak, 0, 1);
#if VTK_HAS_SETCOLORF
    plot->GetXAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("warm_grey").GetData());
    plot->GetYAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("warm_grey").GetData());
#else
    plot->GetXAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
    plot->GetYAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
#endif
    plot->SetColor(colors->GetColor3ub("royal_blue").GetRed(),
        colors->GetColor3ub("royal_blue").GetGreen(),
        colors->GetColor3ub("royal_blue").GetBlue(), 255);



    chart = matrix->GetChart(vtkVector2i(0, 1));
    //chart->GetPlot(0);
    //chart->ClearPlots();
    plot = chart->AddPlot(vtkChart::BAR);
    plot->SetInputData(tablePeak, 0, 1);
#if VTK_HAS_SETCOLORF
    plot->GetXAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("warm_grey").GetData());
    plot->GetYAxis()->GetGridPen()->SetColorF(
        colors->GetColor3d("warm_grey").GetData());
#else
    plot->GetXAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
    plot->GetYAxis()->GetGridPen()->SetColor(
        colors->GetColor3d("warm_grey").GetData());
#endif
    //plot->SetColor(colors->GetColor3ub("rose_madder").GetRed(),
    plot->SetColor(colors->GetColor3ub("dark_blue").GetRed(),
                   colors->GetColor3ub("dark_blue").GetGreen(),
                   colors->GetColor3ub("dark_blue").GetBlue(), 255);
    // Finally render the scene and compare the image to a reference image
    view->GetRenderer()->SetBackground(
        colors->GetColor3d("navajo_white").GetData());
    //view->GetRenderWindow()->Render();
    view->GetInteractor()->Initialize();
    //view->GetInteractor()->Start();

    //vtkNew<vtkGenericOpenGLRenderWindow> genericOpenGLRenderWindow;
#if VTK890
    this->setRenderWindow(genericOpenGLRenderWindow);
#else
    this->SetRenderWindow(renderWindow);
#endif

    vtkNew<vtkEventQtSlotConnect> slotConnector;
    this->Connections = slotConnector;

//#if VTK890
//    this->Connections->Connect(
//        this->genericOpenGLRenderWindow->GetInteractor(),
//        vtkCommand::LeftButtonPressEvent, this,
//        SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
//#else
//    this->Connections->Connect(
//        this->qvtkWidget->GetRenderWindow()->GetInteractor(),
//        vtkCommand::LeftButtonPressEvent, this,
//        SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
//#endif
    genericOpenGLRenderWindow->Render();
    genericOpenGLRenderWindow->Render();
    genericOpenGLRenderWindow->GetInteractor()->Initialize();
}

PeakAndBscanVTKView::PeakAndBscanVTKView(QWidget* parentIn)
{
    QVTKOpenGLNativeWidget();
    constructPeakAndBscanVTKView();
    //showQuantizedPoints(scan);
    ////PeakAndBscanVTKView(scan, parent);
    //buttonWidgetXY = vtkSmartPointer<vtkButtonWidget2D_2>::New();
}

PeakAndBscanVTKView::~PeakAndBscanVTKView()
{

}

PeakAndBscanVTKView::PeakAndBscanVTKView(std::shared_ptr<Scan> scan, QWidget* parent)
{
    QVTKOpenGLNativeWidget();
    constructPeakAndBscanVTKView();
    //showQuantizedPoints(scan);
}

vtkNew<vtkDiscretizableColorTransferFunction> PeakAndBscanVTKView::buildCTF(bool const& raduga, std::vector<ColorStop> colors)
{
    vtkNew<vtkDiscretizableColorTransferFunction> ctf;

    ctf->SetColorSpaceToRGB();
    ctf->SetScaleToLinear();
    ctf->SetNanColor(0.5, 0.5, 0.5);
    //ctf->SetBelowRangeColor(0.0, 0.0, 0.0);

    if (colors.size() > 0) {
        int ans = std::stoi(colors[0].color.substr(1), 0, 16);
        double r = ((ans >> 16) & 0xff) / 255;
        double b = (ans & 0xff) / 255;
        double g = ((ans >> 8) & 0xff) / 255;
        ctf->SetAboveRangeColor(r, g, b);
        ans = std::stoi(colors.at(colors.size() - 1).color.substr(1), 0, 16);
        r = ((ans >> 16) & 0xff) / 255;
        b = (ans & 0xff) / 255;
        g = ((ans >> 8) & 0xff) / 255;
        ctf->SetBelowRangeColor(r, g, b);
    }
    else {

        ctf->SetAboveRangeColor(1.0, 1.0, 1.0);
        ctf->SetBelowRangeColor(0.0, 0.0, 0.0);
    }
    ctf->UseAboveRangeColorOn();
    ctf->UseBelowRangeColorOn();

    if (raduga)
    {
        ctf->AddRGBPoint(-1.0, 1.0, 0.0, 0.0);                 // Red
        ctf->AddRGBPoint(-2.0 / 3.0, 1.0, 128.0 / 255.0, 0.0); // Orange #ff8000
        ctf->AddRGBPoint(-1.0 / 3.0, 1.0, 1.0, 0.0);           // Yellow
        ctf->AddRGBPoint(0.0, 0.0, 1.0, 0.0);                  // Green  #00ff00
        ctf->AddRGBPoint(1.0 / 3.0, 0.0, 1.0, 1.0);            // Cyan
        ctf->AddRGBPoint(2.0 / 3.0, 0.0, 0.0, 1.0);            // Blue
        ctf->AddRGBPoint(1.0, 128.0 / 255.0, 0.0, 1.0);        // Violet #8000ff
        ctf->SetNumberOfValues(7);
    }
    else {
        for (auto colorPair : colors) {
            int ans = stoi(colorPair.color.substr(1), 0, 16);
            auto r = (ans >> 16) & 0xff;
            auto b = ans & 0xff;
            auto g = (ans >> 8) & 0xff;
            //int i = c.toUInt();
            //unsigned int x;
            //std::stringstream ss;
            //ss << std::hex << colorPair.color.substr(1);
            //ss >> x;
            //// output it as a signed type
            //std::cout << static_cast<int>(x) << std::endl;
            //auto r = (x >> 16) & 0xff;
            //auto b = x & 0xff;
            //auto g = (x>>8) & 0xff;
            ctf->AddRGBPoint(colorPair.val, r / 255.0, g / 255.0, b / 255.0);
        }
        ctf->SetNumberOfValues(colors.size());
    }
    ctf->DiscretizeOff();
    return ctf;
}

void PeakAndBscanVTKView::SetPolyDataSourceFromParameters(vtkNew<vtkPolyData>& pointSource, double startCoordinate, double finalCoordinate, ::std::vector< double > lineCoordinates, boost::multi_array<::RgbColor, 2> view)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("range");
    {
        auto xSize = view.shape()[0];
        auto ySize = view.shape()[1];
        for (size_t i = 0; i < ySize; i++) {
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
            for (size_t j = 0; j < xSize; j++) {
                auto stepX = (finalCoordinate - startCoordinate) / xSize;
                auto x = startCoordinate+ stepX *j;
                auto y = lineCoordinates[i];
                auto idOfPoint = points->InsertNextPoint(x, y, 0);
                polyLine->GetPointIds()->InsertNextId(idOfPoint);
                scalars->InsertNextTuple1(view[j][i].red*16*16*16*16 + view[j][i].green * 16 * 16+ view[j][i].blue);
            }
            cells->InsertNextCell(polyLine);
        }
    }
    pointSource->SetPoints(points);
    pointSource->SetLines(cells);
    pointSource->GetPointData()->SetScalars(scalars);
    pointSource->Modified();

    polyDataSource->SetOutput(pointSource);
    polyDataSource->Update();
}

void PeakAndBscanVTKView::SetPolyDataSource(int index, vtkNew<vtkPolyData> &pointSource, ::std::vector< ::NormalizedRange > *normalizedRange)
{
    //vtkNew<vtkPolyData> pointSource;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("range");
    ////auto nRanges = scan->normalizedSpec.begin()->size();
    //for (int nRange = 0; nRange < nRanges; nRange++) {
    int nRange = index;
    {
        ////auto normalizedRanges = scan->normalizedRanges[index];
        //auto normalizedRanges = scan->commonNormalizedRanges[index];
        auto normalizedRanges = normalizedRange->at(index);
        auto view = normalizedRanges.view;
        auto xSize = view.shape()[0];
        auto ySize = view.shape()[1];
        auto kx = (3.14 / 50.0);
        auto ky = (3.14 / 50.0);
        auto shx = 70.0;
        auto shy = 30.0;
        auto Az = 10.0;
        for (size_t i = 0; i < ySize; i++) {
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
            for (size_t j = 0; j < xSize; j++) {
                //auto pointValue = view[i][j];
                //auto peak = line[nRange].peaks[i];
                auto stepX = (normalizedRanges.finalCoordinate - normalizedRanges.startCoordinate) / xSize;
                normalizedRanges.step;
                auto x = normalizedRanges.startCoordinate+ stepX *j;
                //auto y = normalizedRanges.finalLineCoordinate - normalizedRanges.lineCoordinates[i];
                auto y = normalizedRanges.lineCoordinates[i];
                auto z = 0;
                if (scan && scan->parameters.addZCoordinates) {
                    auto xx = kx * (x - shx );
                    auto yy = ky * (y - shy);
                    auto t = (xx * xx + yy * yy);
                    //z = 20*(std::sin(t)* std::cos(t));
                    z = Az*(std::sin(xx)* std::cos(yy));
                }
                auto idOfPoint = points->InsertNextPoint(x, y, z);
                polyLine->GetPointIds()->InsertNextId(idOfPoint);
                scalars->InsertNextTuple1(view[j][i]);
                //scalars->InsertNextTuple1(peak.y);
            }
            cells->InsertNextCell(polyLine);
        }
        //stageProgressed();
    }
    
    pointSource->SetPoints(points);
    pointSource->SetLines(cells);
    pointSource->GetPointData()->SetScalars(scalars);
    pointSource->Modified();
    polyDataSource->SetOutput(pointSource);
    polyDataSource->Update();
}

/// <summary>
/// отображение удара в 3d сцене
/// </summary>
/// <param name="peak"></param>
/// <param name="scan"></param>
void PeakAndBscanVTKView::ShowPeak(Peak& peak, std::shared_ptr<Scan>& scan)
{
    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkRenderWindow> renwin;
    renwin->SetMultiSamples(4);
    renwin->SetSize(640, 480);
    renwin->SetWindowName("ChartsOn3DScene");
    vtkNew<vtkRenderWindowInteractor> iren;
    iren->SetRenderWindow(renderWindow);
    vtkRenderer* renderer = vtkRenderer::New();
    
    //vtkRenderer* renderer = genericOpenGLRenderWindow->GetRenderers()->GetFirstRenderer() ;
    if (!renderer) return;
    renderer->SetBackground(colors->GetColor4d("seagreen").GetData());
    
    renwin->AddRenderer(renderer);
    renderer->ResetCamera();
    renderer->GetActiveCamera()->SetPosition(1.0, 1.0, -4.0);
    renderer->GetActiveCamera()->Azimuth(40);
    renderer->ResetCameraClippingRange();

    // Now the chart,
    vtkNew<vtkChartXY> chart;
    vtkNew<vtkContextScene> chartScene;
    vtkNew<vtkContextActor> chartActor;

    chart->SetAutoSize(false);
    chart->SetSize(vtkRectf(0.0, 0.0, 320, 220));

    chartScene->AddItem(chart);
    chartActor->SetScene(chartScene);

    // Both needed.
    renderer->AddActor(chartActor);
    chartScene->SetRenderer(renderer);

    // Create a table with some points in it.
    vtkNew<vtkTable> table;

    vtkNew<vtkFloatArray> arrX;
    arrX->SetName("X Axis");
    table->AddColumn(arrX);

    vtkNew<vtkFloatArray> arrC;
    arrC->SetName("Cosine");
    table->AddColumn(arrC);

    vtkNew<vtkFloatArray> arrS;
    arrS->SetName("Sine");
    table->AddColumn(arrS);

    vtkNew<vtkFloatArray> arrT;
    arrT->SetName("Tan");
    table->AddColumn(arrT);

    // Test charting with a few more points...
    //int numPoints = 69;
    //float inc = 7.5 / (numPoints - 1.0);
    //table->SetNumberOfRows(numPoints);
    //for (int i = 0; i < numPoints; ++i)
    //{
    //    table->SetValue(i, 0, i * inc);
    //    table->SetValue(i, 1, cos(i * inc) + 0.0);
    //    table->SetValue(i, 2, sin(i * inc) + 0.0);
    //    table->SetValue(i, 3, tan(i * inc) + 0.5);
    //}
    int numPoints = peak.endIndex - peak.beginIndex;
    float inc = 1;
    table->SetNumberOfRows(numPoints);
    for (int i = 0; i < numPoints; ++i)
    {
        auto ind = peak.beginIndex + i * inc;
        table->SetValue(i, 0, ind);
        table->SetValue(i, 1, scan->scanArm.sound.samples.at(ind));
    }


    // Add multiple line plots, setting the colors etc.
    vtkColor3d color3d = colors->GetColor3d("banana");

    vtkPlot* points = chart->AddPlot(vtkChart::POINTS);
#if VTK_HAS_SETCOLORF
    points->SetInputData(table, 0, 1);
    points->SetColorF(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
    dynamic_cast<vtkPlotPoints*>(points)->SetMarkerStyle(vtkPlotPoints::CROSS);
    points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 0, 2);
    points->SetColorF(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
    dynamic_cast<vtkPlotPoints*>(points)->SetMarkerStyle(vtkPlotPoints::PLUS);
    points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 0, 3);
    points->SetColorF(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
#else
    points->SetInputData(table, 0, 1);
    points->SetColor(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
    dynamic_cast<vtkPlotPoints*>(points)->SetMarkerStyle(vtkPlotPoints::CROSS);
    points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 0, 2);
    points->SetColor(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
    dynamic_cast<vtkPlotPoints*>(points)->SetMarkerStyle(vtkPlotPoints::PLUS);
    points = chart->AddPlot(vtkChart::POINTS);
    points->SetInputData(table, 0, 3);
    points->SetColor(color3d.GetRed(), color3d.GetGreen(), color3d.GetBlue());
    points->SetWidth(1.0);
#endif
    renwin->SetMultiSamples(0);
    renwin->Render();
    //genericOpenGLRenderWindow->SetMultiSamples(0);
    //genericOpenGLRenderWindow->Render();
    iren->Initialize();
    iren->Start();
}

/// <summary>
/// отдельное окно с отображением удара (peak) 
/// </summary>
/// <param name="peak"></param>
/// <param name="scan"></param>
void PeakAndBscanVTKView::ChartPeak(Peak& peak, std::shared_ptr<Scan>& scan)
{
    //vtkNew<vtkNamedColors> colors;
    vtkNew<vtkTable> table;
    vtkNew<vtkFloatArray> arrX;
    arrX->SetName("X Axis");
    table->AddColumn(arrX);
    vtkNew<vtkFloatArray> arrP;
    arrP->SetName("Peak");
    table->AddColumn(arrP);
    //// Set up a 2D scene, add an XY chart to it.
    //view->GetRenderWindow()->SetSize(1280, 1024);
    //view->GetRenderWindow()->SetWindowName("PeaksView");
    //view->GetScene()->AddItem(matrix);
    //matrix->SetSize(vtkVector2i(1, 2));
    //matrix->SetGutter(vtkVector2f(50.0, 70.0));

    int numPoints = peak.endIndex - peak.beginIndex;

    float inc = 1;
    //float inc = 7.5 / (numPoints - 1);

    //tablePeak->RemoveAllRows();
    //tablePeak->Modified();
    tablePeak->SetNumberOfRows(numPoints);
    //tablePeak->Modified();
    //table->SetNumberOfRows(numPoints);
    auto numOfAllRows = tableAll->GetNumberOfRows();
    tableAll->SetNumberOfRows(numOfAllRows + numPoints);

    for (int i = 0; i < numPoints; ++i)
    {
        //table->SetValue(i, 0, peak.beginIndex + i * inc);
        //table->SetValue(i, 1, scan->scanArm.sound.samples.at(peak.beginIndex + i * inc));
        tablePeak->SetValue(i, 0, peak.beginIndex + i * inc);
        tablePeak->SetValue(i, 1, scan->scanArm.sound.samples.at(peak.beginIndex + i * inc));
        tableAll->SetValue(numOfAllRows + i, 0, peak.beginIndex + i * inc);
        tableAll->SetValue(numOfAllRows + i, 1, scan->scanArm.sound.samples.at(peak.beginIndex + i * inc));
    }
    tablePeak->Modified();
    //table->Modified();
    tableAll->Modified();
    vtkChart* chart;
    vtkPlot* plot;

    chart = matrix->GetChart(vtkVector2i(0, 0));
    chart->ClearPlots();
    //plot = chart->GetPlot(0);
    plot = chart->AddPlot(vtkChart::LINE);
    plot->SetInputData(tableAll, 0, 1);
    plot->Update();

    chart = matrix->GetChart(vtkVector2i(0, 1));
    //plot = chart->GetPlot(0);
    chart->ClearPlots();
    plot = chart->AddPlot(vtkChart::POINTS);
    plot->SetInputData(tablePeak,0,1);
    plot->Update();

//    vtkChart* chart;
//    vtkPlot* plot ;
//     // Add multiple line plots, setting the colors etc
//    // Lower left plot, a point chart.
//    chart = matrix->GetChart(vtkVector2i(0, 1));
//    chart->ClearPlots();
//    plot = chart->AddPlot(vtkChart::POINTS);
//    plot->SetInputData(table, 0, 1);
//    dynamic_cast<vtkPlotPoints*>(plot)->SetMarkerStyle(vtkPlotPoints::DIAMOND);
//#if VTK_HAS_SETCOLORF
//    plot->GetXAxis()->GetGridPen()->SetColorF(
//        //colors->GetColor3d("warm_grey").GetData());
//        colors->GetColor3d("red").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("red").GetData());
//#else
//    plot->GetXAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//#endif
//    plot->SetColor(colors->GetColor3ub("sea_green").GetRed(),
//        colors->GetColor3ub("sea_green").GetGreen(),
//        colors->GetColor3ub("sea_green").GetBlue(), 255);
////
//    // Upper left plot, a point chart.
//    chart = matrix->GetChart(vtkVector2i(0, 1));
//    chart->GetPlot(0);
//    //chart->ClearPlots();
//    //plot = chart->AddPlot(vtkChart::BAR);
//    plot->SetInputData(table, 0, 1);
//#if VTK_HAS_SETCOLORF
//    plot->GetXAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("warm_grey").GetData());
//#else
//    plot->GetXAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//#endif
//    //plot->SetColor(colors->GetColor3ub("rose_madder").GetRed(),
//    plot->SetColor(colors->GetColor3ub("dark_blue").GetRed(),
//        colors->GetColor3ub("dark_blue").GetGreen(),
//        colors->GetColor3ub("dark_blue").GetBlue(), 255);
//
//    // Upper right plot, a bar and point chart.
////    chart = matrix->GetChart(vtkVector2i(1, 1));
////    plot = chart->AddPlot(vtkChart::BAR);
////    plot->SetInputData(table, 0, 1);
////#if VTK_HAS_SETCOLORF
////    plot->GetXAxis()->GetGridPen()->SetColorF(
////        colors->GetColor3d("green").GetData());
////    plot->GetYAxis()->GetGridPen()->SetColorF(
////        colors->GetColor3d("green").GetData());
////#else
////    plot->GetXAxis()->GetGridPen()->SetColor(
////        colors->GetColor3d("warm_grey").GetData());
////    plot->GetYAxis()->GetGridPen()->SetColor(
////        colors->GetColor3d("warm_grey").GetData());
////#endif
////    plot->SetColor(colors->GetColor3ub("burnt_sienna").GetRed(),
////        colors->GetColor3ub("burnt_sienna").GetGreen(),
////        colors->GetColor3ub("burnt_sienna").GetBlue(), 255);
////
////    plot = chart->AddPlot(vtkChart::POINTS);
////    plot->SetInputData(table, 0, 1);
////    dynamic_cast<vtkPlotPoints*>(plot)->SetMarkerStyle(vtkPlotPoints::CROSS);
////#if VTK_HAS_SETCOLORF
////    plot->GetXAxis()->GetGridPen()->SetColorF(
////        colors->GetColor3d("yellow").GetData());
////    plot->GetYAxis()->GetGridPen()->SetColorF(
////        colors->GetColor3d("yellow").GetData());
////#else
////    plot->GetXAxis()->GetGridPen()->SetColor(
////        colors->GetColor3d("warm_grey").GetData());
////    plot->GetYAxis()->GetGridPen()->SetColor(
////        colors->GetColor3d("warm_grey").GetData());
////#endif
////    plot->SetColor(colors->GetColor3ub("rose_madder").GetRed(),
////        colors->GetColor3ub("rose_madder").GetGreen(),
////        colors->GetColor3ub("rose_madder").GetBlue(), 255);
//
//    // Lower right plot, two line charts.
//    chart = matrix->GetChart(vtkVector2i(0, 0));
//    plot = chart->AddPlot(vtkChart::LINE);
//    plot->SetInputData(tableAll, 0, 1);
//#if VTK_HAS_SETCOLORF
//    plot->GetXAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("white").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("white").GetData());
//#else
//    plot->GetXAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//#endif
//    plot->SetColor(colors->GetColor3ub("dark_orange").GetRed(),
//        colors->GetColor3ub("dark_orange").GetGreen(),
//        colors->GetColor3ub("dark_orange").GetBlue(), 255);
//
//    plot = chart->AddPlot(vtkChart::LINE);
//    plot->SetInputData(table, 0, 1);
//#if VTK_HAS_SETCOLORF
//    plot->GetXAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColorF(
//        colors->GetColor3d("warm_grey").GetData());
//#else
//    plot->GetXAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//    plot->GetYAxis()->GetGridPen()->SetColor(
//        colors->GetColor3d("warm_grey").GetData());
//#endif
//    plot->SetColor(colors->GetColor3ub("royal_blue").GetRed(),
//        colors->GetColor3ub("royal_blue").GetGreen(),
//        colors->GetColor3ub("royal_blue").GetBlue(), 255);
//
    // Finally render the scene and compare the image to a reference image
    //view->GetRenderer()->SetBackground(
    //    colors->GetColor3d("navajo_white").GetData());
    //view->GetRenderWindow()->Render();
    //view->GetInteractor()->Initialize();
    //view->GetInteractor()->Start();
    //view->GetRenderer()->ResetCameraClippingRange();
    view->GetRenderer()->ResetCamera();
    //view->GetRenderer()->Render();
    view->GetRenderWindow()->Render();
    view->GetInteractor()->Initialize();

}

void PeakAndBscanVTKView::showQuantizedPoints(std::shared_ptr<Scan> scan)
{

    auto scan_use_count = scan.use_count();
    if (scan_use_count < 1) return;
    this->scan = scan;
    vtkNew<vtkPolyData> peakRtPolyData;

    ///******* готовим точки пиков
    int numOfPoints = 0;
    double t = 0;
    double x, y, z;
    double deltaZ = -15;
    double shiftDeltaZ =0;

    vtkSmartPointer<vtkPoints> peakRtpoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> peakRtcells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkFloatArray> peakValueArr = vtkSmartPointer<vtkFloatArray>::New();
    peakValueArr->SetName("range");
    vtkSmartPointer<vtkIntArray> peakCountArr = vtkSmartPointer<vtkIntArray>::New();
    peakCountArr->SetName("countPeak");
    boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::min>> akumRt;
    {
        auto currentNumAreaLocal = scan->scanArm.parameters.headAndScanCollectorParameters.currentNumArea;
        //for (auto& line : scan->scanArm.rtPeaks) {
        for (int lineNum = 0; lineNum < scan->scanArm.rtPeaks.size(); lineNum += currentNumAreaLocal) {
            auto line = scan->scanArm.rtPeaks.at(lineNum);
            vtkSmartPointer<vtkPolyLine> peakRtPolyLine = vtkSmartPointer<vtkPolyLine>::New();
            for (int j = 0; j < line.size(); j += currentNumAreaLocal) {
                auto peaks = line.at(j);
                int countLocal = 0;
                for (auto const peak : peaks) {
                    auto idOfPoint = peakRtpoints->InsertNextPoint(peak.x, scan->scanArm.parameters.headAndScanCollectorParameters.height - peak.y,
                        peak.z + shiftDeltaZ * countLocal++);
                    countLocal++;
                    numOfPoints++;
                    peakRtPolyLine->GetPointIds()->InsertNextId(idOfPoint);
                    auto averIndexOfPeak = (peak.endIndex + peak.beginIndex) / 2;
                    //auto beginValue = scan->scanArm.sound.samples.at(peak.beginIndex);
                    //auto endValue = scan->scanArm.sound.samples.at(peak.endIndex);
                    //auto averValue = scan->scanArm.sound.samples.at(averIndexOfPeak);
                    //peakValueArr->InsertNextTuple1(averValue);
                    //akumRt(averValue);
                    peakValueArr->InsertNextTuple1(countLocal);
                    ///************* 26/11/2024
                                        //ChartPeak(peak, scan);
                                        //ShowPeak(peak, scan);
                    ///************* end 26/11/2024
                }
                //peakCountArr->InsertNextTuple1(countLocal);
                //peakCountArr->InsertNextTuple1(line.at(j).size());
            }
            peakRtcells->InsertNextCell(peakRtPolyLine);
        }
        //stageProgressed();
    }
    double minValueRt = boost::accumulators::min(akumRt);
    double maxValueRt = boost::accumulators::max(akumRt);

    vtkNew<vtkPolyData> trajectoryPolyData;
    if (scan->parameters.headAndScanCollectorParameters.needShowCountOfPeak) {
        peakRtPolyData->SetPoints(peakRtpoints);
        peakRtPolyData->SetLines(peakRtcells);
        peakRtPolyData->SetVerts(peakRtcells);
        //peakRtPolyData->GetPointData()->SetScalars(peakCountArr);
        peakRtPolyData->GetPointData()->SetScalars(peakValueArr);
        peakRtPolyData->Modified();
    }
    if (scan->parameters.headAndScanCollectorParameters.needShowTrajectory) {
        vtkSmartPointer<vtkPoints> pointsTrajectory = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> cellsTrajectory = vtkSmartPointer<vtkCellArray>::New();
        //vtkSmartPointer<vtkFloatArray> scalarsTrajectory = vtkSmartPointer<vtkFloatArray>::New();
        //scalarsTrajectory->SetName("Y");
        vtkSmartPointer<vtkPolyLine> trajectoryPolyLine = vtkSmartPointer<vtkPolyLine>::New();
        if (scan->scanArm.isScanArmReady) {
            for (auto elem : scan->scanArm.trajectory.pos)
            {

                auto idOfPoint = pointsTrajectory->InsertNextPoint(elem.x, scan->parameters.headAndScanCollectorParameters.height - elem.y, elem.z);
                trajectoryPolyLine->GetPointIds()->InsertNextId(idOfPoint);

            };
        }
        else {
            for (auto elem : scan->scanArm.trajectory.pos)
            {

                auto idOfPoint = pointsTrajectory->InsertNextPoint(elem.x, elem.y, elem.z);
                trajectoryPolyLine->GetPointIds()->InsertNextId(idOfPoint);

            };
        }
        cellsTrajectory->InsertNextCell(trajectoryPolyLine);
        trajectoryPolyData->SetPoints(pointsTrajectory);
        trajectoryPolyData->SetLines(cellsTrajectory);
        trajectoryPolyData->SetVerts(cellsTrajectory);
        trajectoryPolyData->Modified();
    }

    int indexOfCurrentNormalizedRange = 0;
    SetPolyDataSource(indexOfCurrentNormalizedRange, pointSource, &scan->normalizedRanges);

//    std::cout << "There are " << pointSource->GetNumberOfPoints() << " points."
//        << std::endl;
    ///******* end готовим точки пиков

    vtkNew<vtkQuantizePolyDataPoints> quantizeFilter;
    //quantizeFilter->SetInputConnection(pointSource->GetOutputPort());
    quantizeFilter->SetInputData(pointSource);
    quantizeFilter->SetQFactor(scan->parameters.headAndScanCollectorParameters.currentNumArea);
    //quantizeFilter->SetQFactor(.1);
    quantizeFilter->Update();

    vtkNew<vtkQuantizePolyDataPoints> quantizeFilterRt;
    quantizeFilterRt->SetInputData(peakRtPolyData);
    quantizeFilterRt->SetQFactor(scan->parameters.headAndScanCollectorParameters.currentNumArea);
    quantizeFilterRt->Update();

    vtkPolyData* quantized = quantizeFilter->GetOutput();
    //std::cout << "There are " << quantized->GetNumberOfPoints()
    //    << " quantized points." << std::endl;
    //for (vtkIdType i = 0; i < quantized->GetNumberOfPoints(); i++)
    //    quantized->GetPointData()->GetScalars()->GetTuple(i);
    //for (vtkIdType i = 0; i < pointSource->GetNumberOfPoints(); i++)
    //{
    //    double pOrig[3];
    //    double pQuantized[3];
    //    pointSource->GetPoint( i, pOrig);
    //    if (i < quantized->GetNumberOfPoints()) {
    //        quantized->GetPoints()->GetPoint(i, pQuantized);
    //        double scalar = quantized->GetPointData()->GetScalars()->GetTuple1(i);

    //        std::cout << "ish Point " << i << " : (" << pOrig[0] << ", " << pOrig[1] << ", "
    //            << pOrig[2] << ")" << " Quantized TO: (" << pQuantized[0] << ", "
    //            << pQuantized[1] << ", " << pQuantized[2] << ") have scalar: " << scalar << std::endl;
    //    }
    //    else {
    //        std::cout << "Point " << i << " : (" << pOrig[0] << ", " << pOrig[1] << ", "
    //            << pOrig[2] << ")" << " ( no point)" << std::endl;
    //    }
    //}

    double radiusX = 1.0;
    double radiusY = 1.0;

    //double radius = 0.02;
    auto normalizedRange = scan->normalizedRanges[indexOfCurrentNormalizedRange];
    //auto normalizedRangesXcount = scan->normalizedRanges[indexOfCurrentNormalizedRange].view.shape()[0];
    //auto normalizedRangesYcount = scan->normalizedRanges[indexOfCurrentNormalizedRange].view.shape()[1];
    auto normalizedRangesXcount = normalizedRange.view.shape()[0];
    auto normalizedRangesYcount = normalizedRange.view.shape()[1];
    //////double radius = scan->parameters.headAndScanCollectorParameters.currentNumArea / 2;
    ////double radiusX = (double)scan->parameters.headAndScanCollectorParameters.height / (normalizedRangesXcount);
    ////double radiusY = (double)scan->parameters.headAndScanCollectorParameters.width/ ( normalizedRangesYcount);
    if (scan->scanArm.isScanArmReady) {
        if (normalizedRangesXcount > 0) {
            radiusX = (double)scan->parameters.headAndScanCollectorParameters.height / (normalizedRangesXcount);
        }
        if (normalizedRangesYcount > 0) {
            radiusY = (double)scan->parameters.headAndScanCollectorParameters.width / (normalizedRangesYcount);
        }
    }
    else {
        double stepX = 1.0;
        double stepY = 1.0;
        if (normalizedRangesXcount > 0) {
            stepX = (normalizedRange.finalCoordinate - normalizedRange.startCoordinate) / normalizedRangesXcount;
            if (stepX == 0) {
                stepX = 1;
            }
        }
        if (normalizedRangesYcount > 0) {
            auto lineCoordinatesSize = normalizedRange.lineCoordinates.size();
            if (lineCoordinatesSize > 0) {
                double minY = normalizedRange.lineCoordinates[0];
                double maxY = normalizedRange.lineCoordinates[0];
                for (size_t i = 0; i < lineCoordinatesSize; i++) {
                    auto y = normalizedRange.lineCoordinates[i];
                    if (y > maxY) {
                        maxY = y;
                    }
                    if (y < minY) {
                        minY = y;
                    }
                }
                stepY = (maxY - minY) / normalizedRangesYcount;
                if (stepY == 0) {
                    stepY = 1;
                }

            }
        }
        radiusX = stepX;
        radiusY = stepY;
    }
    vtkNew<vtkSphereSource> sphereSource;
    double radiusZ = scan->parameters.headAndScanCollectorParameters.currentNumArea * scan->parameters.headAndScanCollectorParameters.koeffSphere;
    //double radiusZ = scan->parameters.headAndScanCollectorParameters.currentNumArea / 2;
    sphereSource->SetRadius(radiusZ);

    vtkNew<vtkParametricSuperEllipsoid> parametricSuperEllipsoidForInputMapper;
    parametricSuperEllipsoidForInputMapper->SetN1(0.2);
    parametricSuperEllipsoidForInputMapper->SetN2(0.2);
    parametricSuperEllipsoidForInputMapper->SetXRadius(radiusX*0.6);
    parametricSuperEllipsoidForInputMapper->SetYRadius(radiusY * 0.6);
    parametricSuperEllipsoidForInputMapper->SetZRadius(radiusY / 4);


    vtkSmartPointer<vtkParametricFunctionSource> parametricFunctionSourceForInputMapper = vtkSmartPointer<vtkParametricFunctionSource>::New();
    parametricFunctionSourceForInputMapper->SetParametricFunction(parametricSuperEllipsoidForInputMapper);
    parametricFunctionSourceForInputMapper->SetUResolution(11);
    parametricFunctionSourceForInputMapper->SetVResolution(11);
    parametricFunctionSourceForInputMapper->SetWResolution(11);
    parametricFunctionSourceForInputMapper->Update();

    vtkNew<vtkParametricSuperEllipsoid> parametricSuperEllipsoid;
    parametricSuperEllipsoid->SetN1(0.2);
    parametricSuperEllipsoid->SetN2(0.2);
    parametricSuperEllipsoid->SetXRadius(radiusZ);
    parametricSuperEllipsoid->SetYRadius(radiusZ);
    parametricSuperEllipsoid->SetZRadius(radiusZ / 4);

    vtkSmartPointer<vtkParametricFunctionSource> parametricFunctionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
    parametricFunctionSource->SetParametricFunction(parametricSuperEllipsoid);
    parametricFunctionSource->SetUResolution(11);
    parametricFunctionSource->SetVResolution(11);
    parametricFunctionSource->SetWResolution(11);
    parametricFunctionSource->Update();



    auto params = scan->parameters;

    //vtkNew<vtkLookupTable> lookupTable;
    //lookupTable->SetNumberOfTableValues(params.colorStopsList.size());
    //int i = 0;
    //double minValue;
    //double maxValue;

    //boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::max, boost::accumulators::tag::min>> akum;
    //for (auto colorPair : params.colorStopsList)
    //{
    //    akum(colorPair.val);
    //}
    //minValue = boost::accumulators::min(akum);
    //maxValue = boost::accumulators::max(akum);

    //for (auto colorPair : params.colorStopsList)
    //{
    //    int ans = stoi(colorPair.color.substr(1), 0, 16);
    //    auto r = (ans >> 16) & 0xff;
    //    auto b = ans & 0xff;
    //    auto g = (ans >> 8) & 0xff;

    //    lookupTable->SetTableValue(i, r/255.0,g / 255.0,b / 255.0,1);
    //    i++;
    //}
    //lookupTable->SetRampToLinear();
    //lookupTable->SetTableRange(minValue, maxValue);
    //lookupTable->Build();

    //inputMapper->SetInputData(pointSource);
    inputMapper->SetInputConnection(polyDataSource->GetOutputPort());
    //inputMapper->SetScalarRange(
    //    pointSource->GetPointData()->GetScalars()->GetRange()[0],
    //    pointSource->GetPointData()->GetScalars()->GetRange()[1]);

    auto fn = [](ColorStop  a, ColorStop b) { return a.val >= b.val; };
    ::std::vector< ::ColorStop > colorStopLocalVect;
    for (auto item : params.colorStopsList) {
        colorStopLocalVect.push_back(item);
    }
    
    sort(colorStopLocalVect.begin(), colorStopLocalVect.end(), fn);

    //inputMapper->SetLookupTable(buildCTF(false, params.colorStopsList));
    inputMapper->SetLookupTable(buildCTF(false, colorStopLocalVect));
    //inputMapper->SetSourceConnection(sphereSource->GetOutputPort());
    inputMapper->SetSourceConnection(parametricFunctionSourceForInputMapper->GetOutputPort());
    inputMapper->ScalarVisibilityOn();
    inputMapper->InterpolateScalarsBeforeMappingOff();
    //inputMapper->UseLookupTableScalarRangeOn();
    //inputMapper->SetScalarModeToUseCellData();
    //inputMapper->SetScalarModeToUsePointData();
    //inputMapper->MapScalars(0.4);
    inputMapper->ScalingOff();
    //inputMapper->SetLookupTable(lookupTable);
    inputMapper->Update();
    vtkNew<vtkActor> inputActor;
    inputActor->SetMapper(inputMapper);
    //inputActor->GetProperty()->SetColor(colors->GetColor3d("Orchid").GetData());
    bool isNeedShowBScan = scan->parameters.headAndScanCollectorParameters.needShowBScan;
    inputActor->SetVisibility(isNeedShowBScan);


    vtkNew<vtkScalarBarActor> scalarBar;
    scalarBar->SetLookupTable(inputMapper->GetLookupTable());

    auto tab = inputMapper->GetLookupTable();

    //scalarBar->SetLookupTable(lookupTable);
    scalarBar->SetTitle("Range");
    //scalarBar->SetNumberOfLabels(4);
    scalarBar->UnconstrainedFontSizeOn();
    scalarBar->DragableOn();
    scalarBar->DrawFrameOn();
    scalarBar->SetPosition(0.01, 0.01);
    scalarBar->DrawAboveRangeSwatchOn();
    scalarBar->DrawBelowRangeSwatchOn();

    scalarBar->Modified();

    //vtkNew<vtkPolyDataMapper> polyMapper;
    //polyMapper->SetInputData(pointSource);
    //polyMapper->Update();
    //vtkNew<vtkActor> polyActor;
    //polyActor->SetMapper(polyMapper);
    //polyActor->GetProperty()->EdgeVisibilityOn();
    //polyActor->GetProperty()->SetEdgeColor(colors->GetColor3d("Black").GetData());

    vtkNew<vtkGlyph3DMapper> quantizedMapper;
    quantizedMapper->SetInputConnection(quantizeFilter->GetOutputPort());
    quantizedMapper->SetSourceConnection(sphereSource->GetOutputPort());
    quantizedMapper->ScalarVisibilityOn();
    quantizedMapper->SetLookupTable(inputMapper->GetLookupTable());
    quantizedMapper->ScalingOff();

    vtkNew<vtkActor> quantizedActor;
    quantizedActor->SetMapper(quantizedMapper);
    quantizedActor->GetProperty()->SetColor(colors->GetColor3d("DodgerBlue").GetData());

    vtkNew<vtkPolyDataMapper> trajectoryMapper;
    trajectoryMapper->SetInputData(trajectoryPolyData);
    //trajectoryMapper->ScalarVisibilityOff();
    //trajectoryMapper->ScalingOff();

    vtkNew<vtkActor> trajectoryActor;
    trajectoryActor->SetMapper(trajectoryMapper);
    trajectoryActor->GetProperty()->SetVertexVisibility(true);
    trajectoryActor->GetProperty()->SetColor(colors->GetColor3d("Blue").GetData());
    //trajectoryActor->GetProperty()->SetLineWidth(scan->parameters.headAndScanCollectorParameters.currentNumArea/4);
    //trajectoryActor->GetProperty()->SetLineWidth(4);
    trajectoryActor->GetProperty()->SetPointSize(5);
    trajectoryActor->AddPosition(0, 0, shiftDeltaZ);

    vtkNew<vtkTubeFilter> tuber;
    tuber->SetInputData(trajectoryPolyData);
    tuber->SetNumberOfSides(20);
    tuber->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
    tuber->SetRadius(2);

    vtkNew<vtkPolyDataMapper> tubeMapper;
    tubeMapper->SetInputConnection(tuber->GetOutputPort());
    //tubeMapper->SetScalarRange(tubePolyData->GetScalarRange());

    //vtkNew<vtkPolyDataMapper> lineMapper;
    //lineMapper->SetInputData(tubePolyData);
    //lineMapper->SetScalarRange(tubePolyData->GetScalarRange());
    //vtkNew<vtkActor> lineActor;
    //lineActor->SetMapper(lineMapper);
    //lineActor->GetProperty()->SetLineWidth(3);
   
    //vtkNew<vtkActor> tubeActor;
    //tubeActor->SetMapper(tubeMapper);
    //tubeActor->GetProperty()->SetOpacity(0.6);
    //tubeActor->GetProperty()->SetColor(colors->GetColor3d("Blue").GetData());
    //tubeActor->AddPosition(0,0,shiftDeltaZ);

    //***
    vtkNew<vtkSphereSource> sphereSourceRt;
    sphereSourceRt->SetRadius(radiusZ);

    vtkNew<vtkPolyDataMapper> inputMapperRtPoints;
    inputMapperRtPoints->SetInputData(peakRtPolyData);
    inputMapperRtPoints->SetScalarModeToUsePointData();
    inputMapperRtPoints->SetColorModeToMapScalars();
    inputMapperRtPoints->CreateDefaultLookupTable();
    inputMapperRtPoints->SetScalarRange(peakRtPolyData->GetScalarRange());
    //inputMapperRtPoints->SetScalarModeToDefault();

    vtkNew<vtkGlyph3DMapper> inputMapperRt;
    inputMapperRt->SetInputData(peakRtPolyData);
    inputMapperRt->SetSourceConnection(parametricFunctionSource->GetOutputPort());
    inputMapperRt->SetScalarModeToUsePointData();
    //inputMapperRt->SetRange(peakRtPolyData->GetScalarRange());
    inputMapperRt->CreateDefaultLookupTable();
    inputMapperRt->GetLookupTable()->SetRange(0, 5);
    inputMapperRt->Update();
    //auto tableRt = buildCTF(false, params.colorStopsList);
    //tableRt->SetRange(minValueRt, maxValueRt);
    //inputMapperRt->SetLookupTable(tableRt);

    vtkNew<vtkScalarBarActor> scalarBarRt;
    scalarBarRt->SetLookupTable(inputMapperRt->GetLookupTable());
    scalarBarRt->SetTitle("RT Amplitude");
    scalarBarRt->UnconstrainedFontSizeOn();
    scalarBarRt->DragableOn();
    scalarBarRt->DrawFrameOn();
    //scalarBarRt->SetDisplayPosition(10, 50);
    scalarBarRt->Modified();

    //scalarBarRt->SetOrientationToHorizontal();

    vtkNew<vtkActor> inputActorRt;
    inputActorRt->SetMapper(inputMapperRt);
    inputActorRt->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());
    inputActorRt->GetProperty()->SetOpacity(0.4);
    inputActorRt->AddPosition(radiusZ, radiusZ, shiftDeltaZ);

    vtkNew<vtkActor> inputActorRtPoints;
    inputActorRtPoints->SetMapper(inputMapperRtPoints);
    inputActorRtPoints->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
    inputActorRtPoints->GetProperty()->SetPointSize(5);
    inputActorRtPoints->AddPosition(0, 0, shiftDeltaZ);

    //vtkNew<vtkPolyDataMapper> quantizeTrajectoryRtMapper;
    //quantizeTrajectoryRtMapper->SetInputConnection(quantizeFilterRt->GetOutputPort());
    //quantizeTrajectoryRtMapper->CreateDefaultLookupTable();
    //quantizeTrajectoryRtMapper->SetColorModeToMapScalars();
    //quantizeTrajectoryRtMapper->SetColorModeToDirectScalars();

    //vtkNew<vtkActor> quantizeTrajectoryActor;
    //quantizeTrajectoryActor->SetMapper(quantizeTrajectoryRtMapper);
    //quantizeTrajectoryActor->GetProperty()->SetVertexVisibility(true);
    //quantizeTrajectoryActor->GetProperty()->SetColor(colors->GetColor3d("Pink").GetData());
    //quantizeTrajectoryActor->GetProperty()->SetPointSize(5);
    //quantizeTrajectoryActor->AddPosition(0, 0, shiftDeltaZ);

    vtkNew<vtkGlyph3DMapper> quantizedMapperRt;
    quantizedMapperRt->SetInputConnection(quantizeFilterRt->GetOutputPort());
    quantizedMapperRt->SetSourceConnection(parametricFunctionSource->GetOutputPort());
    quantizedMapperRt->ScalarVisibilityOn();
    quantizedMapperRt->ScalingOff();

    vtkNew<vtkActor> quantizedActorRt;
    quantizedActorRt->SetMapper(quantizedMapperRt);
    quantizedActorRt->GetProperty()->SetColor(colors->GetColor3d("Aquamarine").GetData());
    //***

  // Now create a lookup table that consists of the full hue circle
  // (from HSV).
    vtkNew<vtkLookupTable> hueLut;
    hueLut->SetTableRange(0, 2000);
    hueLut->SetHueRange(0, 1);
    hueLut->SetSaturationRange(1, 1);
    hueLut->SetValueRange(1, 1);
    hueLut->Build(); // effective built

    // Finally, create a lookup table with a single hue but having a range
    // in the saturation of the hue.
    vtkNew<vtkLookupTable> satLut;
    satLut->SetTableRange(0, 2000);
    satLut->SetHueRange(0.6, 0.6);
    satLut->SetSaturationRange(0, 1);
    satLut->SetValueRange(1, 1);
    satLut->Build(); // effective built

    // Create the second (axial) plane of the three planes. We use the
   // same approach as before except that the extent differs.
    imageDefectsView->SetDimensions(10, 10, 1);
    //imageDefectsView->

    imageData->SetDimensions(256, 256, 1);
    imageData->AllocateScalars(VTK_DOUBLE, 1);

    const int* dims = imageData->GetDimensions();
    std::cout << "Number of points: " << imageData->GetNumberOfPoints()
        << std::endl;
    std::cout << "Number of cells: " << imageData->GetNumberOfCells()
        << std::endl;

    for (int z = 0; z < dims[2]; z++)
    {
        for (int y = 0; y < dims[1]; y++)
        {
            for (int x = 0; x < dims[0]; x++)
            {
                double* pixel =
                    static_cast<double*>(imageData->GetScalarPointer(x, y, z));
                pixel[0] = 215.0;
            }
        }
    }
    imageData->Modified();

    ImageGridSource->SetGridSpacing(2, 2, 0);
    ImageGridSource->SetGridOrigin(0, 0, 0);
    ImageGridSource->SetDataExtent(0, 255, 0, 255, 0, 0);
    ImageGridSource->SetDataScalarTypeToUnsignedChar();
    ImageGridSource->SetFillValue(122);
    ImageGridSource->Update();

    vtkNew<vtkNamedColors> colors;

    vtkNew<vtkImageData> colorImage;
    CreateColorImage(colorImage, 255, 255);

    vtkNew<vtkImageMapper> imageMapper;
    imageMapper->SetInputData(colorImage);
    imageMapper->SetColorWindow(255);
    imageMapper->SetColorLevel(127.5);

    vtkNew<vtkActor2D> imageActor;
    imageActor->SetMapper(imageMapper);
    imageActor->SetPosition(20, 20);

    //vtkImageData* image = new 
    //axialColors->SetInputConnection(ImageGridSource->GetOutputPort()); //TODO на входе должен быть vtkImageData
    axialColors->SetInputData(colorImage); //TODO на входе должен быть vtkImageData
    axialColors->SetLookupTable(hueLut);
    axialColors->Update();

    //axial->SetInputData(colorImage);
    //axial->GetMapper()->SetInputConnection(axialColors->GetOutputPort());
    axial->GetMapper()->SetInputData(colorImage);
    axial->SetDisplayExtent(0, 255, 0, 255, 10, 10);
    axial->ForceOpaqueOn();

    //// Create the third (coronal) plane of the three planes. We use
    //// the same approach as before except that the extent differs.
    //vtkNew<vtkImageMapToColors> coronalColors;
    //coronalColors->SetInputData(ImageGridSource->GetInput());
    //coronalColors->SetLookupTable(satLut);
    //coronalColors->Update();
    //    vtkNew<vtkImageActor> coronal;
    //coronal->GetMapper()->SetInputConnection(coronalColors->GetOutputPort());
    //coronal->SetDisplayExtent(0, 255, 128, 128, 0, 92);
    //coronal->ForceOpaqueOn();


        // Define viewport ranges.
        // (xmin, ymin, xmax, ymax)
    double leftViewport[4] = { 0.0, 0.0, 0.5, 1.0 };
    double rightViewport[4] = { 0.5, 0.0, 1.0, 1.0 };

    // Setup both renderers.
    renderWindow->AddRenderer(leftRenderer);
    leftRenderer->SetViewport(leftViewport);
    leftRenderer->SetBackground(colors->GetColor3d("Bisque").GetData());

    renderWindow->AddRenderer(rightRenderer);

    rightRenderer->SetViewport(rightViewport);
    rightRenderer->SetBackground(colors->GetColor3d("PaleTurquoise").GetData());
    rightRenderer->GradientBackgroundOn();

     renderer->AddActor(inputActor);    //11_03_2026 - заремарил этустроку //16_03_2026 - вернул
    //renderer->AddActor(inputActorRt); //11_03_2026 - заремарил этустроку //16_03_2026 - вернул
    renderer->AddActor(inputActorRtPoints);
    renderer->AddActor(trajectoryActor);
    //renderer->AddActor2D(scalarBar);
    //renderer->AddActor2D(scalarBarRt);

    leftRenderer->AddActor(inputActor); //11_03_2026 - заремарил этустроку //16_03_2026 - вернул
    leftRenderer->AddActor(inputActorRt);
    leftRenderer->AddActor(trajectoryActor);
    leftRenderer->AddActor(inputActorRtPoints);
    //leftRenderer->AddActor(quantizeTrajectoryActor);
    //leftRenderer->AddActor(tubeActor);
    leftRenderer->AddActor2D(scalarBar);
    leftRenderer->AddActor2D(scalarBarRt);
    //leftRenderer->AddActor(polyActor);
    rightRenderer->AddActor(quantizedActor);
    rightRenderer->AddActor(quantizedActorRt);
    //leftRenderer->AddActor(axial);
    //leftRenderer->AddActor(coronal);
    //leftRenderer->AddActor2D(imageActor);

    leftRenderer->ResetCamera();
    //leftRenderer->ResetCameraClippingRange();
    //leftRenderer->GetActiveCamera()->SetPosition(1.0, 1.0, -4.0);
    rightRenderer->SetActiveCamera(leftRenderer->GetActiveCamera());

    vtkNew<vtkTextProperty> tprop;
    tprop->SetColor(colors->GetColor3d("Yellow").GetData());
    tprop->ShadowOn();
    tprop->SetFontSize(20);
    // Create a vtkCubeAxesActor2D. Use the outer edges of the bounding box to
    // draw the axes. Add the actor to the renderer.
    vtkNew<vtkCubeAxesActor2D> axes1;
    axes1->SetInputConnection(polyDataSource->GetOutputPort());
    axes1->SetCamera(leftRenderer->GetActiveCamera());
    axes1->SetLabelFormat("%6.2f");
    axes1->SetFlyModeToOuterEdges();
    axes1->SetAxisTitleTextProperty(tprop);
    axes1->SetAxisLabelTextProperty(tprop);
    axes1->GetProperty()->SetLineWidth(2);
    leftRenderer->AddViewProp(axes1);

    // Create a vtkCubeAxesActor2D. Use the closest vertex to the camera to
    // determine where to draw the axes. Add the actor to the renderer.
    vtkNew<vtkCubeAxesActor2D> axes2;
    axes2->SetViewProp(quantizedActor);
    axes2->SetInputConnection(polyDataSource->GetOutputPort());
    axes2->SetCamera(rightRenderer->GetActiveCamera());
    axes2->SetLabelFormat("%6.2g");
    axes2->SetFlyModeToClosestTriad();
    axes2->ScalingOff();
    axes2->SetAxisTitleTextProperty(tprop);
    axes2->SetAxisLabelTextProperty(tprop);
    axes2->GetProperty()->SetLineWidth(2);
    rightRenderer->AddViewProp(axes2);

    //double* bx = new double((bs) / sizeOfWindow[0]);
    double* bx = new double(10.0);
    double bd = 10.0;
    double by = 10.0;
    double bs = 10.0;
    
    ////buttonWidgetXY->createAndPlace(interactor, leftRenderer, (iconDir + "/icons /btnXY.png").c_str(), bx, bd, by, bs);
    //buttonWidgetXY->createAndPlace(interactor, leftRenderer, "/icons /btnXY.png", bx, bd, by, bs);
    ////buttonWidgetXY->AddObserver(vtkCommand::StateChangedEvent, callbackButtonWidgetXY);
    //buttonWidgetXY->On();


    vtkNew<MyStyle2> interactorStyle;
    interactorStyle->SetDefaultRenderer(leftRenderer);
    interactorStyle->SetCurrentRenderer(leftRenderer);
    interactor->SetInteractorStyle(interactorStyle);

    renderWindow->SetSize(640, 360);
    renderWindow->SetWindowName("333QuantizePolyDataPoints");
    interactor->SetRenderWindow(renderWindow);

    scalarBarWidgetRt->SetInteractor(interactor);
    scalarBarWidgetRt->SetScalarBarActor(scalarBarRt);
    scalarBarWidgetRt->GetScalarBarActor()->SetPosition(0, 0);
    scalarBarWidgetRt->On();

    scalarBarWidget->SetInteractor(interactor);
    scalarBarWidget->SetScalarBarActor(scalarBar);
    scalarBarWidget->On();

    //interactor->GetInteractorStyle()->AutoAdjustCameraClippingRange();

    renderWindow->Render();
    //interactor->Start();
    interactor->Initialize();
}

void PeakAndBscanVTKView::showNView(std::shared_ptr<Scan> scan, int n, bool needToShowOriginalView, bool needShowRandomizedData)
{
    //vtkNew<vtkNamedColors> colors;
    ////showNView Create a grid
    //vtkNew<vtkRectilinearGrid> grid;
    std::srand(std::time(nullptr));

    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    auto grid = vtkSmartPointer<vtkRectilinearGrid>::New();

    // Размер панели
    double LPanel = 9000;
    double HPanel = 300;
    int LPanelStrikeCount = 23400;
    int HPanelStrikeCount = 100;

    if (scan && (!needShowRandomizedData) && (scan->lines.size() > 0)) {
        LPanel = std::abs(double(scan->normalizedRanges.front().finalCoordinate - scan->normalizedRanges.front().startCoordinate));
        HPanel = std::abs(double(scan->normalizedRanges.front().finalLineCoordinate - scan->normalizedRanges.front().lineCoordinate));
        LPanelStrikeCount = scan->normalizedRanges.front().maxView[0].size();
        HPanelStrikeCount = scan->normalizedRanges.front().maxView[0, 0].size();
    }

    grid->SetDimensions(LPanelStrikeCount, HPanelStrikeCount, 1);

    double deltaX = LPanel / LPanelStrikeCount;
    double deltaY = HPanel / HPanelStrikeCount;

    int viewPortCounts = n;
    double halfDeltaXforCamera = (LPanel / viewPortCounts) / 2;

    double xSize = (grid->GetDimensions()[0]) * deltaX;

    // --- X координаты ---
    auto xArray = vtkSmartPointer<vtkDoubleArray>::New();
    double val = -deltaX;

    for (int i = 0; i < grid->GetDimensions()[0]; i++)
    {
        val += deltaX;
        xArray->InsertNextValue(val);
    }


    // --- Y координаты ---
    //ySize = (grid.GetDimensions()[1]) * deltaY
    auto yArray = vtkSmartPointer<vtkDoubleArray>::New();
    double valy = -deltaY;

    for (int i = 0; i < grid->GetDimensions()[1]; i++)
    {
        valy += deltaY;
        yArray->InsertNextValue(valy);
    }

    // --- Z координаты ---
    auto zArray = vtkSmartPointer<vtkDoubleArray>::New();
    double delta = 2.0;
    val = -delta;

    for (int i = 0; i < grid->GetDimensions()[2]; i++)
    {
        val += delta;
        zArray->InsertNextValue(val);
    }

    grid->SetXCoordinates(xArray);
    grid->SetYCoordinates(yArray);
    grid->SetZCoordinates(zArray);

    // --- Scalars ---
    auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetName("MyScalars");

    vtkIdType gridsNumPoints = grid->GetNumberOfPoints();

    for (vtkIdType id = 0; id < gridsNumPoints; id++)
    {
        //double p[3];
        //grid->GetPoint(id,p);
        //# scalars.InsertNextValue(0.5 + random.randint(0, 100) / 100 * p[0] / xSize)
        vtkIdType rnd = std::rand() % gridsNumPoints;

        double randomPoint[3];
        grid->GetPoint(rnd, randomPoint);
        scalars->InsertNextValue(randomPoint[0] / xSize);
    }

    // Рисуем круги
    double rndz = 0.0;
    double r = 5.0;
    double valu = -1.0 * xSize;
    paintCircle(grid, scalars, r, valu, deltaX, deltaY, 0);
    paintCircle(grid, scalars, r, valu, deltaX, deltaY, 0);
    paintCirclePure(grid, scalars, r, valu, deltaX, deltaY, 0);


    // --- Границы viewport ---

    for (int j = 0; j < viewPortCounts; j++)
    {
        for (int i = 0; i < HPanelStrikeCount; i++)
        {
            int ijk[3];
            double pcoords[3];

            double point[3] = {
                (j + 1) * 2 * halfDeltaXforCamera,
                i * deltaY,
                0.0
            };

            int inside = grid->ComputeStructuredCoordinates(point, ijk, pcoords);

            if (inside)
            {
                vtkIdType idp = grid->ComputePointId(ijk);

                scalars->SetValue(idp, 100);
                if (idp > 0) scalars->SetValue(idp - 1, 100);
                if (idp < gridsNumPoints - 1) scalars->SetValue(idp + 1, 100);
            }
        }
    }

    grid->GetPointData()->SetScalars(scalars);
    grid->Modified();

    // --- Mapper ---
    auto mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    mapper->ScalarVisibilityOn();
    mapper->SetColorModeToMapScalars();
    //    mapper.Update()

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d("PeachPuff").GetData());

    // --- Render window ---

    //auto rw = vtkSmartPointer<vtkRenderWindow>::New();
    //auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(rw);

    //double xmins[] = { 0, 0, 0, 0 };
    //double xmaxs[] = { 1, 1, 1, 1 };
    double allViewSize = 1.0;
    double viewSize = 0.1;
    double commonViewSize = 0.0;
    if (scan->parameters.commonViewNeeded) {
        commonViewSize = 0.1;
    };
    viewSize = (allViewSize - commonViewSize) / viewPortCounts;
    std::vector<double> xmins;
    std::vector<double> xmaxs;
    std::vector<double> ymins;
    std::vector<double> ymaxs;
    std::vector<double> camera_height;
    double camera_height_value = 900.0;
    std::vector <const char*> ren_bkg;

    std::vector <double> camera_position_X;

    double value = 0.0;
    for (auto i = 0; i < viewPortCounts; i++) {
        xmins.push_back(0.0);
        xmaxs.push_back(1.0);
        ymins.push_back(value);
        value += viewSize;
        ymaxs.push_back(value);
        camera_height.push_back(camera_height_value);
        auto colo = "LightGrey";
        if (i % 2) {
            colo = "Grey";
        }
        ren_bkg.push_back(colo);
        camera_position_X.push_back(halfDeltaXforCamera*(2*i+1));
    }
    if (scan->parameters.commonViewNeeded) {
        xmins.push_back(0.0);
        xmaxs.push_back(1.0);
        ymins.push_back(value);
        value += commonViewSize;
        ymaxs.push_back(value);
        camera_height.push_back(camera_height_value);
        ren_bkg.push_back("LightGrey");
        camera_position_X.push_back(LPanel/2);
    };
    //double ymins[] = { 0, 0.3, 0.6, 0.9 };
    //double ymaxs[] = { 0.3, 0.6, 0.9, 1 };
    //double camera_height[] = { 900, 900, 900, 900 };
    //const char* ren_bkg[] = { "LightGrey", "Grey", "LightGrey", "Grey" };
    //double camera_position_X[] = {
    //    halfDeltaXforCamera,
    //    3 * halfDeltaXforCamera,
    //    5 * halfDeltaXforCamera,
    //    LPanel / 2
    //};

    double camera_position_Y = HPanel / 2;


    for (int i = 0; i < std::size(xmins); i++)
    {
        auto ren = vtkSmartPointer<vtkRenderer>::New();
        ren->SetViewport(xmins[i], ymins[i], xmaxs[i], ymaxs[i]);

        rw->AddRenderer(ren);

        auto camera = vtkSmartPointer<vtkCamera>::New();

        camera->SetFocalPoint(camera_position_X[i], camera_position_Y, 0);
        camera->SetPosition(camera_position_X[i], camera_position_Y, camera_height[i]);

        ren->SetActiveCamera(camera);
        ren->AddActor(actor);
        ren->SetBackground(colors->GetColor3d(ren_bkg[i]).GetData());
    }

    rw->SetSize(2500, 1300);
    rw->SetWindowName("900cmx30cm");

    rw->Render();
    //iren->Start();
    iren->Initialize();

}

void PeakAndBscanVTKView::createImageDataFromDefectsView(vtkImageData* image, ::DefectsView* renderedDefectPoints)
{
    auto xDim = renderedDefectPoints->view.shape()[0];
    auto yDim = renderedDefectPoints->view.shape()[1];
        image->SetDimensions(xDim, yDim, 1);
        image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

        //vtkNew<vtkNamedColors> colors;
        //auto pixelColor = colors->GetColor3ub("Turquoise").GetData();
        //std::array<unsigned char, 3> drawColor1{ 0, 0, 0 };
        //for (auto i = 0; i < 3; ++i)
        //{
        //    drawColor1[i] = pixelColor[i];
        //}

        for (unsigned int x = 0; x < xDim; x++)
         {
            for (unsigned int y = 0; y < yDim; y++)
            {
                ::RgbColor color = renderedDefectPoints->view[x][y];
                unsigned char* pixel =
                    static_cast<unsigned char*>(image->GetScalarPointer(x, (yDim-1)- y, 0));
                pixel[0] = color.red;
                pixel[1] = color.green;
                pixel[2] = color.blue;
                //for (auto j = 0; j < 3; ++j)
                //{
                //    pixel[j] = pixelColor[j];
                //}
            }
        }
        image->Modified();
}


