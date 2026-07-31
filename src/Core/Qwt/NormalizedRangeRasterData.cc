/*
 * Core/Qwt/NormalizedRangeRasterData.cc
 */

#include <iterator>
#include <limits>
#include <boost/range/algorithm/lower_bound.hpp>
//#include <iostream> //*******

#include "Core/Qwt/NormalizedRangeRasterData.hh"


NormalizedRangeRasterData::NormalizedRangeRasterData(const NormalizedRange& range)
  : range(range)
{
  m_intervals.at(Qt::XAxis) = QwtInterval(range.startCoordinate, range.finalCoordinate);
  double minLocal = 0; //  если получили одну строку, задаем ей ненулевую высоту для корректного определения площади дефекта
  double maxLocal = 1;//  если получили одну строку, задаем ей ненулевую высоту для корректного определения площади дефекта
    if (range.lineCoordinates.begin() != range.lineCoordinates.end()) {
        auto xMinLocal = std::min_element(range.lineCoordinates.begin(), range.lineCoordinates.end());
        auto xMaxLocal = std::max_element(range.lineCoordinates.begin(), range.lineCoordinates.end());
        auto xMinLocalF = std::min_element(range.finalLineCoordinates.begin(), range.finalLineCoordinates.end());
        auto xMaxLocalF = std::max_element(range.finalLineCoordinates.begin(), range.finalLineCoordinates.end());

        minLocal = std::min(*xMinLocal, *xMinLocalF);
        maxLocal = std::max(*xMaxLocal, *xMaxLocalF);
    }
    m_intervals.at(Qt::YAxis) = QwtInterval(minLocal, maxLocal);
 // float minVal = std::numeric_limits<float>::max();
 // float maxVal = -std::numeric_limits<float>::max();
 ///* std::cout << static_cast<int>(range.extremum); */
 // for (std::size_t j = 0; j < range.view.shape()[1]; j++) {
 //   for (std::size_t i = 0; i < range.view.shape()[0]; i++) {
 //     minVal = std::min(minVal, range.view[i][j]);
 //     maxVal = std::max(maxVal, range.view[i][j]);
 //   }
 // }
 // m_intervals.at(Qt::ZAxis)  = QwtInterval(minVal, maxVal);
    m_intervals.at(Qt::ZAxis)  = QwtInterval(range.min, range.max);
}

double NormalizedRangeRasterData::value(double x, double y) const
{
  double px = (x - range.startCoordinate) / double(std::abs(range.finalCoordinate - range.startCoordinate));
  std::size_t ix = std::max(std::size_t(0), std::min(static_cast<std::size_t>(std::floor(px * range.view.val.shape()[0] + 0.5)), range.view.val.shape()[0] - 1));

  auto yiter = (range.lineCoordinates.back() > range.lineCoordinates.front()) 
             ? boost::lower_bound(range.lineCoordinates, y)
             : boost::lower_bound(range.lineCoordinates, y, std::greater<double>());
  if (yiter != range.lineCoordinates.begin()) --yiter;
  auto iy = std::distance(range.lineCoordinates.begin(), yiter);

  return range.view.val[ix][iy];
}

QwtInterval NormalizedRangeRasterData::interval(Qt::Axis axis) const {
    try {
        return m_intervals.at(axis);
    }
    catch (std::out_of_range exeption) {
        return QwtInterval();
    }
}

