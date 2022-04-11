/*
 * Core/Qwt/DefectPointsItem.hh
 */

#pragma once

#include <vector>
#include <qwt_plot_item.h>
#include <qwt_plot_rasteritem.h>

#include "Core/ScanData.hh"

class DefectPointsItem : public QwtPlotRasterItem
{
public:
  DefectPointsItem(const DefectsView& defects);
  virtual QRectF boundingRect() const override;

  static const int Rtti_DefectPointsItem = QwtPlotItem::Rtti_PlotUserItem + 1;

protected:
  virtual QImage renderImage(const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& area, const QSize& imageSize) const override;

private:
  const DefectsView& defects;
  QRectF rect;
};
