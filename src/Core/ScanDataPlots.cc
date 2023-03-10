/*
 * Core/ScanDataPlots.cc
 */

#include <boost/assign/list_of.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/numeric.hpp>
#include <UCL/Exception.hh>
#include <UCL/PlotView/AttributeKeys.hh>

#include "Core/ScanDataPlots.hh"
#include "Core/Qwt/RangeScanLineSeriesData.hh"
#include "Core/Qwt/SourceScanLineSeriesData.hh"
#include <vtkDoubleArray.h>

namespace adp = boost::adaptors;
namespace p   = boost::phoenix;
namespace pa  = boost::phoenix::arg_names;

SourceScanLineCurve::
SourceScanLineCurve(const QString& objectName, const QString& className, const SourceScanLine& line)
  : objectName(objectName), className(className), line(line)
{ }

QString 
SourceScanLineCurve::
getObjectName() const
{
  return objectName;
}

QString 
SourceScanLineCurve::
getClassName() const
{
  return className;
}

QVariant 
SourceScanLineCurve::
getAttribute(const QUuid& key, const QVariant& defaultValue) const
{
  if (key == uts::plotting::attribute::SAMPLE_RATE) {
    return line.sampleRate;
  } else {
    return defaultValue;
  }
}

vtkSmartPointer<vtkTable>
SourceScanLineCurve::
getPoints()
{
  auto table = vtkTable::New();
  auto size = line.samples.size();
  auto xs = vtkSmartPointer<vtkDoubleArray>::New();
  auto ys = vtkSmartPointer<vtkDoubleArray>::New();
  xs->SetName("X");
  ys->SetName("Y");

  for (auto i = 0u; i < size; i++){
    xs->InsertNextValue((line.finalCoordinate - line.startCoordinate) * double(i) / double(line.samples.size()) + line.startCoordinate);
    ys->InsertNextValue(line.samples[i]);
  }
  table->AddColumn(xs);
  table->AddColumn(ys);
  return table;
}

RangeScanLineCurve::
RangeScanLineCurve(const QString& objectName, const QString& className, const RangeScanLine& line)
  : objectName(objectName), className(className), line(line)
{ }

QString 
RangeScanLineCurve::
getObjectName() const
{
  return objectName;
}

QString 
RangeScanLineCurve::
getClassName() const
{
  return className;
}

vtkSmartPointer<vtkTable>
RangeScanLineCurve::
getPoints()
{
  auto table = vtkSmartPointer<vtkTable>::New();
  auto xs = vtkSmartPointer<vtkDoubleArray>::New();
  auto ys = vtkSmartPointer<vtkDoubleArray>::New();
  auto size = line.samples.size();
  xs->SetName("X");
  ys->SetName("Y");

  for (auto i = 0u; i < size; i++){
    xs->InsertNextValue((line.finalCoordinate - line.startCoordinate) * double(line.sampleIndexes[i]) / double(line.sourceLineSize) + line.startCoordinate);
    ys->InsertNextValue(line.samples[i]);
  }

  table->AddColumn(xs);
  table->AddColumn(ys);

  return table;
}

SourceScanLineGroup::
SourceScanLineGroup(const QString& name, const QString& className, const SourceScanLine& line)
  : name(name), className(className), line(line)
{ }

QString 
SourceScanLineGroup::
getName() const
{
  return name;
}

std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> 
SourceScanLineGroup::
getCurves() const
{
  return { std::make_shared<SourceScanLineCurve>(name, className, line) };
}

std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> 
SourceScanLineGroup::
getMarkers() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractMarker>>();
}

boost::optional<QRectF> 
SourceScanLineGroup::
getZoomBase() const
{
  QRectF rect;
  rect.setLeft(line.startCoordinate);
  rect.setRight(line.finalCoordinate);
  if (! line.samples.empty()) {
    rect.setBottom(*boost::min_element(line.samples));
    rect.setTop(*boost::max_element(line.samples));
  } else {
    rect.setBottom(0);
    rect.setTop(0);
  }

  return rect;
}

std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> 
SourceScanLineGroup::
getAuxiliaryGroups() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>>();
}

QString SourceScanLineGroup::getClassName() const
{
  return className;
}

std::shared_ptr<uts::plotting::AbstractSurface> SourceScanLineGroup::getSurface() const
{
  return {};
}

RangeScanLineGroup::
RangeScanLineGroup(const QString& name, const QString& className, const RangeScanLine& line)
  : name(name), className(className), line(line)
{ }

QString 
RangeScanLineGroup::
getName() const
{
  return name;
}

std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> 
RangeScanLineGroup::
getCurves() const
{
  return{ std::make_shared<RangeScanLineCurve>(name, className, line) };
}

std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> 
RangeScanLineGroup::
getMarkers() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractMarker>>();
}

boost::optional<QRectF> 
RangeScanLineGroup::
getZoomBase() const
{
  QRectF rect;
  rect.setLeft(line.startCoordinate);
  rect.setRight(line.finalCoordinate);
  if (! line.samples.empty()) {
    rect.setBottom(*boost::min_element(line.samples));
    rect.setTop(*boost::max_element(line.samples));
  } else {
    rect.setBottom(0);
    rect.setTop(0);
  }

  return rect;
}

std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> 
RangeScanLineGroup::
getAuxiliaryGroups() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>>();
}

QString RangeScanLineGroup::getClassName() const
{
  return className;
}

std::shared_ptr<uts::plotting::AbstractSurface> RangeScanLineGroup::getSurface() const
{
  return {};
}

ScanDataRangeGroup::
ScanDataRangeGroup(const QString& name, 
                   const RangeScanLine& range)
  : name(name), range(range)
{ }

QString 
ScanDataRangeGroup::
getName() const
{
  return name;
}

std::vector<std::shared_ptr<uts::plotting::AbstractCurve>> 
ScanDataRangeGroup::
getCurves() const
{
  return{ std::make_shared<RangeScanLineCurve>(QString("line %1 %2").arg(range.range.from).arg(range.range.to),
                                               "range-high-frequency-line",range) };
}

std::vector<std::shared_ptr<uts::plotting::AbstractMarker>> 
ScanDataRangeGroup::
getMarkers() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractMarker>>();
}

boost::optional<QRectF> 
ScanDataRangeGroup::
getZoomBase() const
{
  QRectF rect;
  rect.setLeft(range.startCoordinate);
  if (! range.samples.empty()) {
    rect.setRight(range.finalCoordinate);
    rect.setBottom(*boost::min_element(range.samples));
    rect.setTop(*boost::max_element(range.samples));
  } else {
    rect.setRight(0);
    rect.setBottom(0);
    rect.setTop(0);
  }

  return rect;
}

std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>> 
ScanDataRangeGroup::
getAuxiliaryGroups() const
{
  return std::vector<std::shared_ptr<uts::plotting::AbstractItemGroup>>();
}

QString ScanDataRangeGroup::getClassName() const
{
  return "range";
}

std::shared_ptr<uts::plotting::AbstractSurface> ScanDataRangeGroup::getSurface() const
{
  return {};
}

SourceScanLineCategory::
SourceScanLineCategory(const std::shared_ptr<Scan>& scan)
  : scan(scan)
{ }

QString 
SourceScanLineCategory::
getName() const
{
  return "source-scan-line-category";
}

unsigned int 
SourceScanLineCategory::
getGroupCount() const
{
    return 0;// scan->lines.size();
}

std::shared_ptr<uts::plotting::AbstractItemGroup> 
SourceScanLineCategory::
getGroup(unsigned int idx) const
{
  /*if (idx < scan->lines.size()) {
    return std::make_shared<SourceScanLineGroup>(QString("scaled line %1").arg(idx), "scaled-line", scan->lines[idx]);
  } else {
    BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
  }*/
}

unsigned int 
SourceScanLineCategory::
getSubCategoryCount() const
{
  return 0;
}

std::shared_ptr<uts::plotting::AbstractItemGroupCategory> 
SourceScanLineCategory::
getSubCategory(unsigned int idx) const
{
  BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
}

QString SourceScanLineCategory::getClassName() const
{
  return "source-scan";
}

LineRangesMetaCategory::
LineRangesMetaCategory(const std::shared_ptr<Scan>& scan)
  : scan(scan)
{ }

QString 
LineRangesMetaCategory::
getName() const
{
  return "line-ranges-category";
}

unsigned int 
LineRangesMetaCategory::
getGroupCount() const
{
  return 0;
}

std::shared_ptr<uts::plotting::AbstractItemGroup> 
LineRangesMetaCategory::
getGroup(unsigned int idx) const
{
  BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
}

unsigned int 
LineRangesMetaCategory::
getSubCategoryCount() const
{
  return scan->ranges.size();
}

std::shared_ptr<uts::plotting::AbstractItemGroupCategory> 
LineRangesMetaCategory::
getSubCategory(unsigned int idx) const
{
  if (idx >= scan->ranges.size()) {
    BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
  }

  return std::make_shared<LineRangesCategory>(scan->ranges[idx], idx + 1);
}

QString LineRangesMetaCategory::getClassName() const
{
  return "line-ranges-meta";
}

LineRangesCategory::
LineRangesCategory(const std::vector<RangeScanLine>& ranges, unsigned int lineNumber)
  : ranges(ranges), lineNumber(lineNumber)
{ }

QString 
LineRangesCategory::
getName() const
{
  return QString("line-%1-ranges-category").arg(lineNumber);
}

unsigned int 
LineRangesCategory::
getGroupCount() const
{
  return ranges.size();
}

std::shared_ptr<uts::plotting::AbstractItemGroup> 
LineRangesCategory::
getGroup(unsigned int idx) const
{
  if (idx >= ranges.size()) {
    BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
  } else {
    return std::make_shared<ScanDataRangeGroup>(QString("line-range %1 [ %2 %3 ]").arg(idx + 1).arg(ranges[idx].range.from).arg(ranges[idx].range.to),
                                                ranges[idx]);
  }
}

unsigned int 
LineRangesCategory::
getSubCategoryCount() const
{
  return 0;
}

std::shared_ptr<uts::plotting::AbstractItemGroupCategory> 
LineRangesCategory::
getSubCategory(unsigned int idx) const
{
  BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
}

QString LineRangesCategory::getClassName() const
{
  return "line-ranges";
}

ScanDataRangesModel::
ScanDataRangesModel(const std::shared_ptr<Scan>& scan)
  : scan(scan)
{ }

unsigned 
ScanDataRangesModel::
getGroupCount() const 
{
  return 0;
    /*scan->lines.size()
    + boost::accumulate(scan->ranges | adp::transformed(p::size(pa::_1)), 0)
    ;*/
}

std::shared_ptr<uts::plotting::AbstractItemGroup>
ScanDataRangesModel::
getGroup(unsigned idx) const
{
  /*if (idx < scan->lines.size()) {
    return std::make_shared<SourceScanLineGroup>(QString("scaled line %1").arg(idx), "scaled-line", scan->lines[idx]);
  } else {
    idx -= scan->lines.size();

    for (std::size_t ir = 0; ir != scan->ranges.size(); ir++) {
      if (idx >= scan->ranges[ir].size()) {
        idx -= scan->ranges[ir].size();
        continue;
      }

      return std::make_shared<ScanDataRangeGroup>(
        QString("line %1 range %2 [ %3 %4 ]").arg(ir + 1).arg(idx + 1).arg(scan->ranges[ir][idx].range.from).arg(scan->ranges[ir][idx].range.to), 
        scan->ranges[ir][idx]);
    }
  }*/

  BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
}

unsigned int 
ScanDataRangesModel::
getCategoryCount() const
{
  return 2;
}

std::shared_ptr<uts::plotting::AbstractItemGroupCategory>
ScanDataRangesModel::
getCategory(unsigned int idx) const
{
  switch (idx) {
  default:
    BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Index(idx));
  case 0:
    return std::make_shared<SourceScanLineCategory>(scan);
  case 1:
    return std::make_shared<LineRangesMetaCategory>(scan);
  }
}
