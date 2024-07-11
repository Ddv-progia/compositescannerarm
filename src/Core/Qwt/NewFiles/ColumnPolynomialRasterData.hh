/*
 * Core/Qwt/ColumnPolynomialRasterData.hh
 */

#pragma once

#include <qwt_raster_data.h>
#include "Core/ScanData.hh"
#include <memory>
#include <qwt_interval.h>

class ColumnPolynomialRasterData : public QwtRasterData
{
	std::array< QwtInterval, 3> m_intervals;
public:
  explicit ColumnPolynomialRasterData(const std::vector<Polynomial>& polynomials, unsigned int sampleRate, unsigned int lineCount);
  virtual double value(double x, double y) const override;
  virtual QwtInterval interval(Qt::Axis axis) const override;

private:
  const std::vector<Polynomial>& polynomials;
  unsigned int sampleRate;
  unsigned int lineCount;
};