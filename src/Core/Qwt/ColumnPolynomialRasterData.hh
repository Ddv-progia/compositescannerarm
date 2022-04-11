/*
 * Core/Qwt/ColumnPolynomialRasterData.hh
 */

#pragma once

#include <qwt_raster_data.h>
#include "Core/ScanData.hh"

class ColumnPolynomialRasterData : public QwtRasterData
{
public:
  explicit ColumnPolynomialRasterData(const std::vector<Polynomial>& polynomials, unsigned int sampleRate, unsigned int lineCount);
  virtual double value(double x, double y) const override;

private:
  const std::vector<Polynomial>& polynomials;
  unsigned int sampleRate;
  unsigned int lineCount;
};