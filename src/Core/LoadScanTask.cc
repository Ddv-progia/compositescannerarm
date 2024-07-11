/*
 * Core/LoadScanTask.cc
 */

#include <cstdint>
#include <boost/format.hpp>
#include <db_cxx.h>

#include "Core/LineEncoding.hh"
#include "Core/LoadScanTask.hh"
#include "Core/WMACompression.hh"

namespace {

  template<typename T>
  T getVal(Db& db, const std::string& name)
  {
    T result;

    Dbt key((void*)name.c_str(), name.size());
    Dbt value;
    value.set_data(&result);
    value.set_ulen(sizeof(T));
    value.set_flags(DB_DBT_USERMEM);

    db.get(0, &key, &value, 0);

    return result;
  }

  template<typename T>
  T getVal(Db& db, const std::string& name, T defaultValue)
  {
    T result;

    Dbt key((void*)name.c_str(), name.size());
    Dbt value;
    value.set_data(&result);
    value.set_ulen(sizeof(T));
    value.set_flags(DB_DBT_USERMEM);

    if (db.get(0, &key, &value, 0) != DB_NOTFOUND) {
      return result;
    } else {
      return defaultValue;
    }
  }

  std::string getStr(Db& db, const std::string& name)
  {
    Dbt key((void*)name.c_str(), name.size());
    Dbt value;
    db.get(0, &key, &value, 0);

    return std::string(static_cast<char*>(value.get_data()), value.get_size());
  }

  template<typename T>
  void getVec(Db& db, const std::string& name, std::vector<T>& x)
  {
    Dbt key((void*)name.c_str(), name.size());
    Dbt value;
    value.set_data((void*)x.data());
    value.set_ulen(x.size() * sizeof(T));
    value.set_flags(DB_DBT_USERMEM);
    db.get(0, &key, &value, 0);
  }

}

LoadScanTask::LoadScanTask(const QString& pathname, ScanFactory& scanFactory, const ProcessingParameters& parameters, bool ignoreSavedParameters)
  : pathname(pathname), scanFactory(scanFactory), parameters(parameters), ignoreSavedParameters(ignoreSavedParameters)
{ }

void LoadScanTask::operator()()
{
  Db db(0, 0);
  try {
    db.open(0, pathname.toUtf8().data(), 0, DB_BTREE, 0, 0);

    emit started(QString("Чтение из файла\n%1").arg(pathname), 1);

    if (! ignoreSavedParameters) {
      parameters.initialSkip = getVal<double>(db, "parameters.initialSkip");
      parameters.peakMagnitudeLimit = getVal<double>(db, "parameters.peakMagnitudeLimit");
      parameters.peakBackstep = getVal<double>(db, "parameters.peakBackstep");
      parameters.peakForestep = getVal<double>(db, "parameters.peakForestep");
      parameters.peakPauseCount= getVal<unsigned int>(db, "parameters.peakPauseCount", 1000);
      //parameters.peakPauseCount= getVal<double>(db, "parameters.peakPauseCount", 1000);
      // parameters.peakPauseCount - экспериментальное значение. Для определения ширины пика.
      // (см. ScanAlgorythm) Использование закомментировано.

      auto rangesSize = getVal<std::uint32_t>(db, "parameters.ranges.@size");
      parameters.ranges.resize(rangesSize);
      for (std::size_t i = 0; i < parameters.ranges.size(); i++) {
        parameters.ranges[i].from = getVal<double>(db, (boost::format("parameters.ranges.@%1%.from") % i).str());
        parameters.ranges[i].to = getVal<double>(db, (boost::format("parameters.ranges.@%1%.to") % i).str());
      }

      auto defectPointsSize = getVal<std::uint32_t>(db, "parameters.defectPoints.@size");
      parameters.defectPoints.resize(defectPointsSize);
      for (std::size_t i = 0; i < parameters.defectPoints.size(); i++) {
        parameters.defectPoints[i].title = getStr(db, (boost::format("parameters.defectPoints.@%1%.title") % i).str());
        parameters.defectPoints[i].red.range = getVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.red.range") % i).str());
        parameters.defectPoints[i].red.limit = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.limit") % i).str());
        parameters.defectPoints[i].red.amplification = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.amplification") % i).str());
        parameters.defectPoints[i].blue.range = getVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.blue.range") % i).str());
        parameters.defectPoints[i].blue.limit = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.limit") % i).str());
        parameters.defectPoints[i].blue.amplification = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.amplification") % i).str());
        parameters.defectPoints[i].green.range = getVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.green.range") % i).str());
        parameters.defectPoints[i].green.limit = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.limit") % i).str());
        parameters.defectPoints[i].green.amplification = getVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.amplification") % i).str());
      }

      parameters.columnModelOrder = getVal<unsigned>(db, "parameters.columnModelOrder");
    }
    scanFactory.startNewScan(parameters);

    auto linesSize = getVal<std::uint32_t>(db, "lines.@size");
    emit stageStarted("Чтение строк", linesSize);
    for (std::uint32_t i = 0; i < linesSize; i++) loadLine(db,i);
    
    emit finished();
    scanFactory.finishScan(boost::optional<QString&>(pathname));

    db.close(0);
  } catch (...) {
    db.close(0);
    throw;
  }
}

void LoadScanTask::loadLine(Db& db, std::uint32_t i)
{
  SourceScanLine line;

  line.sampleRate = getVal<std::uint32_t>(db, (boost::format("lines.@%1%.sampleRate") % i).str());
  line.startCoordinate = getVal<double>(db, (boost::format("lines.@%1%.startCoordinate") % i).str());
  line.finalCoordinate = getVal<double>(db, (boost::format("lines.@%1%.finalCoordinate") % i).str());
  line.lineCoordinate = getVal<double>(db, (boost::format("lines.@%1%.lineCoordinate") % i).str(), -1.0);

  if (line.lineCoordinate == -1.0) line.lineCoordinate = 0.0;

  auto lineSize = getVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@size") % i).str());
  auto lineFormat = static_cast<LineEncoding>(getVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@format") % i).str(), 
                                                                    static_cast<std::uint32_t>(LineEncoding::Float)));

  switch (lineFormat) {
  case LineEncoding::Float:
    line.samples.resize(lineSize);
    getVec<float>(db, (boost::format("lines.@%1%.samples") % i).str(), line.samples);
    break;
  case LineEncoding::Int16:
    {
      std::vector<std::int16_t> compressedSamples(lineSize);
      getVec<std::int16_t>(db, (boost::format("lines.@%1%.samples") % i).str(), compressedSamples);
      for (auto x : compressedSamples) line.samples.push_back(float(x) / float(0x8000));
    }
    break;
  case LineEncoding::WindowsMediaLossless: 
    {
      auto compressedSize = getVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@byteSize") % i).str());
      auto attributesSize = getVal<std::uint32_t>(db, (boost::format("lines.@%1%.samples.@serializedAttributes.@byteSize") % i).str());

      std::vector<std::uint8_t> compressedSamples(compressedSize);
      getVec<std::uint8_t>(db, (boost::format("lines.@%1%.samples") % i).str(), compressedSamples);

      std::vector<std::uint8_t> serializedAttributes(attributesSize);          
      getVec<std::uint8_t>(db, (boost::format("lines.@%1%.samples.@serializedAttributes") % i).str(), serializedAttributes);
        
      line.samples = wmaLosslessDecompress(WMACompressedData(std::move(compressedSamples), std::move(serializedAttributes)), line.sampleRate);
    }
  }
  //*******
  // //отбрасываем ненужные строки
  //if ((i > 7) && (i < 17)) {
  //    scanFactory.addRangeScanLine(line);
  //}
  ////*******
  scanFactory.addRangeScanLine(line);
  emit stageProgressed();
}
