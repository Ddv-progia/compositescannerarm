/*
 * Core/Qwt/MultiArraySliceSeriesData.hh
 */

#pragma once

#include <boost/multi_array.hpp>
#include <qwt_series_data.h>

class MultiArraySliceSeriesData : public QwtSeriesData<QPointF>
{
public:
  explicit MultiArraySliceSeriesData(const boost::multi_array<float, 2>::const_array_view<1>::type& line, 
                                     double startCoordinate,
                                     double finalCoordinate);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  boost::multi_array<float, 2>::const_array_view<1>::type line;
  double startCoordinate;
  double finalCoordinate;
  QRectF rect;
};
