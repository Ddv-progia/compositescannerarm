/*
 * Core/Qwt/ScanLineSeriesData.hh
 */

#pragma once

#include <QtCore/QPointF>
#include <qwt_series_data.h>

#include "Core/ScanData.hh"

class RangeScanLineSeriesData : public QwtSeriesData<QPointF>
{
public:
  explicit RangeScanLineSeriesData(const RangeScanLine& line);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  const RangeScanLine& line;
  QRectF rect;
};
