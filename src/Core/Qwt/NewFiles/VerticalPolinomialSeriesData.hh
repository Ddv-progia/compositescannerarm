/*
 * Core/Qwt/VerticalPolinomialSeriesData.hh
 */

#pragma once

#include <qwt_series_data.h>

class VerticalPolinomialSeriesData : public QwtSeriesData<QPointF>
{
public:
  explicit VerticalPolinomialSeriesData(const std::vector<double>& coefs, double start, double end);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  std::vector<double> coefs;
  double start;
  double end;
  QRectF rect;
};
