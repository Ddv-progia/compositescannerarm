/*
 * Core/Qwt/ColumnPolynomialRasterData.cc
 */

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/min.hpp>

#include "Core/Qwt/ColumnPolynomialRasterData.hh"

namespace ba = boost::accumulators;

namespace {

  double evalPoly(const Polynomial& poly, double x)
  {
    double y = 0.0;
    for (std::size_t i = 0; i < poly.coefficients.size(); i++)
      y += std::pow(x, i) * poly.coefficients[i];
    return y;
  }

}

ColumnPolynomialRasterData::ColumnPolynomialRasterData(const std::vector<Polynomial>& polynomials, unsigned int sampleRate, unsigned int lineCount)
  : polynomials(polynomials), sampleRate(sampleRate), lineCount(lineCount)
{ 
  setInterval(Qt::XAxis, QwtInterval(0, double(polynomials.size() - 1) / double(lineCount)));
  setInterval(Qt::YAxis, QwtInterval(0, lineCount - 1));

  ba::accumulator_set<double, ba::features<ba::tag::max, ba::tag::min>> zValueAcc;
  for (auto const& poly : polynomials) {
    for (double x = 0; x < lineCount; x++) {
      zValueAcc(evalPoly(poly, x));
    }
  }

  setInterval(Qt::ZAxis, QwtInterval(ba::min(zValueAcc), ba::max(zValueAcc)));
}

double ColumnPolynomialRasterData::value(double x, double y) const
{
  return evalPoly(polynomials[x], y);
}