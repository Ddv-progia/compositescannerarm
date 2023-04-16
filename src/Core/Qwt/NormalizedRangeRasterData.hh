/*
 * Core/Qwt/NormalizedRangeRasterData.hh
 */

#pragma once

#include <boost/multi_array.hpp>
#include <qwt_raster_data.h>
#include <qwt_interval.h>
#include "Core/ScanData.hh"
#include <array>

class NormalizedRangeRasterData : public QwtRasterData
{
	std::array< QwtInterval, 3> m_intervals;
public:
  explicit NormalizedRangeRasterData(const NormalizedRange& range);
  virtual double value(double x, double y) const override;
  virtual QwtInterval interval(Qt::Axis axis) const override;

private:
  const NormalizedRange& range;
};