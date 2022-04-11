/*
 * Core/ImageProcessing.hh
 */

#pragma once 

#include <QImage>
#include <QObject>
#include <qwt_plot.h>
#include <Core/ScanData.hh>
#include <boost/multi_array.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include "opencv2/opencv.hpp"

namespace improc
{

  namespace contourWindow
  {
    void show(cv::Mat& img);
  }

  cv::Mat qImageToMat(const QImage& image);
  QImage* matToQImage(const cv::Mat& image);
  std::vector<std::vector<cv::Point> > findContours(const cv::Mat& img, int threshold = 1);

  namespace rules{
	  class DefectSelectionRule;
  }
};


struct Defect {
  std::vector<QPointF> contour;
  double area;
  double height;
  double width;
  QPointF center;
  DefectType type;
  Defect(const std::vector<QPointF>& contour, double area, double height = 0,double width = 0,QPointF center = QPointF(0,0)):
		contour(contour),area(area),height(height),width(width),center(center) {}
};




// арта скана в модели составлена из предположени€,что площадь 1 элемента = 1 пикселю. ћаштабирование 
//под конечное изображение(на экране) производитс€ умножением координат дефекта на шаг пикселей в конечном изображении.
//“акой подход позвол€ет примен€ть алгаритмы OpenCV без преобразований координат и работать с массивами как с координатами
//пикселей на карте скана

class PlotDefectsModel: public QObject
{
  Q_OBJECT
  std::vector<Defect> defects;
  std::shared_ptr<Scan> scan;
  std::vector<Defect> filteredDefects;
  
  void addDefect(const std::vector<cv::Point>& cvContour);
  std::vector<Defect> selectDefectsByRule(const std::vector<Defect>& inDefects, improc::rules::DefectSelectionRule* rule);
public:
  PlotDefectsModel(const std::shared_ptr<Scan>& scan,DefectsView& defectView, QObject* parent = 0);
  PlotDefectsModel(const std::shared_ptr<Scan>& scan, QObject* parent = 0);
  std::vector<Defect> currentDefects();
  Q_SLOT void selectSingleDefect(size_t n);
  
  Q_SIGNAL void defectSelected(const Defect&);
};
