/*
 * Core/ScanDataPlots.hh
 */

#pragma once

#include <UCL/PlotView/Model.hh>
#include "Core/ScanData.hh"
#include <qwt_series_data.h>
#include <vtkTable.h>
#include <vtkSmartPointer.h>

class SourceScanLineCurve : public uts::plotting::AbstractCurve
{
public:
  SourceScanLineCurve(const QString& objectName, const QString& className, const SourceScanLine& line);

  virtual QString getObjectName() const override;
  virtual QString getClassName() const override;
  virtual QVariant getAttribute(const QUuid& key, const QVariant& defaultValue = QVariant()) const override;
  virtual vtkSmartPointer<vtkTable> getPoints() override;
  virtual double currentAbgle() override { return 0; }
  virtual double currentVerticalShift() override { return 0; }
  virtual double currentHorizontalShift() override { return 0; }
  virtual void transform(double angle, double vertShift, double horizShift)override {  }

private:
  QString objectName;
  QString className;
  const SourceScanLine& line;
};

class RangeScanLineCurve : public uts::plotting::AbstractCurve
{
public:
  RangeScanLineCurve(const QString& objectName, const QString& className, const RangeScanLine& line);

  virtual QString getObjectName() const override;
  virtual QString getClassName() const override;
  virtual vtkSmartPointer<vtkTable> getPoints() override;
  virtual double currentAbgle() override { return 0; }
  virtual double currentVerticalShift() override { return 0; }
  virtual double currentHorizontalShift() override { return 0; }
  virtual void transform(double angle, double vertShift, double horizShift)override {  }
private:
  QString objectName;
  QString className;
  const RangeScanLine& line;
};

class SourceScanLineGroup : public uts::plotting::AbstractItemGroup
{
public:
  SourceScanLineGroup(const QString& name, const QString& className, const SourceScanLine& line);

  QString getName() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> getCurves() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> getMarkers() const override;
  boost::optional<QRectF> getZoomBase() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> getAuxiliaryGroups() const override;
  QString getClassName() const override;
  std::shared_ptr<uts::plotting::AbstractSurface> getSurface() const override;
private:
  QString name;
  QString className;
  const SourceScanLine& line;
};

class RangeScanLineGroup : public uts::plotting::AbstractItemGroup
{
public:
  RangeScanLineGroup(const QString& name, const QString& className, const RangeScanLine& line);

  QString getName() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> getCurves() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> getMarkers() const override;
  boost::optional<QRectF> getZoomBase() const override;
  std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> getAuxiliaryGroups() const override;
  QString getClassName() const override;
  std::shared_ptr<uts::plotting::AbstractSurface> getSurface() const override;
private:
  QString name;
  QString className;
  const RangeScanLine& line;
};


class ScanDataRangeGroup : public uts::plotting::AbstractItemGroup
{
public:
  ScanDataRangeGroup(const QString& name, const RangeScanLine& range);

  virtual QString getName() const override;
  virtual std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> getCurves() const override;
  virtual std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> getMarkers() const override;
  virtual boost::optional<QRectF> getZoomBase() const override;
  virtual std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> getAuxiliaryGroups() const override;

  QString getClassName() const override;

  std::shared_ptr<uts::plotting::AbstractSurface> getSurface() const override;
private:
  QString name;
  const RangeScanLine& range;
};

class SourceScanLineCategory : public uts::plotting::AbstractItemGroupCategory
{
public:
  explicit SourceScanLineCategory(const std::shared_ptr<Scan>& scan);

  virtual QString getName() const override;
  virtual unsigned int getGroupCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroup> getGroup(unsigned int idx) const override;
  virtual unsigned int getSubCategoryCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroupCategory> getSubCategory(unsigned int idx) const override;

  QString getClassName() const override;
private:
  std::shared_ptr<Scan> scan;
};

class LineRangesMetaCategory : public uts::plotting::AbstractItemGroupCategory
{
public:
  explicit LineRangesMetaCategory(const std::shared_ptr<Scan>& scan);

  virtual QString getName() const override;
  virtual unsigned int getGroupCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroup> getGroup(unsigned int idx) const override;
  virtual unsigned int getSubCategoryCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroupCategory> getSubCategory(unsigned int idx) const override;

  QString getClassName() const override;
private:
  std::shared_ptr<Scan> scan;
};

class LineRangesCategory : public uts::plotting::AbstractItemGroupCategory
{
public:
  explicit LineRangesCategory(const std::vector<RangeScanLine>& ranges, unsigned int lineNumber);

  virtual QString getName() const override;
  virtual unsigned int getGroupCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroup> getGroup(unsigned int idx) const override;
  virtual unsigned int getSubCategoryCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroupCategory> getSubCategory(unsigned int idx) const override;

  QString getClassName() const override;
private:
  const std::vector<RangeScanLine>& ranges;
  unsigned int lineNumber;
};

class ScanDataRangesModel : public uts::plotting::AbstractPlotCollectionModel
{
public:
  explicit ScanDataRangesModel(const std::shared_ptr<Scan>& scan);

  virtual unsigned getGroupCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroup> getGroup(unsigned idx) const override;

  virtual unsigned int getCategoryCount() const override;
  virtual std::shared_ptr<uts::plotting::AbstractItemGroupCategory> getCategory(unsigned idx) const override;
private:
  std::shared_ptr<Scan> scan;
};
