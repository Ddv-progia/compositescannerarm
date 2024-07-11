/*
 * Core/Qwt/MultiArrayColorSliceVerticalSeriesData.hh
 */

#pragma once

#include <boost/multi_array.hpp>
#include <qwt_series_data.h>

#include "Core/ScanData.hh"

class MultiArrayColorSliceVerticalSeriesData : public QwtSeriesData<QPointF>
{
public:
  explicit MultiArrayColorSliceVerticalSeriesData(const boost::multi_array<RgbColor, 2>::const_array_view<1>::type& line, 
                                                  unsigned int sampleRate,
                                                  uint8_t RgbColor::* member);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  boost::multi_array<RgbColor, 2>::const_array_view<1>::type line;
  unsigned int sampleRate;
  uint8_t RgbColor::* member;
  QRectF rect;
};
