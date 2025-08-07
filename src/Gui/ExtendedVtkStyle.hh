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
#include <QVTKOpenGLNativeWidget.h>
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
#include <array>

namespace myStyleSpace {

	class MyStyle : public vtkInteractorStyleImage
	{
	public:
		static MyStyle* New();
		vtkTypeMacro(MyStyle, vtkInteractorStyleImage);

		std::vector<vtkActor2D*> Numbers;

		void OnLeftButtonDown() override
		{
			this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
				this->Interactor->GetEventPosition()[1],
				0, // always zero.
				this->CurrentRenderer);
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);
			this->AddNumber(picked);

			// Forward events
			vtkInteractorStyleImage::OnLeftButtonDown();

			// this->Interactor->GetRenderWindow()->Render();
			this->Interactor->Render();
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
	vtkStandardNewMacro(MyStyle);
}

