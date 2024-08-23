/*
 * Core/SaveScanTask.cc
 */

#include <cstdint>
#include <boost/format.hpp>
#include <db_cxx.h>

#include "Core/SaveScanTask.hh"
#include "Core/WMACompression.hh"

namespace {

  template<typename T>
  void putVal(Db& db, const std::string& name, T x)
  {
    Dbt key((void*)name.c_str(), name.size());
    Dbt value(&x, sizeof(x));
    db.put(0, &key, &value, DB_NOOVERWRITE);
  }

  void putStr(Db& db, const std::string& name, const std::string& x)
  {

    Dbt key((void*)name.c_str(), name.size());
    Dbt value((void*)x.data(), x.size());
    db.put(0, &key, &value, DB_NOOVERWRITE);
  }

  template<typename T>
  void putVec(Db& db, const std::string& name, const std::vector<T>& x)
  {
    Dbt key((void*)name.c_str(), name.size());
    Dbt value((void*)x.data(), x.size() * sizeof(T));
    db.put(0, &key, &value, DB_NOOVERWRITE);
  }

}

SaveScanTask::SaveScanTask(const QString& pathname, const Scan& scan, LineEncoding encoding)
  : pathname(pathname), scan(scan), encoding(encoding)
{ }

void SaveScanTask::operator() ()
{
  Db db(0, 0);

  try {
    db.open(0, pathname.toUtf8().data(), 0, DB_BTREE, DB_CREATE | DB_TRUNCATE, 0);
    emit started(QString("Запись в файл\n%1").arg(pathname), 1);

    putVal<double>(db, "parameters.initialSkip", scan.parameters.initialSkip);
    putVal<double>(db, "parameters.stepForSplitFrequencyRanges", scan.parameters.stepForSplitFrequencyRanges);
    putVal<double>(db, "parameters.peakMagnitudeLimit", scan.parameters.peakMagnitudeLimit);
    putVal<double>(db, "parameters.peakBackstep", scan.parameters.peakBackstep);
    putVal<double>(db, "parameters.peakForestep", scan.parameters.peakForestep);
    putVal<unsigned int>(db, "parameters.peakPauseCount", scan.parameters.peakPauseCount);
    //putVal<unsigned int>(db, "parameters.peakPauseCount", scan.parameters.countOfPeakToCatchForAreaBox);

    //putVal<double>(db, "parameters.peakPauseCount", scan.parameters.peakPauseCount);
    
    putVal<std::uint32_t>(db, "parameters.ranges.@size", scan.parameters.ranges.size());    
    for (std::size_t i = 0; i < scan.parameters.ranges.size(); i++) {
      putVal<double>(db, (boost::format("parameters.ranges.@%1%.from") % i).str(), scan.parameters.ranges[i].from);
      putVal<double>(db, (boost::format("parameters.ranges.@%1%.to") % i).str(), scan.parameters.ranges[i].to);
    }

    putVal<std::uint32_t>(db, "parameters.defectPoints.@size", scan.parameters.defectPoints.size());
    for (std::size_t i = 0; i < scan.parameters.defectPoints.size(); i++) {
      putStr(db, (boost::format("parameters.defectPoints.@%1%.title") % i).str(), scan.parameters.defectPoints[i].title);
      putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.red.range") % i).str(), scan.parameters.defectPoints[i].red.range);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.limit") % i).str(), scan.parameters.defectPoints[i].red.limit);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.amplification") % i).str(), scan.parameters.defectPoints[i].red.amplification);
      putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.blue.range") % i).str(), scan.parameters.defectPoints[i].blue.range);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.limit") % i).str(), scan.parameters.defectPoints[i].blue.limit);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.amplification") % i).str(), scan.parameters.defectPoints[i].blue.amplification);
      putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.green.range") % i).str(), scan.parameters.defectPoints[i].green.range);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.limit") % i).str(), scan.parameters.defectPoints[i].green.limit);
      putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.amplification") % i).str(), scan.parameters.defectPoints[i].green.amplification);
    }

    putVal<unsigned>(db, "parameters.columnModelOrder", scan.parameters.columnModelOrder);

    putVal<std::uint32_t>(db, "lines.@size", scan.lines.size());

    emit stageStarted("Запись строк", scan.lines.size());
    for (std::size_t i = 0; i < scan.lines.size(); i++) {
      putVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@format") % i).str(), static_cast<std::uint32_t>(encoding));
      switch (encoding) {
      case LineEncoding::Float:
        putVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@size") % i).str(), scan.lines[i].samples.size());
        putVec<float>(db, (boost::format("lines.@%1%.samples") % i).str(), scan.lines[i].samples);
        break;
      case LineEncoding::Int16:
        {
          std::vector<std::int16_t> compressedSamples;
          compressedSamples.reserve(scan.lines[i].samples.size());
          for (auto x : scan.lines[i].samples) compressedSamples.push_back(x * 0x8000);
          putVec<std::int16_t>(db, (boost::format("lines.@%1%.samples") % i).str(), compressedSamples);
          putVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@size") % i).str(), scan.lines[i].samples.size());
        }
        break;
      case LineEncoding::WindowsMediaLossless:
        {
          WMACompressedData data = wmaLosslessCompress(scan.lines[i].samples, scan.lines[i].sampleRate);
          putVec<std::uint8_t>(db, (boost::format("lines.@%1%.samples") % i).str(), data.compressedSamples);
          putVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@byteSize") % i).str(), data.compressedSamples.size());
          putVec<std::uint8_t>(db, (boost::format("lines.@%1%.samples.@serializedAttributes") % i).str(), data.serializedAttributes);
          putVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@serializedAttributes.@byteSize") % i).str(), data.serializedAttributes.size());
        }
        break;
      }
      
      putVal<std::uint32_t>(db, (boost::format("lines.@%1%.sampleRate") % i).str(), scan.lines[i].sampleRate);
      putVal<double>(db, (boost::format("lines.@%1%.startCoordinate") % i).str(), scan.lines[i].startCoordinate);
      putVal<double>(db, (boost::format("lines.@%1%.finalCoordinate") % i).str(), scan.lines[i].finalCoordinate);
      putVal<double>(db, (boost::format("lines.@%1%.lineCoordinate") % i).str(), scan.lines[i].lineCoordinate);

      emit stageProgressed();
    }

    db.close(0);
    emit finished();
  } catch (...) {
    db.close(0);
    throw;
  }

}
