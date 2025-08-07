/*
 * Core/ScanAlgorithms.cc
 */

#include <algorithm>
#include <iterator>
#include <numeric>
#include <fftw3.h>
#include <boost/format.hpp>
#include <QString>
#include <QtWidgets/QMessageBox>
#include <UCL/Exception.hh>
#include <UCL/SignalProcessing/Decimate.hh>
#include <UCL/SignalProcessing/Difference.hh>

#include "Core/ScanAlgorithms.hh"

using boost::format;

namespace {

  const std::size_t HIGH_FREQUENCY_ESTIMATED_PART = 4;

}

std::vector<Peak>
    findPeaks(std::vector<float>::const_iterator srcBegin,
        std::vector<float>::const_iterator srcEnd,
        unsigned int sampleRate,
        float peakLimit,
        std::size_t backstep,
        std::size_t forestep,
        std::size_t peakPauseCount)
{
  //const auto backstep = static_cast<std::size_t>(std::floor(backstepSeconds * sampleRate + 0.5));
  //const auto forestep = static_cast<std::size_t>(std::floor(forestepSeconds * sampleRate + 0.5));
  //const auto pauseCount = static_cast<std::size_t>(std::floor(peakPauseCountSeconds * sampleRate + 0.5));

  if (forestep == 0) {
    BOOST_THROW_EXCEPTION(uts::IncorrectArgumentException() 
      << uts::ErrInfo_Description((format("Длительность пика слишком мала: %1% при частоте дискретизации %2%") % forestep % sampleRate).str()));
  }
  std::vector<Peak> peaks;
  std::size_t srcSize = std::distance(srcBegin, srcEnd);
 //*******
 // Попытка выделить пики по-другому
 //std::size_t peakStart = 0;
 //std::size_t peakEnd = 0;
 //std::size_t peakPauseCount = 0;
 //std::size_t peakWidthMaxLimit = std::size_t(2 * peakPauseCountSeconds);
 //for (std::size_t i = 0; i < srcSize;) { //не рассматриваем пик, попадающий на границу,так как он дает неверный спектр
 //  if (std::abs(*(srcBegin + i)) > peakLimit) {
 //      peakPauseCount = 0;
 //      if (peakStart == 0) {
 //          peakStart = i;
 //          peakEnd = i;
 //      }
 //      else {
 //          peakEnd = i;
 //      }
 //  }
 //  else {
 //      peakPauseCount++;
 //  }
 //  if ((peakPauseCount > peakPauseCountSeconds)||((peakEnd - peakStart)> peakWidthMaxLimit)) {
 //      peaks.push_back(Peak{ (unsigned int)std::max(0, static_cast<int>(peakStart - peakPauseCount)), (unsigned int)(peakEnd + peakPauseCount) });
 //      peakStart = 0;
 //      peakEnd = 0;
 //      peakPauseCount = 0;
 //  }
 //  i++;
 //}          if (startPos < 0) startPos = 0;

 //*******
  int endOfPeakPos = 0;
  auto peakLenght = forestep + backstep;
  for (std::size_t i = 0; i + forestep < srcSize;) { //не рассматриваем пик, попадающий на границу,так как он дает неверный спектр
      if (std::abs(*(srcBegin + i)) > peakLimit) {

          int startPos = i - backstep;
          if (startPos < 0) startPos = 0;
          //peaks.push_back(Peak{ (unsigned int)std::max((unsigned int)0, static_cast<unsigned int>(startPos)), (unsigned int)(i + forestep) });
          peaks.push_back(Peak{ (unsigned int)(startPos), (unsigned int)(i + forestep) });
          //i += (forestep+ pauseCount);
          i += peakLenght;
          endOfPeakPos = i;
      }
      else {
          if (std::abs(int(i) - endOfPeakPos) > (peakLenght)) {
              peaks.push_back(Peak{ static_cast<unsigned int>(endOfPeakPos + 1), (unsigned int)(i) });
              endOfPeakPos = i;
          }
          i++;
      }
  }
  //*******

  //if (peaks.size() == 0) {
  //    //BOOST_THROW_EXCEPTION(uts::IncorrectArgumentException() << uts::ErrInfo_Description("Пики не найдены. Проверьте настройки пикового детектора"));
  //    QMessageBox::critical(nullptr, "Сбой загрузки настроек", QString::fromUtf8("Пики не найдены. Проверьте настройки пикового детектора"));
  //}
  return peaks;
}

/// <summary>
/// ищет координаты пика по timestamp'у
/// заполняет peak найденными координатами
/// </summary>
/// <param name="indexInSound"></param>
/// <param name="peak"></param>
/// <returns>true, если координаты найдены</returns>
bool getCoordinateOfPeak(size_t& indexInSound, size_t& curChunkIndex, Peak& peak, unsigned int soundSampleRate, std::shared_ptr<::std::vector< ::Position >> positions, std::shared_ptr< ::std::vector< ::SourceScanChunk>> scanArmChunks)
{
    bool notFoundCurChunkIndex = true;
    unsigned long long int timestampForIndexInSound = 0;
    while (curChunkIndex < scanArmChunks->size() && notFoundCurChunkIndex) {
        auto currChunk = scanArmChunks->at(curChunkIndex);
        if ((indexInSound >= currChunk.startpositionOfChunk) && (indexInSound < currChunk.endpositionOfChunk)) {
            timestampForIndexInSound = currChunk.timestamp -
                (unsigned long long int)((currChunk.endpositionOfChunk - indexInSound) * (1 / soundSampleRate));
            notFoundCurChunkIndex = false;     // нашли, выход из цикла
        }
        else {
            ++curChunkIndex;
        }
    }
    if (notFoundCurChunkIndex) {
        curChunkIndex--;
        return false;
    }
    size_t curTrajectoryIndex = 0; // первый найденный индекс, по которому timestamp элемента в Trajectory больше,чем timestamp искомого пика
    auto currSizeOfTrajectory = positions->size();
    while (curTrajectoryIndex < currSizeOfTrajectory) {
        auto curTrajectoryTimeStamp = positions->at(curTrajectoryIndex).timeStamp;
        if (timestampForIndexInSound > curTrajectoryTimeStamp) {
            ++curTrajectoryIndex;
        }
        else {
            if (timestampForIndexInSound == curTrajectoryTimeStamp) {
                peak.x = positions->at(curTrajectoryIndex).x;
                peak.y = positions->at(curTrajectoryIndex).y;
                peak.z = positions->at(curTrajectoryIndex).z;
                //notFoundCurTrajectoryIndex = false;
                return true;
            }
            else {
                if (curTrajectoryIndex > 0) {
                    auto trajectoryIndexPred = curTrajectoryIndex - 1;
                    auto dst = (curTrajectoryTimeStamp - positions->at(trajectoryIndexPred).timeStamp);
                    if (dst) {
                        double kt = double(curTrajectoryTimeStamp - timestampForIndexInSound) / double (dst);
                        auto dx = (positions->at(curTrajectoryIndex).x - positions->at(trajectoryIndexPred).x) * kt;
                        auto dy = (positions->at(curTrajectoryIndex).y - positions->at(trajectoryIndexPred).y) * kt;
                        auto dz = (positions->at(curTrajectoryIndex).z - positions->at(trajectoryIndexPred).z) * kt;
                        peak.x = positions->at(curTrajectoryIndex).x - dx;
                        peak.y = positions->at(curTrajectoryIndex).y - dy;
                        peak.z = positions->at(curTrajectoryIndex).z - dz;
                        //notFoundCurTrajectoryIndex = false;
                        return true;
                    }
                    else ++curTrajectoryIndex;
                }
                else ++curTrajectoryIndex;
            }
        }
    }
    if (curTrajectoryIndex >= currSizeOfTrajectory) {
        curTrajectoryIndex = currSizeOfTrajectory - 1;
        if (curTrajectoryIndex < 0) curTrajectoryIndex = 0;
    }
    return false;
}


namespace {

  std::size_t peakLength(const Peak& peak)
  {
    return peak.endIndex - peak.beginIndex;
  }

  std::size_t maxPeakLength(const std::vector<Peak>& peaks)
  {
      std::vector<size_t> peaksize;
      for (auto peak: peaks){
          peaksize.push_back(peak.endIndex- peak.beginIndex);
      }
    switch (peaks.size()) {
    case 0:
      return std::size_t(0);
    case 1:
      return peakLength(peaks.front());
    case 2:
      return std::max(peakLength(peaks.front()), peakLength(peaks.back()));
    default:
      //return peakLength(peaks[1]);
        return size_t(*std::max_element(peaksize.begin(), peaksize.end()));
    }
  }

  std::size_t fftSizeForLength(std::size_t x)
  {
    return (x - (x & 1)) / 2 + 1;
  }

  double calculateFftSum(fftwf_complex* fft,std::size_t fftLength)
  {
    auto fftSize = fftSizeForLength(fftLength);
    double fftSum = 0.0;
    for(std::size_t i = 0;i<fftSize;i++)
      fftSum += std::sqrt(fft[i][0] * fft[i][0] + fft[i][1] * fft[i][1]);
    return fftSum;
  }

  void appendPeakRanges(std::vector<std::vector<float>>& output,
                        const std::vector<FrequencyRange>& ranges,
                        unsigned int sampleRate,
                        std::size_t fftLength,
                        fftwf_complex* fft)
  {
    auto fftSize = fftSizeForLength(fftLength);

    for (std::size_t rangeIndex = 0; rangeIndex < ranges.size(); rangeIndex++) {
      auto rangeStartIndex = static_cast<std::size_t>(std::max(0.0, std::floor(ranges[rangeIndex].from * double(fftLength) / double(sampleRate) + 0.5)));
      auto rangeEndIndex = std::min(fftSize - 1, static_cast<std::size_t>(std::floor(ranges[rangeIndex].to * double(fftLength) / double(sampleRate) + 0.5)));

      double sum = 0.0;
      for (std::size_t i = rangeStartIndex; i < rangeEndIndex; i++) {
        sum += std::sqrt(fft[i][0] * fft[i][0] + fft[i][1] * fft[i][1]);
      }
      sum /= rangeEndIndex - rangeStartIndex;

      output[rangeIndex].push_back(sum);
    }
  }
}

std::vector<std::vector<float>>
splitFrequencyRanges(std::vector<float>::const_iterator srcBegin, 
                     std::vector<float>::const_iterator srcEnd,
                     unsigned int sampleRate,
                     const std::vector<Peak>& peaks,
                     const std::vector<FrequencyRange>& ranges)
{
  std::vector<std::vector<float>> result(ranges.size());

  auto maxFFTLength = maxPeakLength(peaks);
  auto maxFFTSize = fftSizeForLength(maxFFTLength);
  auto inputFrame = static_cast<float*>(fftwf_malloc(maxFFTLength * sizeof(float)));
  auto fft = static_cast<fftwf_complex*>(fftwf_malloc(maxFFTSize * sizeof(fftwf_complex))); 

  // Преобразование первого пика
  std::copy(srcBegin + peaks.front().beginIndex, srcBegin + peaks.front().endIndex, inputFrame);
  fftwf_plan firstPlan = fftwf_plan_dft_r2c_1d(peakLength(peaks.front()), inputFrame, fft, FFTW_ESTIMATE);
  fftwf_execute(firstPlan);

  appendPeakRanges(result, ranges, sampleRate, peakLength(peaks.front()), fft);

  // Преобразование основного массива пиков
  fftwf_plan mainPlan = fftwf_plan_dft_r2c_1d(maxFFTLength, inputFrame, fft, FFTW_ESTIMATE);
  for (std::size_t i = 1; i < peaks.size() - 1; i++) {
    //std::fill(inputFrame, inputFrame + maxFFTLength, 0); //*******
    std::copy(srcBegin + peaks[i].beginIndex, srcBegin  + peaks[i].endIndex, inputFrame);
    fftwf_execute(mainPlan);

    appendPeakRanges(result, ranges, sampleRate, maxFFTLength, fft);
  }

  // Преобразование последнего пика, если он не единственный
  if (peaks.size() > 1) {
      auto bi = peaks.back().beginIndex;
      auto ei = peaks.back().endIndex;

    //std::copy(srcBegin + peaks.back().beginIndex, srcBegin + peaks.back().endIndex, inputFrame);
    std::copy(srcBegin + bi, srcBegin + ei, inputFrame);
    fftwf_plan lastPlan = fftwf_plan_dft_r2c_1d(peakLength(peaks.back()), inputFrame, fft, FFTW_ESTIMATE); 
    //fftwf_execute(firstPlan);
    //*******
    fftwf_execute(lastPlan);
    //*******

    appendPeakRanges(result, ranges, sampleRate, peakLength(peaks.back()), fft);

    fftwf_destroy_plan(lastPlan);
  }
  
  // Освобождение ресурсов
  fftwf_destroy_plan(firstPlan);
  fftwf_destroy_plan(mainPlan);
  
  fftwf_free(fft);
  fftwf_free(inputFrame);

  return result;
}

std::vector<FrequencyRange> constructFrequencyRanges(int beginFreq, int endFreq, int step)
{
  std::vector<FrequencyRange> result;
  for(auto i = beginFreq;i<endFreq;i+=step){
      //*******
      auto start = i;
      if (start == 0)
          start = 100; // не хотим использовать "нулевые" частоты при формировании видов.
    result.push_back(FrequencyRange{ (double)start, (double)i + step });
    //result.push_back(FrequencyRange{ (double)i, (double)i + step }); 
    //*******
  }
  return result;
}

std::vector<FrequencyRange> constructCommonRanges()
{
  std::vector<FrequencyRange> result;
  auto ranges = constructFrequencyRanges(0,12000,2000);
  result.insert(result.end(),ranges.begin(),ranges.end());
  ranges = constructFrequencyRanges(12000,30000,3000);
  result.insert(result.end(),ranges.begin(),ranges.end());
  ranges = constructFrequencyRanges(30000,50000,5000);
  result.insert(result.end(),ranges.begin(),ranges.end());

  return result;
}