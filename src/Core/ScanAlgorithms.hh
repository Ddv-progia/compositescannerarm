/*
 * Core/ScanAlgorithms.hh
 */

#pragma once

#include <vector>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "Core/ScanData.hh"

std::vector<Peak>
findPeaks(std::vector<float>::iterator srcBegin, 
          std::vector<float>::iterator srcEnd,
          unsigned int sampleRate, 
          float peakLimit, 
          std::size_t backstep,
          std::size_t forestep,
          std::size_t peakPauseCount=20);
//findPeaks(std::vector<float>::const_iterator srcBegin, 
//          std::vector<float>::const_iterator srcEnd,
//          unsigned int sampleRate, 
//          float peakLimit, 
//          std::size_t backstep,
//          std::size_t forestep,
//          std::size_t peakPauseCount=20);
////std::vector<Peak>
////findPeaks(std::vector<float>::const_iterator srcBegin, 
////          std::vector<float>::const_iterator srcEnd,
////          unsigned int sampleRate, 
////          float peakLimit, 
////          double backstepSeconds,
////          double forestepSeconds,
////    //unsigned int peakPauseCountSeconds=20);
////    double peakPauseCountSeconds=20);

bool getCoordinateOfPeak(size_t &indexInSound, size_t &curChunkIndex, Peak& peak, unsigned int soundSampleRate, std::shared_ptr<::std::vector< ::Position > > positions, std::shared_ptr<::std::vector< ::SourceScanChunk>> scanArmChunks);

std::vector<std::vector<float>>
splitFrequencyRanges(std::vector<float>::const_iterator srcBegin, 
                     std::vector<float>::const_iterator srcEnd,
                     unsigned int sampleRate,
                     const std::vector<Peak>& peaks,
                     const std::vector<FrequencyRange>& ranges);


std::vector<FrequencyRange> constructFrequencyRanges(int beginFreq, int endFreq, int step);
std::vector<FrequencyRange> constructCommonRanges();