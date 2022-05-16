/*
 * Core/ScanProcessingTask.hh
 */

#pragma once

#include "Core/ProgressReportingTask.hh"
#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"
#include "Core/ScanFactory.hh"

#include <QLabel>


class ScanProcessingTask : public ProgressReportingTask
{
  Q_OBJECT

  std::vector<SourceScanLine> rawLines;
  ProcessingParameters params;
  std::shared_ptr<Scan> scan;
public:
  ScanProcessingTask(const std::vector<SourceScanLine>& rawLines, const ProcessingParameters& params,std::shared_ptr<Scan>& newScan);
  virtual void operator() () override;

protected:
  Q_SIGNAL void newScanReady(const std::shared_ptr<Scan>& scan);

private:
  SourceScanLineSlice trimLine(const SourceScanLine& line, double initialSkip);
  PeaksLine findPeaks(const SourceScanLineSlice& line, float peakLimit, double backstep, double forestep, unsigned int pauseCount = 30);
  
  RangeScanLine findAverageLine(std::vector<RangeScanLine>& rangedLines,FrequencyRange range);

  void normalizeDirection(PeaksLine& line);
  std::vector<RangeScanLine> splitFrequencyRanges(const SourceScanLineSlice& line, const std::vector<Peak>& peaks);

  void alignLines(std::vector<PeaksLine>& peaks,std::vector<std::vector<RangeScanLine>>& ranges);
  void smoothRanges(Scan& scan);

  std::tuple<std::size_t, std::size_t, std::size_t> getNormalizedIndexes(Scan& scan) const;
  void normalizeRanges(Scan& scan);
  void normalizeRange(NormalizedRange& normalizedRange,
                      std::vector<std::vector<RangeScanLine>>& ranges,
                      std::size_t rangeIndex,
                      std::size_t step, 
                      std::size_t startIndex, 
                      std::size_t stopIndex,
                      ::Extremum extremumOfRangesIn);
  float getNormalizedPeakAt(const RangeScanLine& line, std::size_t idx);
  float getMaxSubrangePeak(const RangeScanLine& line,std::size_t idx);
  float getMinSubrangePeak(const RangeScanLine& line,std::size_t idx);
  float getAverageSubrangePeak(const RangeScanLine& line,std::size_t idx);
  std::tuple<float, float, float> getNormalizedPeakFromSubranges(const RangeScanLine& line, std::size_t idx);
//  std::pair<float,float> getNormalizedPeakFromSubranges(const RangeScanLine& line, std::size_t idx);
  void selectRangesFromSpec(Scan& scan);
  void findRelativeSignals(Scan& scan);
  void findRelativeSignals(Scan& scan,const std::vector<SpecNormalizationParams>& params);
  void normalizeSpectrogram(std::vector<std::vector<RangeScanLine>>& spec);
  Polynomial signleRangeModel(const std::vector<double>& lineView, 
                              const std::vector<double>& lineCoordinates, 
                              unsigned int order);

  std::vector<Polynomial> rangeColumnModels(const std::vector<std::vector<double>>& averageColumns, 
                                            const std::vector<double>& lineCoordinates,
                                            unsigned int order);

  std::vector<std::vector<double>> averageColumns(Scan& scan);
  void subtractColumnModels(Scan& scan, const std::vector<Polynomial>& models);
};