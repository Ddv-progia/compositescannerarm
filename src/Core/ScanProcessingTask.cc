/*
 * Core/ScanProcessingTask.cc
 */

#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/range/empty.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm_ext/iota.hpp>
#include <boost/range/algorithm/lower_bound.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/numeric.hpp>
#include <UCL/Iteration/Variadic/Transform.hh>
#include <UCL/RegressionAnalysis/LeastSquares.hh>

#include "Core/ScanAlgorithms.hh"
#include "Core/ScanDefectRenderingTask.hh"
#include "Core/ScanProcessingTask.hh"
#include "DevTalk/Core/Exception.hh"

namespace adp = boost::adaptors;
namespace ba  = boost::accumulators;
namespace p   = boost::phoenix;
namespace pa  = boost::phoenix::arg_names;
namespace v   = uts::iteration::variadic;

ScanProcessingTask::ScanProcessingTask(const std::vector<SourceScanLine>& rawLines, const ProcessingParameters& params,std::shared_ptr<Scan>& newScan)
  : rawLines(rawLines), params(params), scan(newScan)
{ }

SourceScanLineSlice ScanProcessingTask::trimLine(const SourceScanLine& line, double initialSkip)
{
  SourceScanLineSlice trimmedLine;
  trimmedLine.startCoordinate = line.startCoordinate;
  trimmedLine.finalCoordinate = line.finalCoordinate;
  trimmedLine.sampleRate = line.sampleRate;
  trimmedLine.lineCoordinate = line.lineCoordinate;

  std::size_t offset = static_cast<std::size_t>(std::floor(initialSkip * line.sampleRate + 0.5));
  trimmedLine.samplesBegin = line.samples.begin() + offset;
  trimmedLine.samplesEnd = line.samples.end();

  emit stageProgressed();
  return trimmedLine;
}

//PeaksLine ScanProcessingTask::findPeaks(const SourceScanLineSlice& line, float peakLimit, double backstep, double forestep, unsigned int pauseCount)
PeaksLine ScanProcessingTask::findPeaks(const SourceScanLineSlice& line, float peakLimit, double backstep, double forestep, double pauseCount)
{
  return PeaksLine{ ::findPeaks(line.samplesBegin, line.samplesEnd, line.sampleRate, peakLimit, backstep, forestep, pauseCount),
                    line.startCoordinate,
                    line.finalCoordinate,
                    line.lineCoordinate };
}

void ScanProcessingTask::normalizeDirection(PeaksLine& line)
{
  if (line.finalCoordinate < line.startCoordinate) {
    line = PeaksLine{ std::vector<Peak>(line.peaks.rbegin(), line.peaks.rend()),
                      line.finalCoordinate,
                      line.startCoordinate,
                      line.lineCoordinate };
  }

  emit stageProgressed();
}

std::vector<RangeScanLine> ScanProcessingTask::splitFrequencyRanges(const SourceScanLineSlice& line, const std::vector<Peak>& peaks)
{
  std::vector<RangeScanLine> r;

  if ((line.samplesBegin != line.samplesEnd) && ! peaks.empty()) {
    //auto bands = ::splitFrequencyRanges(line.samplesBegin, line.samplesEnd, line.sampleRate, peaks, params.ranges,params.shouldNormalize);

    //находим полный спектр с дискретизацией 250 Гц

    std::vector<FrequencyRange> ranges;
    
    auto step = params.stepForSplitFrequencyRanges;
    //auto step = 1000; 
    //auto step = 250;
    //auto step = 500;
    unsigned int maxFrequency = line.sampleRate / 2 - step;

    //for(auto startFrequency = 0;startFrequency<=47000;startFrequency+=step)
    for(auto startFrequency = 0; startFrequency<= maxFrequency; startFrequency+=step)
        ranges.push_back(FrequencyRange{ (double)startFrequency, (double)startFrequency + step });
    auto bands = ::splitFrequencyRanges(line.samplesBegin, line.samplesEnd, line.sampleRate, peaks, ranges);
    //

    r.resize(bands.size());
    for (std::size_t i = 0; i < bands.size(); i++) {
      r[i].sampleRate = line.sampleRate;
      r[i].startCoordinate = line.startCoordinate;
      r[i].finalCoordinate = line.finalCoordinate;
      r[i].samples = std::move(bands[i]);
      boost::transform(peaks, std::back_inserter(r[i].sampleIndexes), std::mem_fn(&Peak::beginIndex));
      r[i].range = ranges[i];
      r[i].sourceLineSize = std::distance(line.samplesBegin, line.samplesEnd);
      r[i].lineCoordinate = line.lineCoordinate;
    }
  }

  emit stageProgressed();
  return std::move(r);
}

void ScanProcessingTask::alignLines(std::vector<PeaksLine>& peaks,std::vector<std::vector<RangeScanLine>>& ranges)
{
  auto realPeaks = peaks 
    | adp::filtered([](const PeaksLine& l) { return ! l.peaks.empty(); })
    | adp::transformed([] (const PeaksLine& l) { return l.peaks; })
    ;

  auto forwardPeaks = realPeaks 
    | adp::filtered([] (const std::vector<Peak>& peaks) { return peaks.back().beginIndex > peaks.front().beginIndex; })
    ;

  auto backwardPeaks = realPeaks 
    | adp::filtered([] (const std::vector<Peak>& peaks) { return peaks.back().beginIndex < peaks.front().beginIndex; })
    ;

  if (! boost::empty(forwardPeaks) && ! boost::empty(backwardPeaks)) {
    ba::accumulator_set<std::size_t, ba::stats<ba::tag::mean>> forwardAcc;
    ba::accumulator_set<std::size_t, ba::stats<ba::tag::mean>> backwardAcc;

    for (const auto& fp : forwardPeaks) forwardAcc(fp.front().beginIndex);
    for (const auto& bp : backwardPeaks) backwardAcc(bp.front().beginIndex);

    auto distance = ba::mean(backwardAcc) + ba::mean(forwardAcc);

    for (auto& line : ranges) {
      if (! line.empty() && (line.front().sampleIndexes.front() > line.front().sampleIndexes.back()))
        for (auto& range : line)
          for (auto& si : range.sampleIndexes)
            si = distance - si;
      emit stageProgressed();
    }
  }
}

std::tuple<std::size_t, std::size_t, std::size_t> ScanProcessingTask::getNormalizedIndexes(Scan& scan) const
{
  ba::accumulator_set<std::size_t, ba::stats<ba::tag::mean>> distanceAcc;
  ba::accumulator_set<std::size_t, ba::stats<ba::tag::mean>> startIndexAcc;
  ba::accumulator_set<std::size_t, ba::stats<ba::tag::mean>> stopIndexAcc;

  for (auto& line : scan.ranges) {
    if (line.empty())
      break;
    if (! line.front().samples.empty()) {
      std::vector<std::size_t> lineDistances;
      boost::adjacent_difference(line.front().sampleIndexes, std::back_inserter(lineDistances));
      distanceAcc(*boost::min_element(lineDistances));
      startIndexAcc(line.front().sampleIndexes.front());
      stopIndexAcc(line.front().sampleIndexes.back());
    }
  }
  auto step = ba::mean(distanceAcc)/2;
  auto start = ba::mean(startIndexAcc);
  auto stop = ba::mean(stopIndexAcc);
  return std::make_tuple(step,start,stop);
}

void ScanProcessingTask::normalizeRanges(Scan& scan)
{
  std::size_t step;
  std::size_t startIndex;
  std::size_t stopIndex;
  std::tie(step, startIndex, stopIndex) = getNormalizedIndexes(scan);
  if (step == 0 && startIndex == 0 && stopIndex == 0) return;
  //if (step == 0) step=1000; //******* TODO разобраться с вылетом при step=0
  if (step == 0) step= scan.parameters.stepForSplitFrequencyRanges; //******* TODO разобраться с вылетом при step=0
  auto rangesCount = scan.parameters.ranges.size();
  std::size_t commonRangesCount = 0;
  if (scan.commonRanges.size()>0) {
    commonRangesCount = scan.commonRanges.front().size();
  }
  scan.normalizedRanges.resize(rangesCount);
  scan.commonNormalizedRanges.resize(commonRangesCount);
  auto extremum = ::Extremum::Max;
  for (std::size_t rangeIndex = 0; rangeIndex < rangesCount; rangeIndex++) {
    //params.extremumOfRanges[rangeIndex];
    if (rangeIndex < params.extremumOfRanges.size())
        extremum = params.extremumOfRanges[rangeIndex];
    else
        extremum = ::Extremum::Max;
    normalizeRange(scan.normalizedRanges[rangeIndex],scan.ranges, rangeIndex, step, startIndex, stopIndex, extremum);
    emit stageProgressed();
  }
  extremum = ::Extremum::Max;
  for(std::size_t rangeIndex2 = 0; rangeIndex2 < commonRangesCount; rangeIndex2++){
    normalizeRange(scan.commonNormalizedRanges[rangeIndex2],scan.commonRanges, rangeIndex2, step, startIndex, stopIndex, extremum);
    emit stageProgressed();
  }
}


void ScanProcessingTask::normalizeRange(NormalizedRange& normalizedRange,
                                        std::vector<std::vector<RangeScanLine>>& ranges,
                                        std::size_t rangeIndex, 
                                        std::size_t step, 
                                        std::size_t startIndex, 
                                        std::size_t stopIndex,
                                        ::Extremum extremumOfRangesIn)
{
  auto lineLength = (stopIndex - startIndex) / step;
  auto linesCount = ranges.size();
  auto be = boost::extents[lineLength][linesCount];
  normalizedRange.maxView.resize(be);
  normalizedRange.minView.resize(be);
  normalizedRange.view.resize(be);
  normalizedRange.averView.resize(be); //*******
  normalizedRange.diffView.resize(be); //*******
  if (rangeIndex >= ranges[0].size()) return;
  normalizedRange.max = getNormalizedPeakAt(ranges[0][rangeIndex], startIndex);
  normalizedRange.min = normalizedRange.max;
  normalizedRange.aver = normalizedRange.max; //*******
  normalizedRange.diff = normalizedRange.max; //*******
  namespace ba = boost::accumulators;
  ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::max, ba::tag::min>> accDiff;

  for (std::size_t lineIndex = 0; lineIndex < linesCount; lineIndex++) {
      if (rangeIndex >= ranges[lineIndex].size()) return;

      for (std::size_t peakIndex = 0; peakIndex < lineLength; peakIndex++) {
          float maxPeak = 0.0;
          float minPeak = 0.0;
          float justPeak = 0.0; //*******
          float diffPeak = 0.0; //*******
          if (ranges[lineIndex][rangeIndex].samples.size() > 0) {
              if (scan->parameters.useSubRanges) {
                  //*******
                  //auto peaks = getNormalizedPeakFromSubranges(ranges[lineIndex][rangeIndex], startIndex + peakIndex * step);
                  std::tie(minPeak, maxPeak, justPeak) = getNormalizedPeakFromSubranges(ranges[lineIndex][rangeIndex], startIndex + peakIndex * step);
                  diffPeak = maxPeak - minPeak - 1; //******* Преобразуем разницу по заданию Сергея Ивановича begin
                  //diffPeak = maxPeak - minPeak;


                  //******* Преобразуем среднее по заданию Сергея Ивановича begin
                  int N = 0;
                  for (auto iter = ranges[lineIndex][rangeIndex].subBegin; iter < ranges[lineIndex][rangeIndex].subEnd; iter++) {
                      N++;
                  }
                  justPeak = justPeak * std::sqrt(N);
                  //******* Преобразуем среднее по заданию Сергея Ивановича end

                  //diffPeak = justPeak - (maxPeak - minPeak - 1); //******* Преобразуем разницу 18/07/2024

                  //minPeak = peaks.first;
                  //maxPeak = peaks.second;
                  //*******
              }
              else {
                  maxPeak = getNormalizedPeakAt(ranges[lineIndex][rangeIndex], startIndex + peakIndex * step);
                  minPeak = maxPeak;
                  justPeak = maxPeak; //*******
                  diffPeak = maxPeak; //*******
              }
              acc(justPeak); //******* 
              accDiff(diffPeak); //******* 
              if (maxPeak > normalizedRange.max) normalizedRange.max = maxPeak;
              if (minPeak < normalizedRange.min) normalizedRange.min = minPeak;
          }
              normalizedRange.maxView[peakIndex][lineIndex] = maxPeak;
              normalizedRange.minView[peakIndex][lineIndex] = minPeak;
              normalizedRange.averView[peakIndex][lineIndex] = justPeak; //*******
              normalizedRange.diffView[peakIndex][lineIndex] = diffPeak; //*******
              //normalizedRange.aver = justPeak; //*******
          
      }
  }
  auto average = ba::mean(acc);
  normalizedRange.aver = average; //******* среднее по всем средним
  //normalizedRange.diff = ba::mean(accDiff); //******* среднее по всем разницам
  //normalizedRange.diff = ba::max(accDiff); //******* максимум по всем разницам
  normalizedRange.diff = (ba::max(accDiff)+ ba::mean(accDiff))/2; //*******  //TODO проверить (ba::max(accDiff)+ ba::min(accDiff))/2

  normalizedRange.sampleRate = ranges.front().front().sampleRate / step;
  normalizedRange.startCoordinate = ranges.front().front().startCoordinate;
  normalizedRange.finalCoordinate = ranges.front().front().finalCoordinate;
  normalizedRange.beginIndex = startIndex;
  normalizedRange.endIndex = stopIndex;
  normalizedRange.step = step;
  normalizedRange.extremum = extremumOfRangesIn;
  

  normalizedRange.lineCoordinates.resize(linesCount);
  if (linesCount > 0) {
    //if (ranges[0][rangeIndex].lineCoordinate < 0) {
    //  normalizedRange.lineCoordinates[0] = 0;
    //} else {
    //  normalizedRange.lineCoordinates[0] = ranges[0][rangeIndex].lineCoordinate;
    //}

    for (std::size_t lineIndex = 0; lineIndex < linesCount; lineIndex++) //{
     // if (ranges[lineIndex][rangeIndex].lineCoordinate < 0) {
     //   normalizedRange.lineCoordinates[lineIndex] = normalizedRange.lineCoordinates[lineIndex - 1] + 1;
     // } else {
        normalizedRange.lineCoordinates[lineIndex] = ranges[lineIndex][rangeIndex].lineCoordinate;
     // }
    //}
  }
  if (normalizedRange.extremum == ::Extremum::Max) {
      normalizedRange.view = normalizedRange.maxView;
    }
  else if (normalizedRange.extremum == ::Extremum::Min) {
      normalizedRange.view = normalizedRange.minView;
  }  else if (normalizedRange.extremum == ::Extremum::Diff) {
      normalizedRange.view = normalizedRange.diffView;
  }
  else normalizedRange.view = normalizedRange.averView;
}

float ScanProcessingTask::getNormalizedPeakAt(const RangeScanLine& line, std::size_t idx)
{
    if ((line.sampleIndexes.size() > 0) && (line.samples.size()>0)) {
        auto iidx = boost::lower_bound(line.sampleIndexes, idx);

        if ((iidx != line.sampleIndexes.end()) && (*iidx == idx)) {
            auto idx = std::distance(line.sampleIndexes.begin(), iidx);
            return line.samples[idx];
        }
        else if (iidx != line.sampleIndexes.end()) {
            auto idx = std::distance(line.sampleIndexes.begin(), iidx);
            return line.samples[idx > 0 ? idx : 0];
        }
        else {
            return 0.0;
        }
    }
    else {
        return 0.0;
    }

}

float ScanProcessingTask::getAverageSubrangePeak(const RangeScanLine& line, std::size_t idx)
{
  namespace ba = boost::accumulators;
  ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
  ba::mean(acc);

  float result = line.subBegin->samples[idx];
  for(auto iter = line.subBegin; iter< line.subEnd; iter++){
      acc(iter->samples[idx]);
  }
  result = ba::mean(acc);
  return result;
}

float ScanProcessingTask::getMaxSubrangePeak(const RangeScanLine& line,std::size_t idx)
{
  float result = line.subBegin->samples[idx];
  for(auto iter = line.subBegin; iter< line.subEnd; iter++){
    if(iter->samples[idx]>result)
      result = iter->samples[idx];
  }
  return result;
}

float ScanProcessingTask::getMinSubrangePeak(const RangeScanLine& line,std::size_t idx)
{
  float result = line.subBegin->samples[idx];
  for(auto iter = line.subBegin; iter< line.subEnd; iter++){
    if(iter->samples[idx]<result)
      result = iter->samples[idx];
  }
  return result;
}

std::tuple<float, float, float> ScanProcessingTask::getNormalizedPeakFromSubranges(const RangeScanLine& line, std::size_t idx)
//std::pair<float,float> ScanProcessingTask::getNormalizedPeakFromSubranges(const RangeScanLine& line, std::size_t idx)
{
  auto iidx = boost::lower_bound(line.sampleIndexes, idx);
  float min=0.0;
  float max=0.0;
  float aver=0.0;

  if ((iidx != line.sampleIndexes.end()) && (*iidx == idx)) {
    auto idx = std::distance(line.sampleIndexes.begin(), iidx);
    max = getMaxSubrangePeak(line,idx);
    min = getMinSubrangePeak(line,idx);
    aver = getAverageSubrangePeak(line,idx);
  } else if (iidx != line.sampleIndexes.end()) {
    auto idx2 = std::distance(line.sampleIndexes.begin(), iidx);
    max = getMaxSubrangePeak(line,idx2 > 0 ? idx2 : 0);
    min = getMinSubrangePeak(line,idx2 > 0 ? idx2 : 0);
    aver = getAverageSubrangePeak(line,idx2 > 0 ? idx2 : 0);
  }
 
  return std::make_tuple(min, max, aver);
  //return std::make_pair(min, max);
}

Polynomial ScanProcessingTask::signleRangeModel(const std::vector<double>& column, 
                                                const std::vector<double>& lineCoordinates, 
                                                unsigned int order)
{
  std::vector<double> xs;
  std::vector<double> ys;
  for (std::size_t i = 0; i < column.size(); i++) {
    if (! ((! (column[i] > -std::numeric_limits<double>::infinity())) && (! (column[i] < -std::numeric_limits<double>::infinity())))) {
      xs.push_back(lineCoordinates[i]);
      ys.push_back(column[i]);
    }
  }
  return Polynomial{ uts::regression::leastSquares<std::vector<double>>(xs, ys, order) };
}

std::vector<Polynomial> ScanProcessingTask::rangeColumnModels(const std::vector<std::vector<double>>& averageColumns, 
                                                              const std::vector<double>& lineCoordinates,
                                                              unsigned int order)
{
  std::vector<Polynomial> r;
  for (auto const& column : averageColumns) {
    r.push_back(signleRangeModel(column, lineCoordinates, order));
  }

  emit stageProgressed();
  return r;
}

std::vector<std::vector<double>> ScanProcessingTask::averageColumns(Scan& scan)
{
  typedef boost::multi_array<float, 2>::index_range idxrng;

  auto rangeCount = scan.normalizedRanges.size();
  std::vector<std::vector<double>> r(rangeCount);
  
  for (std::size_t rangeIndex = 0; rangeIndex < scan.normalizedRanges.size(); rangeIndex++) {
    auto rowCount = scan.normalizedRanges[rangeIndex].view.shape()[1];
    r[rangeIndex].resize(rowCount);
    for (std::size_t rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      r[rangeIndex][rowIndex] = ba::mean(boost::for_each(scan.normalizedRanges[rangeIndex].view[boost::indices[idxrng()][rowIndex]], 
                                                         ba::accumulator_set<float, ba::features<ba::tag::mean>>()));
    }

    emit stageProgressed();
  }

  return r;
}

void ScanProcessingTask::subtractColumnModels(Scan& scan, const std::vector<Polynomial>& models)
{
  auto evalPoly = [] (const Polynomial& poly, double x) -> double {
    double y = 0.0;
    for (std::size_t i = 0; i < poly.coefficients.size(); i++)
      y += std::pow(x, i) * poly.coefficients[i];
    return y;
  };

  scan.rangesResiduals.resize(scan.normalizedRanges.size());
  for (std::size_t rangeIndex = 0; rangeIndex < scan.normalizedRanges.size(); rangeIndex++) {
    auto rangeShape = scan.normalizedRanges[rangeIndex].view.shape();
    scan.rangesResiduals[rangeIndex].view.resize(boost::extents[rangeShape[0]][rangeShape[1]]);

    for (std::size_t j = 0; j < scan.normalizedRanges[rangeIndex].view.shape()[1]; j++) {
      for (std::size_t i = 0; i < scan.normalizedRanges[rangeIndex].view.shape()[0]; i++) {
        scan.rangesResiduals[rangeIndex].view[i][j] = 
          scan.normalizedRanges[rangeIndex].view[i][j] - 
          evalPoly(models[rangeIndex], j);
      }
    }

    scan.rangesResiduals[rangeIndex].sampleRate = scan.normalizedRanges[rangeIndex].sampleRate;
    scan.rangesResiduals[rangeIndex].startCoordinate = scan.normalizedRanges[rangeIndex].startCoordinate;
    scan.rangesResiduals[rangeIndex].finalCoordinate = scan.normalizedRanges[rangeIndex].finalCoordinate;
    scan.rangesResiduals[rangeIndex].lineCoordinates = scan.normalizedRanges[rangeIndex].lineCoordinates;
    emit stageProgressed();
  }
}

void ScanProcessingTask::smoothRanges(Scan& scan)
{
  int pointsCount = scan.parameters.smoothingPointsCount/2+1;
  if(pointsCount<2)
    return;

  for(auto& line : scan.normalizedSpec){
    for(auto& range : line){
      auto point = range.samples.begin();
      std::vector<float> result;

      if(range.samples.size()>=pointsCount){
        for(;point<range.samples.end();point++){
	  ba::accumulator_set<float,ba::stats<ba::tag::mean>> Acc;
	  for(int nP = pointsCount ; nP>0; nP--){
            Acc(*point);                        //центральная точка имеет наибольший вклад
            auto begin = point;
            auto end = point;

            for(signed int i = 1;i<nP;i++){	//вклад остальных точек зависит от удаленности от центральной
              if(begin>range.samples.begin()){   //проверка на выход точек за заданный диапазон
                --begin;
                Acc(*begin);
              }
              else if(begin!=point && begin == range.samples.begin())
                Acc(*begin);

              if(end<range.samples.end()-1){
                ++end;
                Acc(*end);
              }
              else if(end!=point && (end == range.samples.end()-1))
                Acc(*end);
	    }
	  }
          result.push_back(ba::mean(Acc));
        }
        range.samples = result;
      }
    }
    emit stageProgressed();
  }
}

RangeScanLine ScanProcessingTask::findAverageLine(std::vector<RangeScanLine>& rangedLines,FrequencyRange range)
{
  RangeScanLine result;
  if (rangedLines.size() > 0) {
      result = *rangedLines.begin();
      result.subBegin = rangedLines.begin();
      result.subEnd = rangedLines.end();

      for (auto iter = rangedLines.begin(); iter < rangedLines.end(); iter++) {
          if (iter->range.from == rangedLines.front().range.from && iter->range.to == rangedLines.back().range.to) continue;
          if (floor(iter->range.from / 500) == floor(range.from / 500))  //TODO 500- magic number?
              result.subBegin = iter;
          if (floor(iter->range.to / 500) == floor(range.to / 500))
              result.subEnd = iter + 1;
      }

      std::vector<ba::accumulator_set<float, ba::stats<ba::tag::mean>>> accs;
      auto size = result.subBegin->samples.size();
      accs.resize(size);

      for (auto i = 0; i < size; i++) {
          for (auto iter = result.subBegin; iter < result.subEnd; iter++) {
              float sample = iter->samples[i];
              if (sample != 0)
                  accs[i](iter->samples[i]);
          }
      }
      result.samples.clear();

      for (auto& acc : accs)
          result.samples.push_back(ba::mean(acc));
  }
  result.range = range;
  return result;
}



void ScanProcessingTask::selectRangesFromSpec(Scan& scan)
{
  auto commonRanges = constructCommonRanges();
  scan.ranges.clear();
  scan.commonRanges.clear();
  for(auto& line : scan.normalizedSpec){
    std::vector<RangeScanLine> rangedLine;
    std::vector<RangeScanLine> commonRangedLine;
    for(auto& range : scan.parameters.ranges) {
      rangedLine.push_back(findAverageLine(line,range));
    }
    //*******
    //if (scan.parameters.shouldNormalize) {
    //    auto normLine = line.back();
    //    //normLine.subBegin = line.end() - 1; //было так. Непонятно зачем. 
    //    //normLine.subEnd = line.end();       //
    //    //rangedLine.push_back(normLine);     //последняя линия - сумма нормирования 
    //        //normLine.subBegin = line.begin(); // так работает //******* 
    //        //normLine.subEnd = line.end();     // так работает //******* 
    //        //rangedLine.push_back(normLine);   // так работает //******* 
    //    rangedLine.push_back(findAverageLine(line, normLine.range));  // сделали так - усредняем по всему диапазону //******* 
    //}
    //*******
    scan.ranges.push_back(rangedLine);
    for(auto& range : commonRanges){
      commonRangedLine.push_back(findAverageLine(line,range));
    }
	if((scan.parameters.shouldNormalize)&&(line.size()>0)){
		auto normLine = line.back();
		////normLine.subBegin = line.end()-1; //было так. Непонятно зачем. 
		////normLine.subEnd = line.end();     //было так. Непонятно зачем. 
		normLine.subBegin = line.begin(); //так работает //******* 
		normLine.subEnd = line.end();     //так работает //******* 
		//commonRangedLine.push_back(normLine); //последняя линия - сумма нормирования
        commonRangedLine.push_back(findAverageLine(line, normLine.range)); // сделали так - усредняем по всему диапазону //******* 
    }
    scan.commonRanges.push_back(commonRangedLine);
  }
}

void ScanProcessingTask::findRelativeSignals(Scan& scan)
{ 
  auto nRanges = scan.spec.begin()->size();
  scan.normalizedSpec = scan.spec;
  for(int nRange = 0;nRange<nRanges;nRange++){
    ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::variance>> acc;
    for(auto& line : scan.spec)
      for(auto& point : line[nRange].samples)
        acc(point);

    auto sigma = std::sqrt(ba::variance(acc));
    auto average = ba::mean(acc);

    for(auto& line:scan.normalizedSpec)
      for(auto& point : line[nRange].samples)
          point = (point - average) / (3 * sigma);

    stageProgressed();
  }
}

void ScanProcessingTask::findRelativeSignals(Scan& scan,const std::vector<SpecNormalizationParams>& params)
{
  auto nRanges = scan.spec.begin()->size();
  scan.normalizedSpec = scan.spec;
  for(int nRange = 0;nRange<nRanges;nRange++){
	auto sigma = params[nRange].sigma;
	auto average = params[nRange].average;

    for(auto& line:scan.normalizedSpec){
        if (line.size() > nRange) {
            for (auto& point : line[nRange].samples)
                point = (point - average) / (3 * sigma);
        }
	}
    stageProgressed();
  }
}

void ScanProcessingTask::normalizeSpectrogram(std::vector<std::vector<RangeScanLine>>& spec)
{
  for(auto& line:spec){
    if (line.size() == 0) {
        //BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Description("Ошибка нормализации спектрограммы: число полученных линий в спектре равно 0"));
    }
    else {
        std::vector<float> sums;
        sums.resize(line.begin()->samples.size());
        for (auto sampleNum = 0; sampleNum < line.begin()->samples.size(); sampleNum++) {
            for (auto& range : line)
                sums[sampleNum] += range.samples[sampleNum];
            for (auto& range : line)
                range.samples[sampleNum] /= sums[sampleNum];
        }
        line.push_back(RangeScanLine(line.back()));
        line.back().samples = sums;
        line.back().range = FrequencyRange{ line.front().range.from, (line.end() - 2)->range.to }; //TODO почему -2?
    }
    emit stageProgressed();
  }
}

void ScanProcessingTask::operator() ()
{
  if (rawLines.empty()) {
    emit finished();
    return;
  }
  else
  {
      ba::accumulator_set<std::size_t, ba::stats<ba::tag::sum>> samplesCountAcc;

      for (const auto& line : rawLines) samplesCountAcc(line.samples.size());

      auto count = ba::sum(samplesCountAcc);
      if (count==0) {
          return;
      }
  }

  auto stagesNumber = 12 + (params.columnModelOrder > 0 ? 1 : 0)+(params.shouldNormalize ? 1 : 0);

  stagesNumber = stagesNumber - static_cast<uint>(scan->processingStage);
  emit started("Обработка скана",stagesNumber);

  scan->parameters = params;

  switch(scan->processingStage){
  default:
  case ScanProcessingStage::RawDataObtained:
    scan->currentRange = 0;
    scan->lines = rawLines;

    emit stageStarted("Выравнивание строк - 1", scan->lines.size());
    scan->trimmedLines.clear();

    //дополнение первой строки до размера следующей
    if (scan->lines.size() > 1) {
//*******
      size_t j = 1;
      if (scan->lines.size() > 2) {
          j = 2;
      }
      auto difference = scan->lines[j].samples.size() - scan->lines[0].samples.size();
      if (std::floor(scan->lines[0].samples.size() / 1000) < std::floor(scan->lines[j].samples.size() / 1000)) {
          scan->lines[0].samples.insert(scan->lines[0].samples.begin(), difference, 0.0);
        }
      else {
          scan->lines[0].samples.erase(scan->lines[0].samples.begin(), scan->lines[0].samples.begin() - difference);
      }
//*******
    }

    boost::transform(scan->lines,
                     std::back_inserter(scan->trimmedLines),
                     p::bind(&ScanProcessingTask::trimLine, this, pa::_1, params.initialSkip));
    scan->processingStage = ScanProcessingStage::LinesTrimmed;

  case ScanProcessingStage::LinesTrimmed:
    emit stageStarted("Детектирование пиков", scan->lines.size());
    scan->peaks.clear();
    boost::transform(scan->trimmedLines, 
                     std::back_inserter(scan->peaks),
                     p::bind(&ScanProcessingTask::findPeaks, this, pa::_1, 
                             static_cast<float>(params.peakMagnitudeLimit), 
                             static_cast<float>(params.peakBackstep), 
                             static_cast<float>(params.peakForestep),
                             static_cast<unsigned int>(params.peakPauseCount)));
                             //static_cast<float>(params.peakPauseCount)));
     scan->processingStage = ScanProcessingStage::PeaksDetected;
  case ScanProcessingStage::PeaksDetected:
    emit stageStarted("Нормализация направления сканирования", scan->lines.size());
    boost::for_each(scan->peaks, p::bind(&ScanProcessingTask::normalizeDirection, this, pa::_1));
    scan->processingStage = ScanProcessingStage::DirectionNormalized;
  case ScanProcessingStage::DirectionNormalized:
    emit stageStarted("Разделение на частотные диапазоны",scan->lines.size());
    scan->spec.clear();
    v::transform([this](const SourceScanLineSlice& line, const PeaksLine& peaksLine) { return this->splitFrequencyRanges(line, peaksLine.peaks); }, 
                  std::back_inserter(scan->spec),
                  begin(scan->trimmedLines), end(scan->trimmedLines),
                  begin(scan->peaks));
    scan->processingStage = ScanProcessingStage::FrequencyRangesSplited;

    if(scan->parameters.shouldNormalize){
      emit stageStarted("Нормализация спектра",scan->spec.size());
      normalizeSpectrogram(scan->spec);
    }

    emit stageStarted("Поиск относительного уровня сигнала",scan->normalizedSpec.size());
    //if (scan->parameters.specNormalization.size() < scan->spec.begin()->size())
    //    findRelativeSignals(*scan);
    //else
    //    findRelativeSignals(*scan, scan->parameters.specNormalization);
    if ((scan->spec.size()>0) && (scan->parameters.specNormalization.size() < scan->spec.at(0).size()))
    {
        findRelativeSignals(*scan);
    }
    else
      findRelativeSignals(*scan,scan->parameters.specNormalization);
  case ScanProcessingStage::FrequencyRangesSplited:
    emit stageStarted("Сглаживание строк",scan->normalizedSpec.size());
    smoothRanges(*scan);     //производится сглаживание по строкам с помощью весовой функции,далее работаем со сглаженными графиками
    scan->processingStage = ScanProcessingStage::RangesSmoothed;
  case ScanProcessingStage::RangesSmoothed:
    emit stageStarted("Выборка исследуемых диапазонов", scan->parameters.ranges.size());
    selectRangesFromSpec(*scan);
    scan->processingStage = ScanProcessingStage::RangesSelected;
  case ScanProcessingStage::RangesSelected:
    emit stageStarted("Выравнивание строк - 2", 2*scan->lines.size());
    alignLines(scan->peaks,scan->ranges);
    alignLines(scan->peaks,scan->commonRanges);
    scan->processingStage = ScanProcessingStage::LinesAligned;
  case ScanProcessingStage::LinesAligned:
    emit stageStarted("Нормализация координат пиков", scan->parameters.ranges.size()+scan->commonRanges.front().size());
    normalizeRanges(*scan);
    scan->processingStage = ScanProcessingStage::RangesNormalized;
  case ScanProcessingStage::RangesNormalized:
    if (scan->parameters.columnModelOrder > 0) {
      emit stageStarted("Построение модели столбцов", scan->parameters.ranges.size() * 2);
      scan->averageColumns = averageColumns(*scan);
      scan->averageColumnPolyniomials = rangeColumnModels(scan->averageColumns, scan->normalizedRanges.front().lineCoordinates, scan->parameters.columnModelOrder);

      emit stageStarted("Вычитание моделей столбцов", scan->parameters.ranges.size());
      subtractColumnModels(*scan, scan->averageColumnPolyniomials);
    } else {
      emit stageStarted("Усреднение столбцов столбцов", scan->parameters.ranges.size());
      scan->averageColumns = averageColumns(*scan);
      scan->rangesResiduals.clear();
      scan->rangesResiduals = scan->normalizedRanges;
    }
    scan->processingStage = ScanProcessingStage::ModelSubstracted;
  case ScanProcessingStage::ModelSubstracted:
    emit stageStarted("Выделение дефектных точек", scan->parameters.defectPoints.size());
    scan->rawDefectPoints.clear();

    for (auto const& dp : scan->parameters.defectPoints) {
      scan->rawDefectPoints.push_back(selectDefectPoints(dp, scan->rangesResiduals));
      emit stageProgressed();
    }

    emit stageStarted("Рендеринг дефектных точек (пропорциональный цвет)", scan->parameters.defectPoints.size());
    scan->renderedDefectPoints.clear();
    for (auto const& dp : scan->parameters.defectPoints) {
      scan->renderedDefectPoints.push_back(renderDefectPoints(dp, scan->rangesResiduals, scan->parameters.defectRendering));
      emit stageProgressed();
    }

    emit stageStarted("Рендеринг дефектных точек (фиксированный цвет)", scan->parameters.defectPoints.size());
    scan->renderedDefectPointsFixedColor.clear();
    for (auto const& dp : scan->parameters.defectPoints) {
      scan->renderedDefectPointsFixedColor.push_back(renderDefectPointsWithFixedColor(dp, scan->rangesResiduals, scan->parameters.defectRendering));
      emit stageProgressed();
    }
  }

  emit finished();
  emit newScanReady(scan);
}
