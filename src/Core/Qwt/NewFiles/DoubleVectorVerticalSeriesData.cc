/*
 * Core/Qwt/DoubleVectorVerticalSeriesData.cc
 */

#include "Core/Qwt/DoubleVectorVerticalSeriesData.hh"

DoubleVectorVerticalSeriesData::DoubleVectorVerticalSeriesData(const std::vector<double>& line, const std::vector<double>& lineCoordinates)
  : line(line), lineCoordinates(lineCoordinates)
{
  rect = qwtBoundingRect(*this);
}

size_t DoubleVectorVerticalSeriesData::size() const
{
  return line.size();
}

QPointF DoubleVectorVerticalSeriesData::sample(size_t i) const
{
  return QPointF(line[i], lineCoordinates[i]);
}

QRectF DoubleVectorVerticalSeriesData::boundingRect() const
{
  return rect;
}
