/*
 * Core/Qwt/VerticalPolinomialSeriesData.cc
 */

#include "Core/Qwt/VerticalPolinomialSeriesData.hh"

namespace {

  double evalPoly(const std::vector<double>& coefs, double x)
  {
    double y = 0.0;
    for (std::size_t i = 0; i < coefs.size(); i++)
      y += std::pow(x, i) * coefs[i];
    return y;
  }

}

VerticalPolinomialSeriesData::VerticalPolinomialSeriesData(const std::vector<double>& coefs, double start, double end)
  : coefs(coefs), start(start), end(end)
{
  rect = qwtBoundingRect(*this);
}

size_t VerticalPolinomialSeriesData::size() const
{
  return std::abs(end - start) * 10;
}

QPointF VerticalPolinomialSeriesData::sample(size_t i) const
{
  int sign = start < end ? 1 : -1;
  return QPointF(evalPoly(coefs, start + sign * double(i) / 10.0), start + sign * double(i) / 10.0);
}

QRectF VerticalPolinomialSeriesData::boundingRect() const
{
  return rect;
}
