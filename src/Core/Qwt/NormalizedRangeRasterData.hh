/*
 * Core/Qwt/NormalizedRangeRasterData.hh
 */

#pragma once

#include <boost/multi_array.hpp>
#include <qwt_raster_data.h>

#include "Core/ScanData.hh"

class NormalizedRangeRasterData : public QwtRasterData
{
public:
  explicit NormalizedRangeRasterData(const NormalizedRange& range);
  virtual double value(double x, double y) const override;

private:
  const NormalizedRange& range;
};