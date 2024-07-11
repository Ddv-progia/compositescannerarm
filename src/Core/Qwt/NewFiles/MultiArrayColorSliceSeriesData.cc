/*
 * Core/Qwt/MultiArrayColorSliceSeriesData.cc
 */

#include "Core/Qwt/MultiArrayColorSliceSeriesData.hh"

MultiArrayColorSliceSeriesData::MultiArrayColorSliceSeriesData(const boost::multi_array<RgbColor, 2>::const_array_view<1>::type& line, 
                                                               double startCoordinate,
                                                               double finalCoordinate,
                                                               uint8_t RgbColor::* member)
  : line(line), startCoordinate(startCoordinate), finalCoordinate(finalCoordinate), member(member)
{
  rect = qwtBoundingRect(*this);
}

size_t MultiArrayColorSliceSeriesData::size() const
{
  return line.size();
}

QPointF MultiArrayColorSliceSeriesData::sample(size_t i) const
{
  return QPointF(startCoordinate + double(i) * std::abs(finalCoordinate - startCoordinate) / double(line.size()), line[i].*member);
}

QRectF MultiArrayColorSliceSeriesData::boundingRect() const
{
  return rect;
}
