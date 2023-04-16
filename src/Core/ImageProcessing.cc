/*
 * Core/ImageProcessing.cc
 */

#include"Core/ImageProcessing.hh"
#include "opencv2/imgproc/imgproc.hpp"
#include <qmessagebox.h>

namespace improc
{
  namespace contourWindow
  {
    cv::Mat src, src_gray, dst, detected_edges;
    int lowThreshold;
    int const max_lowThreshold = 100;
    int const ratio = 3;
    int const kernel_size = 3;
    const char* window_name = "Edge Map";


    void CannyThreshold(int, void*)
    {
      /// Reduce noise with a kernel 3x3
      cv::blur( src_gray, detected_edges, cv::Size(3, 3) );
      /// Canny detector
      cv::Canny( detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, kernel_size );
      /// Using Canny’s output as a mask, we display our result
      dst = cv::Scalar::all(0);
      src.copyTo( dst, detected_edges);
      cv::imshow( window_name, dst );
    }


    void show(cv::Mat& img)
    {
      src = img;
      if( !img.data )
        return;
      /// Create a matrix of the same type and size as src (for dst)
      dst.create( img.size(), img.type() );
      cvtColor( img, src_gray, cv::COLOR_BGR2BGRA );
      /// Create a window
      cv::namedWindow( window_name, cv::WINDOW_AUTOSIZE );
      /// Create a Trackbar for user to enter threshold
      cv::createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );
      /// Show the image
      CannyThreshold(0, 0);
      /// Wait until user exit program by pressing a key
      cv::waitKey(0);
      return;
    }
  }
  ///////////////////////////////////////////////////////////

  namespace rules
  {
    class DefectSelectionRule
    {
    public:
      virtual bool operator()(const Defect& defect) {
        return true;
      }
    };

    class SelectByLesserArea: public DefectSelectionRule
    {
      double limit;
    public:
      SelectByLesserArea(double limit): limit(limit) {}
      virtual bool operator()(const Defect& defect) override {
        return (defect.area <= limit) ? true : false;
      }
    };

    class SelectByLargerArea: public DefectSelectionRule
    {
      double limit;
    public:
      SelectByLargerArea(double limit): limit(limit) {}
      virtual bool operator()(const Defect& defect) override {
        return (defect.area >= limit) ? true : false;
      }
    };

    class SelectClosed: public DefectSelectionRule
    {
    public:
      SelectClosed() {}
      virtual bool operator()(const Defect& defect) override {
        return (defect.contour.back() == defect.contour.front()) ? true : false;
      }
    };
  }
/////////////////////
  cv::Mat qImageToMat(const QImage& image)
  {
    cv::Mat mat = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar*)image.bits(), image.bytesPerLine());
    cv::Mat result = cv::Mat(mat.rows, mat.cols, CV_8UC3 );
    int from_to[] = { 0, 0,  1, 1,  2, 2 };
    cv::mixChannels( &mat, 1, &result, 1, from_to, 3 );
    return result;
  }

  QImage* matToQImage(const cv::Mat& image)
  {
    QImage* result;
    if(!image.data)
      return nullptr;
    cv::Mat color;
    //Если один канал у изображения
    if( image.channels() == 1) {
      //Делаем трехканальное изображение
      color.create(cv::Size(image.cols, image.rows), CV_8UC3);
      std::vector<cv::Mat> ls;
      ls.push_back(image);
      ls.push_back(image);
      ls.push_back(image);
      cv::merge(ls, color);
    } else {
      color = image;
    }
    result = new QImage(color.cols, color.rows, QImage::Format_RGB888);
    QMessageBox::warning(nullptr, "Новый openCV", "ImageProcessing.cc str 123");
    /*IplImage im = static_cast<IplImage>(color);
    if(result->bytesPerLine() != im.widthStep) {
      //Копирование по строкам
      for(int i = 0; i < result->height(); i++) {
        memcpy(result->bits() + i * result->bytesPerLine(), color.data + im.widthStep * i, im.widthStep);
      }
    } else {
      //Копирование всего изображения
      memcpy(result->bits(), color.data, im.imageSize);
    }*/
    //в исходном изображении перепутаны каналы B и R (BGR вместо RGB). Исправляем
    result = new QImage(result->rgbSwapped());
    return result;
  }


  std::vector<std::vector<cv::Point> > findContours(const cv::Mat& img,const DefectSearchingParameters& params)
  {
    cv::Mat grayImg, edges, filteredImg;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    if( !img.data )
      return contours;

    //приведение к серому изображению и сглаживание
    cvtColor( img, grayImg, cv::COLOR_RGB2GRAY);
    
	//1) Сглаживание
	if(params.useBlur)
		cv::blur( grayImg, grayImg, cv::Size(params.blurWidth, params.blurHeight) );
    //операция объединения включает в себя последовательную эрозию и дилатацию
	//2) Объединение
	if(params.useUnion){
		cv::Mat kernel = cv::getStructuringElement(params.unionKernel.kernelType, 
			                                       cv::Size( params.unionKernel.width,params.unionKernel.height), 
												   cv::Point( params.unionKernel.centerx, params.unionKernel.centery ) );
		cv::dilate(grayImg, grayImg, kernel);
		cv::erode(grayImg, grayImg, kernel);
	}
	//3) Разъединениe
	if(params.useSeparation){
		cv::Mat kernel = cv::getStructuringElement(params.separationKernel.kernelType, 
			                                       cv::Size( params.separationKernel.width,params.separationKernel.height), 
												   cv::Point( params.separationKernel.centerx, params.separationKernel.centery ) );
		cv::erode(grayImg, grayImg, kernel);
		cv::dilate(grayImg, grayImg, kernel);
	}

    cv::threshold(grayImg,edges,0,255, cv::THRESH_BINARY);
	
	cv::findContours(edges, contours, hierarchy, cv::RETR_LIST, params.edgesApproximationType);

    return contours;
  }
}


void PlotDefectsModel::addDefect(const std::vector<cv::Point>& cvContour)
{
  /*using namespace boost;

  double area;
  geometry::model::d2::point_xy<float> center;
  std::vector<QPointF> contour;
  geometry::model::polygon<geometry::model::d2::point_xy<float> > poly;

  auto averageY=std::abs((scan->lines.back().lineCoordinate-scan->lines.front().lineCoordinate)/scan->lines.size());

  //пересчёт в метрическую систему, считаем координаты центром точки
  for(auto & point : cvContour) {
	double nextY = (point.y+1)!=scan->lines.size() ? scan->lines[point.y+1].lineCoordinate : scan->lines[point.y].lineCoordinate+averageY;
	float y = (scan->lines[point.y].lineCoordinate + nextY)/2;
	float xStep = (scan->renderedDefectPointsFixedColor.front().finalCoordinate - scan->renderedDefectPointsFixedColor.front().startCoordinate ) / scan->normalizedRanges.begin()->view.size();
	float x = xStep*point.x - scan->renderedDefectPointsFixedColor.front().startCoordinate ;

    contour.push_back(QPointF(x, y));
    geometry::append(poly, boost::geometry::model::d2::point_xy<double>(x, y));
  }
  //замыкание контуров
  contour.push_back(contour.front());
  geometry::append(poly, boost::geometry::model::d2::point_xy<double>(contour.front().x(), contour.front().y()));

  area = std::abs(geometry::area(poly));
  geometry::centroid(poly,center);

  double left = contour.front().x();
  double right = left;
  double top = contour.front().y();
  double bottom = top;
  for(auto& point : contour){
	  left = point.x()<left ? point.x() : left; 
	  right = point.x()>right ? point.x() : right; 
	  top = point.y()>top ? point.y() : top; 
	  bottom = point.y()<bottom ? point.y() : bottom; 
  }

  defects.push_back(Defect(contour, area, std::abs(top - bottom), std::abs(right - left),QPointF(center.x(),center.y())));
  */
}

PlotDefectsModel::PlotDefectsModel(const std::shared_ptr<Scan>& scan, DefectsView& defectView, QObject* parent): scan(scan), QObject(parent)
{
  //формирование трехканального изображения
  cv::Mat defectImage(static_cast<int>(defectView.view.begin()->size()), static_cast<int>(defectView.view.size()), CV_8UC3);

  for(auto i = 0; i < defectView.view.begin()->size(); i++) {
    unsigned char* const line( defectImage.ptr<unsigned char>(i) );
    for(auto j = 0; j < 3*defectView.view.size(); j+=3) {
      auto pixelImpl = defectView.view[j/3][i];
      auto pixel = qRgb(pixelImpl.red, pixelImpl.green, pixelImpl.blue);
      if(pixelImpl.red != 255 | pixelImpl.green != 255 | pixelImpl.blue != 255){
		line[j] = 255;
		line[j+1] = 255;
		line[j+2] = 255;
	  }else{
        line[j] = 0;
		line[j+1] = 0;
		line[j+2] = 0;
	  }
    }
  }

  //поиск контуров и занесение в вектор дефектов
  auto contourList = improc::findContours(defectImage,scan->parameters.defectSearching);
  defects.clear();
  for(auto & contour : contourList)
    addDefect(contour);

  filteredDefects = selectDefectsByRule(defects, new improc::rules::SelectByLargerArea(10.0));
}

PlotDefectsModel::PlotDefectsModel(const std::shared_ptr<Scan>& scan, QObject* parent): scan(scan), QObject(parent)
{
    if (!scan->normalizedRanges.empty()) {
        //формирование трехканального изображения
    //*******
        auto sizeA = scan->normalizedRanges.begin()->view.begin()->size();
        auto sizeB = scan->normalizedRanges.begin()->view.size();
        cv::Mat defectImage(static_cast<int>(sizeA),
            static_cast<int>(sizeB), CV_8UC3);
        //for (auto i = 0; i < scan->normalizedRanges.front().view.begin()->size(); i++) {
        //    unsigned char* const line(defectImage.ptr<unsigned char>(i));
        //    for (auto j = 0; j < 3 * scan->normalizedRanges.front().view.size(); j += 3) {
        //        line[j] = 0;
        //        line[j + 1] = 0;
        //        line[j + 2] = 0;
        //    }
        //}
        for (auto i = 0; i < sizeA; i++) {
            unsigned char* const line(defectImage.ptr<unsigned char>(i));
            for (auto j = 0; j < 3 * sizeB; j += 3) {
                line[j] = 0;
                line[j + 1] = 0;
                line[j + 2] = 0;
            }
        }
    //*******
        for (auto& defectRange : scan->parameters.defectSearching.defectRanges) {
            if (defectRange.range >= scan->normalizedRanges.size()) continue;
            auto defectView = scan->normalizedRanges[defectRange.range];
            for (auto i = 0; i < defectView.view.begin()->size(); i++) {
                unsigned char* const line(defectImage.ptr<unsigned char>(i));
                for (auto j = 0; j < 3 * defectView.view.size(); j += 3) {
                    auto value = defectView.view[j / 3][i];
                    if (scan->parameters.defectSearching.isDefectInside) {
                        if (value <= defectRange.maximumValue && value >= defectRange.minimumValue) {
                            line[j] = 255;
                            line[j + 1] = 255;
                            line[j + 2] = 255;
                        }
                        else {
                            continue;
                        }
                    }
                    else {
                        if (value >= defectRange.maximumValue || value <= defectRange.minimumValue) {
                            line[j] = 255;
                            line[j + 1] = 255;
                            line[j + 2] = 255;
                        }
                        else {
                            continue;
                        }
                    }
                }
            }
        }
        //поиск контуров и занесение в вектор дефектов
        auto contourList = improc::findContours(defectImage, scan->parameters.defectSearching);
        defects.clear();
        for (auto& contour : contourList)
            addDefect(contour);
        filteredDefects = selectDefectsByRule(defects, new improc::rules::SelectByLargerArea(scan->parameters.defectSearching.minDefectArea));
        //filteredDefects = selectDefectsByRule(defects, new improc::rules::SelectByLargerArea(10.0));
    }
}


std::vector<Defect> PlotDefectsModel::selectDefectsByRule(const std::vector<Defect>& inDefects, improc::rules::DefectSelectionRule* rule)
{
  std::vector<Defect> result;
  for(auto & defect : inDefects) {
    if((*rule)(defect))
      result.push_back(defect);
  }
  return result;
}

std::vector<Defect> PlotDefectsModel::currentDefects()
{
  return filteredDefects;
}

void PlotDefectsModel::selectSingleDefect(size_t n)
{
  if(filteredDefects.size() <= n) return;
  emit defectSelected(filteredDefects.at(n));
}