/*
 * Core/Qwt/SourceScanLineSeriesData.cc
 */

#include "Core/Qwt/SourceScanLineSeriesData.hh"

SourceScanLineSeriesData::
SourceScanLineSeriesData(const SourceScanLine& line)
  : line(line)
{ 
  rect = qwtBoundingRect(*this);
}

size_t 
SourceScanLineSeriesData::
size() const
{
  return line.samples.size();
}

QPointF 
SourceScanLineSeriesData::
sample(size_t i) const
{
  return QPointF((line.finalCoordinate - line.startCoordinate) * double(i) / double(line.samples.size()) + line.startCoordinate, line.samples[i]);
}

QRectF 
SourceScanLineSeriesData::
boundingRect() const
{
  return rect;
}
