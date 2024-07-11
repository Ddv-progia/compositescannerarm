/*
 * Core/Qwt/ScanLineSeriesData.cc
 */

#include "Core/Qwt/RangeScanLineSeriesData.hh"

RangeScanLineSeriesData::
RangeScanLineSeriesData(const RangeScanLine& line)
  : line(line)
{ 
  rect = qwtBoundingRect(*this);
}

size_t 
RangeScanLineSeriesData::
size() const
{
  return line.samples.size();
}

QPointF 
RangeScanLineSeriesData::
sample(size_t i) const
{
  return QPointF((line.finalCoordinate - line.startCoordinate) * double(line.sampleIndexes[i]) / double(line.sourceLineSize) + line.startCoordinate, 
                 line.samples[i]);
}

QRectF 
RangeScanLineSeriesData::
boundingRect() const
{
  return rect;
}
