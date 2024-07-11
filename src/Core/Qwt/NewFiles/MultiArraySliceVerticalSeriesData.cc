/*
 * Core/Qwt/MultiArraySliceVerticalSeriesData.cc
 */

#include "Core/Qwt/MultiArraySliceVerticalSeriesData.hh"

MultiArraySliceVerticalSeriesData::MultiArraySliceVerticalSeriesData(const boost::multi_array<float, 2>::const_array_view<1>::type& line, 
                                                                     const std::vector<double>& lineCoordinates)
  : line(line), lineCoordinates(lineCoordinates)
{
  rect = qwtBoundingRect(*this);
}

size_t MultiArraySliceVerticalSeriesData::size() const
{
  return line.size();
}

QPointF MultiArraySliceVerticalSeriesData::sample(size_t i) const
{
  return QPointF(line[i], lineCoordinates[i]);
}

QRectF MultiArraySliceVerticalSeriesData::boundingRect() const
{
  return rect;
}
