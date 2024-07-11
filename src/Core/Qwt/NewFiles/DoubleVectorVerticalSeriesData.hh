/*
 * Core/Qwt/DoubleVectorVerticalSeriesData.hh
 */

#pragma once

#include <qwt_series_data.h>

class DoubleVectorVerticalSeriesData : public QwtSeriesData<QPointF>
{
public:
  DoubleVectorVerticalSeriesData(const std::vector<double>& line, const std::vector<double>& lineCoordinates);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  const std::vector<double>& line;
  const std::vector<double>& lineCoordinates;
  QRectF rect;
};
