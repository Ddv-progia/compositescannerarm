#pragma once

#include <boost/accumulators/statistics/variance.hpp>
#include <QVTKOpenGLNativeWidget.h>
#include <QVTKInteractor.h>
#include <sstream>
#include <vtkActor2D.h>
#include <vtkButtonWidget.h>
#include <vtkChart.h>
#include <vtkChartMatrix.h>
#include <vtkContextView.h>
#include <vtkCoordinate.h>
#include <vtkGlyph3DMapper.h>
#include <vtkImageMapToColors.h>
#include <vtkImageGridSource.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleJoystickCamera.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkNamedColors.h>

#include <vtkNew.h>
#include <vtkPNGReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkTable.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkVectorText.h>


#include <vtkAbstractPicker.h>
#include <vtkCoordinate.h>
#include <vtkDataSetMapper.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageMapper3D.h>
#include <vtkMapper.h>
#include <vtkQuantizePolyDataPoints.h>
#include <vtkScalarBarWidget.h>
#include <vtkSphereSource.h>

#include <vtkPolyDataMapper2D.h>
#include <vtkProp3DButtonRepresentation.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <Core/ScanData.hh>


//class vtkButtonWidget2D_2 : public vtkButtonWidget
//{
//public:
//
//	static vtkButtonWidget2D_2* New();
//	vtkTypeMacro(vtkButtonWidget2D_2, vtkButtonWidget);
//	vtkButtonWidget2D_2();
//
//	vtkNew<vtkVectorText> atext;
//	vtkNew<vtkActor> textActor;
//	vtkNew<vtkTexturedButtonRepresentation2D> buttonRepresentation2D;
//	void createAndPlace(vtkRenderWindowInteractor* interactor, vtkRenderer* renderer, const char* fileName,
//		double* x = new double(0.02), double  xSpace = 0.004, double  y = 0.02, double size = 30.0);
//	//private:
//  //  vtkActor* LastPickedActor;
//  //  vtkProperty* LastPickedProperty;
//};
//vtkStandardNewMacro(vtkButtonWidget2D_2);


class PeakAndBscanVTKView : public QVTKOpenGLNativeWidget
{
	//Q_OBJECT
public:
	PeakAndBscanVTKView(QWidget* parentIn = Q_NULLPTR);
	~PeakAndBscanVTKView();
	PeakAndBscanVTKView(std::shared_ptr<Scan> scan, QWidget* parent = Q_NULLPTR);
	vtkNew<vtkDiscretizableColorTransferFunction> buildCTF(bool const& raduga, std::vector<ColorStop> colors);
	void constructPeakAndBscanVTKView();
	void SetPolyDataSource(int index, vtkNew<vtkPolyData> &pointSource, ::std::vector< ::NormalizedRange > *normalizedRange);
	void SetPolyDataSourceFromParameters(vtkNew<vtkPolyData> &pointSource, double startCoordinate, double finalCoordinate, ::std::vector< double > lineCoordinates, boost::multi_array<::RgbColor, 2> view );
	void ChartPeak(Peak& peak, std::shared_ptr<Scan>& scan);
	void ShowPeak(Peak& peak, std::shared_ptr<Scan>& scan);
	void showQuantizedPoints(std::shared_ptr<Scan> scan);
	void createImageDataFromDefectsView(vtkImageData* image, ::DefectsView* renderedDefectPoints);
	//QPointer<QVTKOpenGLNativeWidget> armVtkRenderWidget = new QVTKOpenGLNativeWidget();
	//vtkRenderWindowInteractor* renderWindowInteractor;
	vtkNew < QVTKInteractor> renderWindowInteractor;
	vtkNew<vtkGenericOpenGLRenderWindow> genericOpenGLRenderWindow;
	//vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
	//vtkNew<QVTKInteractor> interactor;
	vtkNew<vtkRenderWindowInteractor> interactor;
	//vtkSmartPointer<vtkButtonWidget2D_2> buttonWidgetXY;
	//vtkNew<vtkButtonWidget2D_2> buttonWidgetXY;
	vtkNew<vtkRenderWindow> renderWindow;
	vtkNew<vtkRenderer> renderer;
	vtkNew<vtkRenderer> leftRenderer;
	vtkNew<vtkRenderer> rightRenderer;
	vtkNew<vtkActor> actor;
	vtkNew<vtkActor> actorQuantized;
	vtkNew<vtkDataSetMapper> mapper;
	vtkNew<vtkGlyph3DMapper> inputMapper;
	vtkNew<vtkDataSetMapper> mapperQuantized;
	vtkNew<vtkPolyData> sphere;
	vtkNew<vtkTrivialProducer> polyDataSource;
	vtkNew<vtkQuantizePolyDataPoints> quantizeFilter;
	vtkNew<vtkScalarBarWidget> scalarBarWidget;
	vtkNew<vtkScalarBarWidget> scalarBarWidgetRt;
	vtkNew<vtkNamedColors> colors;
	std::shared_ptr<Scan> scan;
	vtkSmartPointer<vtkImageData> imageDefectsView = vtkSmartPointer<vtkImageData>::New();
	vtkNew<vtkContextView> view;
	vtkNew<vtkChartMatrix> matrix;
	vtkNew<vtkTable> tableAll;
	vtkNew<vtkFloatArray> arrXAll;
	vtkNew<vtkFloatArray> arrYAll;
	vtkNew<vtkTable> tablePeak;
	vtkNew<vtkFloatArray> arrX;
	vtkNew<vtkFloatArray> arrP;

	vtkNew<vtkPolyData> pointSource;
	vtkNew<vtkImageData> imageData;
	vtkNew<vtkImageGridSource> ImageGridSource;
	vtkNew<vtkImageMapToColors> axialColors;
	vtkNew<vtkImageActor> axial;


	vtkSmartPointer<vtkEventQtSlotConnect> Connections;
	//vtkChart* chart;
	//vtkPlot* plot;
};


namespace {
	class MyStyle2 : public vtkInteractorStyleSwitch
		//class MyStyle : public vtkInteractorStyleImage
		//class MyStyle : public vtkInteractorStyleSwitch

	{
	public:
		vtkTypeMacro(MyStyle2, vtkInteractorStyleSwitch);
		static MyStyle2* New();

		std::vector<vtkActor2D*> Numbers;

		void OnLeftButtonDown() override
		{
			vtkInteractorStyleSwitch::OnLeftButtonDown();
			this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
				this->Interactor->GetEventPosition()[1],
				0, // always zero.
				this->CurrentRenderer);
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);
			this->AddNumber(picked);

			// Forward events
			//vtkInteractorStyleImage::OnLeftButtonDown();

			this->Interactor->GetRenderWindow()->Render();
			/*this->Interactor->Render();*/
		}

		void OnRightButtonDown() override
		{
			std::cout << "Pressed right mouse button." << std::endl;
			// Forward events.
			//vtkInteractorStyleTrackballCamera::OnRightButtonDown();
			vtkInteractorStyleSwitch::OnRightButtonDown();
		}

		void AddNumber(double p[3])
		{
			vtkNew<vtkNamedColors> colors;

			std::cout << "Adding marker at " << p[0] << " " << p[1]; //<< std::endl;

			// normally, with an image you would do
			// double* s = image->GetSpacing();
			// double* o = image->GetOrigin();
			// p[0] = static_cast<int>( (p[0] - o[0]) / s[0] + 0.5 );
			p[0] = static_cast<int>(p[0] + 0.5);
			p[1] = static_cast<int>(p[1] + 0.5);

			std::cout << " -> " << p[0] << " " << p[1] << std::endl;

			// Convert the current number to a string
			std::stringstream ss;
			ss << Numbers.size();

			// Create an actor for the text
			vtkNew<vtkVectorText> textSource;
			textSource->SetText(ss.str().c_str());

			// get the bounds of the text
			textSource->Update();
			const double* bounds = textSource->GetOutput()->GetBounds();
			// transform the polydata to be centered over the pick position
			const double center[3] = { 0.5 * (bounds[1] + bounds[0]),
								0.5 * (bounds[3] + bounds[2]), 0.0 };

			vtkNew<vtkTransform> trans;
			trans->Translate(-center[0], -center[1], 0);
			trans->Translate(p[0], p[1], 0);

			vtkNew<vtkTransformPolyDataFilter> tpd;
			tpd->SetTransform(trans);
			tpd->SetInputConnection(textSource->GetOutputPort());

			// Create a mapper
			vtkNew<vtkPolyDataMapper2D> mapper;
			vtkNew<vtkCoordinate> coordinate;
			coordinate->SetCoordinateSystemToWorld();
			mapper->SetTransformCoordinate(coordinate);
			mapper->SetInputConnection(tpd->GetOutputPort());

			vtkNew<vtkActor2D> actor;
			actor->SetMapper(mapper);
			actor->GetProperty()->SetColor(colors->GetColor3d("Yellow").GetData());

			this->CurrentRenderer->AddViewProp(actor);
			this->Numbers.push_back(actor);
		}
	};
	vtkStandardNewMacro(MyStyle2);


}
