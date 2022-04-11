/*
 * Core/Qwt/MultiArraySliceSeriesData.cc
 */

#include "Core/Qwt/MultiArraySliceSeriesData.hh"

MultiArraySliceSeriesData::MultiArraySliceSeriesData(const boost::multi_array<float, 2>::const_array_view<1>::type& line, 
                                                     double startCoordinate,
                                                     double finalCoordinate)
  : line(line), startCoordinate(startCoordinate), finalCoordinate(finalCoordinate)
{
  rect = qwtBoundingRect(*this);
}

size_t MultiArraySliceSeriesData::size() const
{
  return line.size();
}

QPointF MultiArraySliceSeriesData::sample(size_t i) const
{
  return QPointF(startCoordinate + double(i) * std::abs(finalCoordinate - startCoordinate) / double(line.size()), line[i]);
}

QRectF MultiArraySliceSeriesData::boundingRect() const
{
  return rect;
}
