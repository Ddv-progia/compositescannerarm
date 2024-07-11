/*
 * Core/Qwt/RangeRasterData.cc
 */

#include <iterator>
#include <limits>
#include <boost/range/algorithm/lower_bound.hpp>

#include "Core/Qwt/RangeRasterData.hh"
#include <qwt_interval.h>

RangeRasterData::RangeRasterData(std::vector<std::vector<RangeScanLine>> const & data, int rangeN) : 
  beginX(std::numeric_limits<double>::min())
  , endX(std::numeric_limits<double>::max())
  , minZ(std::numeric_limits<float>::max())
  , maxZ(std::numeric_limits<float>::min())
{
  for (auto const& line : data){
    auto rangeLine = line[rangeN];
    if (rangeLine.startCoordinate > rangeLine.finalCoordinate){
      std::swap(rangeLine.finalCoordinate, rangeLine.startCoordinate);
      //std::reverse(rangeLine.samples.begin(), rangeLine.samples.end());
    }
    range.push_back(std::move(rangeLine));
  }
  
  for (auto& line : range)
  {
    linesCoordinates.push_back(line.lineCoordinate);
    beginX = std::max(line.startCoordinate, beginX);
    endX = std::min(line.finalCoordinate, endX);
    auto maxZIter = std::max_element(line.samples.begin(), line.samples.end());
    auto minZIter = std::min_element(line.samples.begin(), line.samples.end());
    if (maxZIter != line.samples.end())
      maxZ = std::max(maxZ, *maxZIter);
    if (minZIter != line.samples.end())
      minZ = std::min(minZ, *minZIter);
  }
  m_intervals.at(Qt::XAxis) = QwtInterval(beginX, endX);

  if (linesCoordinates.back() > linesCoordinates.front()) {
      m_intervals.at(Qt::YAxis) = QwtInterval(linesCoordinates.front(),linesCoordinates.back() + (linesCoordinates.back() - linesCoordinates[linesCoordinates.size() - 2]));
  } else {
      m_intervals.at(Qt::YAxis) = QwtInterval(linesCoordinates.back() + (linesCoordinates.back() - linesCoordinates[linesCoordinates.size() - 2]),linesCoordinates.front());
  }
  m_intervals.at(Qt::ZAxis) = QwtInterval(minZ, maxZ);
}

double RangeRasterData::value(double x, double y) const
{
  auto yiter = (linesCoordinates.back() > linesCoordinates.front())
    ? boost::lower_bound(linesCoordinates, y)
    : boost::lower_bound(linesCoordinates, y, std::greater<double>());
  if (yiter != linesCoordinates.begin()) --yiter;
  auto iy = std::distance(linesCoordinates.begin(), yiter);

  auto xStep = double(std::abs(endX - beginX))/range[iy].samples.size();

  auto ix = std::max(std::size_t(0), static_cast<std::size_t> ((x - beginX) / xStep));
  ix = std::min(ix, range[iy].samples.size() - 1);
  return range[iy].samples[ix];
}

QwtInterval RangeRasterData::interval(Qt::Axis axis) const {
    try {
        return m_intervals.at(axis);
    }
    catch (std::out_of_range exeption) {
        return QwtInterval();
    }
}
