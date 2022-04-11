/*
 * Core/Qwt/MultiArrayColorSliceVerticalSeriesData.cc
 */

#include "Core/Qwt/MultiArrayColorSliceVerticalSeriesData.hh"

MultiArrayColorSliceVerticalSeriesData::MultiArrayColorSliceVerticalSeriesData(const boost::multi_array<RgbColor, 2>::const_array_view<1>::type& line, 
                                                                               unsigned int sampleRate,
                                                                               uint8_t RgbColor::* member)
  : line(line), sampleRate(sampleRate), member(member)
{
  rect = qwtBoundingRect(*this);
}

size_t MultiArrayColorSliceVerticalSeriesData::size() const
{
  return line.size();
}

QPointF MultiArrayColorSliceVerticalSeriesData::sample(size_t i) const
{
  return QPointF(line[i].*member, double(i) / double(sampleRate));
}

QRectF MultiArrayColorSliceVerticalSeriesData::boundingRect() const
{
  return rect;
}
