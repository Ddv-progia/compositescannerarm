/*
 * Core/Qwt/DefectPointsItem.cc
 */

#include <algorithm>
#include <boost/range/empty.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <qwt_scale_map.h>

#include "Core/Qwt/DefectPointsItem.hh"
#include <Qt6/QtGui/qimage.h>

namespace adp = boost::adaptors;

DefectPointsItem::DefectPointsItem(const DefectsView& defects)
  : defects(defects)
{ 
  if (defects.lineCoordinates.back() > defects.lineCoordinates.front()) {
    rect.setBottom(defects.lineCoordinates.front());
    rect.setTop(defects.lineCoordinates.back() + (defects.lineCoordinates.back() - defects.lineCoordinates[defects.lineCoordinates.size() - 2]));
  } else {
    rect.setTop(defects.lineCoordinates.front());
    rect.setBottom(defects.lineCoordinates.back() + (defects.lineCoordinates.back() - defects.lineCoordinates[defects.lineCoordinates.size() - 2]));
  }

  
  rect.setLeft(defects.startCoordinate);
  rect.setRight(defects.finalCoordinate);
}

QRectF DefectPointsItem::boundingRect() const
{
  return rect;
}

QImage DefectPointsItem::renderImage(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& area, const QSize& imageSize) const
{
  auto imageData = new uint32_t[static_cast<std::size_t>(xMap.pDist() * yMap.pDist())];

  for (std::size_t py = 0, imageIndex = 0; py < yMap.pDist(); py++) {
    for (std::size_t px = 0; px < xMap.pDist(); px++, imageIndex++) {
      auto sx = static_cast<int>(std::floor(
        (xMap.invTransform(px) - defects.startCoordinate) / double(std::abs(defects.finalCoordinate - defects.startCoordinate)) * defects.view.shape()[0] + 0.5));
      auto sy = yMap.invTransform(py);

      auto yiter = (defects.lineCoordinates.back() > defects.lineCoordinates.front()) 
             ? boost::lower_bound(defects.lineCoordinates, sy)
             : boost::lower_bound(defects.lineCoordinates, sy, std::greater<double>());
      if (yiter != defects.lineCoordinates.begin()) --yiter;
      auto iy = std::distance(defects.lineCoordinates.begin(), yiter);

      if (sx >= 0 && iy >= 0 && sx < defects.view.shape()[0] && iy < defects.view.shape()[1]) {
          imageData[imageIndex] = (0xFF << 24) & (defects.view[sx][iy].red << 16) & (defects.view[sx][iy].green << 8) & defects.view[sx][iy].blue;
      } else {
        imageData[imageIndex] = 0xFF000000;
      }
    }
  }

  return QImage(reinterpret_cast<uchar*>(imageData), xMap.pDist(), yMap.pDist(), QImage::Format_RGB32, 
                [] (void* x) { delete[] static_cast<uint32_t*>(x); });
}
