/*
 * Core/Qwt/SourceScanLineSeriesData.hh
 */

#pragma once

#include <QtCore/QPointF>
#include <qwt_series_data.h>

#include "Core/ScanData.hh"

class SourceScanLineSeriesData : public QwtSeriesData<QPointF>
{
public:
  explicit SourceScanLineSeriesData(const SourceScanLine& line);

  virtual size_t size() const override;
  virtual QPointF sample(size_t i) const override;
  virtual QRectF boundingRect() const override;

private:
  const SourceScanLine& line;
  QRectF rect;
};
