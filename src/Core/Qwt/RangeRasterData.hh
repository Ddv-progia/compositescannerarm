/*
 * Core/Qwt/RangeRasterData.hh
 */

#pragma once

#include <boost/multi_array.hpp>
#include <qwt_raster_data.h>
#include <qwt_interval.h>
#include "Core/ScanData.hh"

class RangeRasterData : public QwtRasterData
{
	std::array< QwtInterval, 3> m_intervals;
public:
  explicit RangeRasterData(std::vector<std::vector<RangeScanLine>> const & data,int rangeN);
  virtual double value(double x, double y) const override;
  virtual QwtInterval interval(Qt::Axis axis) const override;

private:
  std::vector<RangeScanLine> range;
  std::vector<double> linesCoordinates;
  double beginX, endX;
  float minZ, maxZ;
};