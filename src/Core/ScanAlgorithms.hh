/*
 * Core/ScanAlgorithms.hh
 */

#pragma once

#include <vector>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "Core/ScanData.hh"

std::vector<Peak>
findPeaks(std::vector<float>::const_iterator srcBegin, 
          std::vector<float>::const_iterator srcEnd,
          unsigned int sampleRate, 
          float peakLimit, 
          double backstepSeconds,
          double forestepSeconds,
    unsigned int peakPauseCountSeconds=20);


std::vector<std::vector<float>>
splitFrequencyRanges(std::vector<float>::const_iterator srcBegin, 
                     std::vector<float>::const_iterator srcEnd,
                     unsigned int sampleRate,
                     const std::vector<Peak>& peaks,
                     const std::vector<FrequencyRange>& ranges);


std::vector<FrequencyRange> constructFrequencyRanges(int beginFreq, int endFreq, int step);
std::vector<FrequencyRange> constructCommonRanges();