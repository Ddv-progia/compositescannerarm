/*
 * Core/ScanProcessingTask.cc
 */

#include <iterator>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/weighted_mean.hpp>
#include <boost/accumulators/statistics/weighted_sum.hpp>
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

//#include <boost/python.hpp>

#include <UCL/Iteration/Variadic/Transform.hh>
#include <UCL/RegressionAnalysis/LeastSquares.hh>
#include <UCL/SignalProcessing/Difference.hh>

#include "Core/ScanAlgorithms.hh"
#include "Core/ScanDefectRenderingTask.hh"
#include "Core/ScanProcessingTask.hh"
#include "DevTalk/Core/Exception.hh"

namespace adp = boost::adaptors;
namespace ba  = boost::accumulators;
namespace p   = boost::phoenix;
namespace pa  = boost::phoenix::arg_names;
namespace v   = uts::iteration::variadic;

ScanProcessingTask::ScanProcessingTask(std::vector<SourceScanLine>& rawLines, const ProcessingParameters& params,std::shared_ptr<Scan>& newScan)
//ScanProcessingTask::ScanProcessingTask(const std::vector<SourceScanLine>& rawLines, const ProcessingParameters& params,std::shared_ptr<Scan>& newScan)
  : rawLines(rawLines), params(params), scan(newScan)
{ }

//ScanProcessingTask::ScanProcessingTask(const std::vector<SourceScanLine>& rawLines, const ProcessingParameters& params, std::shared_ptr<Scan>& newScan, std::shared_ptr<ScanArm>& newScanArm)
//    : rawLines(rawLines), params(params), scan(newScan), scanArm(newScanArm)
//{ }

SourceScanLineSlice ScanProcessingTask::trimLine(SourceScanLine& line, double initialSkip)
{
  SourceScanLineSlice trimmedLine;
  trimmedLine.startCoordinate =     line.startCoordinate;
  trimmedLine.finalCoordinate =     line.finalCoordinate;
  trimmedLine.sampleRate =          line.sampleRate;
  trimmedLine.lineCoordinate =      line.lineCoordinate;
  trimmedLine.finalLineCoordinate = line.finalLineCoordinate;
  trimmedLine.timestampStart=       line.timestampStart;
  trimmedLine.timestampEnd=         line.timestampEnd;
  trimmedLine.startCoordinateZ=     line.startCoordinateZ;
  trimmedLine.finalCoordinateZ=     line.finalCoordinateZ;

  std::size_t offset = static_cast<std::size_t>(std::floor(initialSkip * line.sampleRate + 0.5));
  
  trimmedLine.samplesEnd = line.samples.end();
  trimmedLine.samplesBegin = line.samples.begin() + offset;
  //trimmedLine.samplesBegin = line.samples.begin();
  emit stageProgressed();
  return trimmedLine;
}

PeaksLine ScanProcessingTask::findPeaks(const SourceScanLineSlice& line, float peakLimitIn, double backstepIn, double forestepIn, unsigned int pauseCountIn)
//PeaksLine ScanProcessingTask::findPeaks(SourceScanLineSlice* line, float peakLimitIn, double backstepIn, double forestepIn, double pauseCountIn)
{
    const auto backstep = static_cast<std::size_t>(std::floor(backstepIn * line.sampleRate + 0.5));
    const auto forestep = static_cast<std::size_t>(std::floor(forestepIn * line.sampleRate + 0.5));
    const auto pauseCount = static_cast<std::size_t>(std::floor(pauseCountIn * line.sampleRate + 0.5));

    auto pl =  PeaksLine{ ::findPeaks(line.samplesBegin, line.samplesEnd, line.sampleRate, peakLimitIn, backstep, forestep, pauseCount),
                    line.startCoordinate,
                    line.finalCoordinate,
                    line.lineCoordinate,
                    line.finalLineCoordinate,
                    line.timestampStart,
                    line.timestampEnd,
                    line.startCoordinateZ,
                    line.finalCoordinateZ,

    };
    return pl;
  //return PeaksLine{ ::findPeaks(line.samplesBegin, line.samplesEnd, line.sampleRate, peakLimitIn, backstep, forestep, pauseCount),
  //                  line.startCoordinate,
  //                  line.finalCoordinate,
  //                  line.lineCoordinate,
  //                  line.finalLineCoordinate };
  ////return PeaksLine{ ::findPeaks(line.samplesBegin, line.samplesEnd, line.sampleRate, peakLimit, backstep, forestep, pauseCount),
  ////                  line.startCoordinate,
  ////                  line.finalCoordinate,
  ////                  line.lineCoordinate,
  ////                  line.finalLineCoordinate };
}

/// <summary>
/// ищет координаты пика по timestamp'у
/// заполняет peak найденными координатами
/// </summary>
/// <param name="indexInSound"></param>
/// <param name="peak"></param>
/// <returns>true, если координаты найдены</returns>
//bool ScanProcessingTask::getCoordinateOfPeaks(PeaksLine& line)
bool ScanProcessingTask::getCoordinateOfPeaks(std::shared_ptr<Scan> scan, int needSave)
{
    if (scan->scanArm.isScanArmReady) {
        if (scan->scanArm.trajectory.pos.size() > 0) {
            bool isAllCoordinatesObtained = true;

            if (scan->scanArm.sourceScanChunks.chunks.size() > 0) {
                //TODO проверить! indexInSound не меняется в getCoordinateOfPeak()
                auto soundSampleRate = scan->parameters.headAndScanCollectorParameters.soundsSampleRate;
                auto trajectorySampleRate = scan->scanArm.trajectory.sampleRate;
                auto positions = std::make_shared<::std::vector< ::Position >>(scan->scanArm.trajectory.pos);
                auto scanArmChunks = std::make_shared<::std::vector< ::SourceScanChunk >>(scan->scanArm.sourceScanChunks.chunks);
                size_t indexInSound = 0;
                size_t curChunkIndex = 0;
                for (auto line : scan->peaks) {
                    for (auto peak : line.peaks) {
                        unsigned long long int timestampForIndexInSound = 0;
                        indexInSound = peak.beginIndex;
                        bool isCoordinatesObtained = getCoordinateOfPeak(indexInSound, curChunkIndex, peak, soundSampleRate, positions, scanArmChunks);
                        isAllCoordinatesObtained = isAllCoordinatesObtained && isCoordinatesObtained;
                    }
                }
                return isAllCoordinatesObtained;
            }
            else {
                      
                // пики собраны в линию , по N штук на клетку.
                // где N = scan->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox
                //
                auto peaksCountPerCell = scan->parameters.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox;
                for (size_t lineNum = 0; lineNum < scan->peaks.size(); lineNum++) {
                    auto line = scan->peaks.at(lineNum);
                    auto peaksCount = line.peaks.size();
                    if (peaksCount > 0) {
                        auto startCoordinate = line.startCoordinate;
                        auto finalCoordinate = line.finalCoordinate;
                        auto yStart = line.lineCoordinate;
                        auto yFinal = line.finalLineCoordinate;
                        auto zStart = line.startCoordinateZ;
                        auto zFinal = line.finalCoordinateZ;
                        if (yFinal == -1) {
                            yFinal = yStart; // TODO эта проверка должна быть не здесь, а там где формируется line.finalLineCoordinate
                            line.finalLineCoordinate = yStart;
                        }
                        //if (yFinal == 0) {
                        //    yFinal = yStart; // TODO эта проверка должна быть не здесь, а там где формируется line.finalLineCoordinate
                        //    line.finalLineCoordinate = yStart;
                        //}
                        double xSize = (finalCoordinate - startCoordinate);
                        double ySize = (yFinal - yStart);
                        double zSize = (zFinal - zStart);
                        double countPerCell = peaksCount / peaksCountPerCell;
                        double multiplexX = xSize / countPerCell;
                        double multiplexY = ySize / countPerCell;
                        double multiplexZ = zSize / countPerCell;
                        int j = 0;
                        for (auto& peak : line.peaks) {
                            auto x = startCoordinate + multiplexX * std::floor(j/ peaksCountPerCell);
                            auto y = yStart + multiplexY * j;
                            auto z = zStart + multiplexZ * j;
                            scan->peaks.at(lineNum).peaks.at(j).x = x;
                            scan->peaks.at(lineNum).peaks.at(j).y = y;
                            scan->peaks.at(lineNum).peaks.at(j).z = z;
                            j++;
                        }
                    }
                    else {
                        isAllCoordinatesObtained = false;
                    }
                }
                //isAllCoordinatesObtained = isAllCoordinatesObtained && isCoordinatesObtained;
                return isAllCoordinatesObtained;

            }
        }
        else 
            if ((scan->scanArm.rtPeaks.size() > 0) && (scan->parameters.headAndScanCollectorParameters.needPackInLine)) 
            {
                ::std::vector< ::std::vector< ::std::list< ::Peak > > > rtPeaks;
                //::std::list< ::Peak > peakListSorted;
                ::std::list< ::Peak > peakListSorted;
                for (auto line : scan->scanArm.rtPeaks)
                {
                    for (auto column : line)
                    {
                        for (auto peak : column)
                        {
                            peakListSorted.push_back(peak);
                        }
                    }
                }
                peakListSorted.sort([](::Peak a, ::Peak b) {return a.beginIndex < b.beginIndex; });
                std::vector<unsigned int> peaksIndexesSorted;
                boost::transform(peakListSorted, std::back_inserter(peaksIndexesSorted), std::mem_fn(&Peak::beginIndex));

                int i = 0;
                for (auto& line : scan->peaks)
                {
                    int j = 0;
                    //auto peakFound = ++peakListSorted.begin();
                    //auto peakFoundPred = peakListSorted.begin();
                    int p = 1;
                    //std::cout << "line.size = " << line.peaks.size();
                    for (auto& peak : line.peaks)
                    {
                        auto iidx = boost::lower_bound(peaksIndexesSorted, peak.beginIndex);

                        if (iidx != peaksIndexesSorted.end())
                        {
                            auto ijdx = std::distance(peaksIndexesSorted.begin(), iidx);
                            auto idx2 = ijdx > 0 ? ijdx : 0;
                            auto peakLocal = *boost::next(peakListSorted.begin(), idx2);
                            peak.x = peakLocal.x;
                            peak.y = peakLocal.y;
                            peak.z = peakLocal.z;
                            //scan->peaks.at(i).peaks.at(j).x = x;
                            //scan->peaks.at(i).peaks.at(j).y = y;
                            //scan->peaks.at(i).peaks.at(j).z = 0.0;

                        }
                        j++;
                    }
                    i++;
                }


                //    //vector<int>::iterator lower, upper;
                //lower = lower_bound(peakListSorted.begin(), peakListSorted.end(), peak);
                //upper = upper_bound(v.begin(), v.end(), 6);

                //auto realPeaks = peaks
                //    | adp::filtered([](const PeaksLine& l) { return !l.peaks.empty(); })
                //    | adp::transformed([](const PeaksLine& l) { return l.peaks; })
                //    ;

                    //struct
                //{
                //    bool operator()(int a, int b) const { return a < b; }
                //}
                //customLess;
                //std::sort(peakList.begin(), peakList.end(), customLess);
            }
    }
    else {
        std::size_t i = 0;
        for (auto line : scan->peaks) {
            auto peaksCount = line.peaks.size();
            if (peaksCount > 0) {
                auto startCoordinate = line.startCoordinate;
                auto finalCoordinate = line.finalCoordinate;
                auto yStart = line.lineCoordinate;
                auto yFinal = line.finalLineCoordinate;
                auto zStart = line.startCoordinateZ;
                auto zFinal = line.finalCoordinateZ;
                if (yFinal == -1) {
                    yFinal = yStart; // TODO эта проверка должна быть не здесь, а там где формируется line.finalLineCoordinate
                    line.finalLineCoordinate = yStart;
                }
                double xSize = (finalCoordinate - startCoordinate);
                double ySize = (yFinal - yStart);
                double zSize = (zFinal - zStart);
                //double multiplex = (finalCoordinate - startCoordinate) / peaksCount;
                auto endIndexIt = line.peaks.back().endIndex;
                auto beginIndexIt = line.peaks.front().beginIndex;
                //int allIndexCount = line.peaks.back().endIndex - line.peaks.front().beginIndex;
                int allIndexCount = endIndexIt - beginIndexIt;
                if (allIndexCount > 0) {

                    double multiplexX = xSize / allIndexCount;
                    double multiplexY = ySize / allIndexCount;
                    double multiplexZ = zSize / allIndexCount;
                    std::size_t j = 0;
                    for (auto& peak : line.peaks) {
                        //int peakIndex = (peak.endIndex + peak.beginIndex) / 2 - line.peaks.front().beginIndex;
                        int peakIndex = (peak.endIndex + peak.beginIndex) / 2;
                        ////peak.x = startCoordinate + multiplexX * peakIndex;
                        ////peak.y = yStart + multiplexY * peakIndex;
                        //scan->peaks.at(i).peaks.at(j).x = startCoordinate + multiplexX * peakIndex;
                        //scan->peaks.at(i).peaks.at(j).y = yStart + multiplexY * peakIndex;
                        scan->peaks.at(i).peaks.at(j).x = startCoordinate + multiplexX * (peakIndex - beginIndexIt);
                        scan->peaks.at(i).peaks.at(j).y = yStart + multiplexY * (peakIndex - beginIndexIt);
                        scan->peaks.at(i).peaks.at(j).z = zStart + multiplexZ * (peakIndex - beginIndexIt);
                        Position posLocal;
                        posLocal.x = scan->peaks.at(i).peaks.at(j).x;
                        posLocal.y = scan->peaks.at(i).peaks.at(j).y;
                        posLocal.z = scan->peaks.at(i).peaks.at(j).z;
                        scan->scanArm.trajectory.pos.push_back(posLocal);

                        j++;
                    }
                    i++;
                    //return true;
                }
            }
            //else return false;
        }
    }
    return true;
}

void ScanProcessingTask::formPeaksLineOnWidthHeightAndCoordinateOfPeaks(std::vector<PeaksLine>& peaks, unsigned int height, unsigned int width, unsigned int multyplex)
{
    std::vector < std::vector <int>> counter; // количество пиков в ячейке
    scan->peaks.clear();
    auto h = height % multyplex;
    auto w = width % multyplex;
    for (auto i = 0; i < h; i++) {
        PeaksLine pl;
        for (auto j = 0; j < w; j++) {
            ::Peak p;
            pl.peaks.push_back(p);
        }
        scan->peaks.push_back(pl);
    }

}

void ScanProcessingTask::normalizeDirection(PeaksLine& line)
{
    // Инвертируем строки с обратным ходом
    if (line.finalCoordinate < line.startCoordinate) {
        //std::reverse(&arr[x + 1], &arr[y]);
        std::reverse(line.peaks.begin(), line.peaks.end());

        auto tempValue = line.finalCoordinate;
        line.finalCoordinate = line.startCoordinate;
        line.startCoordinate = tempValue;
        
        tempValue = line.finalLineCoordinate;
        line.finalLineCoordinate = line.lineCoordinate;
        line.lineCoordinate = tempValue;

        tempValue = line.finalCoordinateZ;
        line.finalCoordinateZ = line.startCoordinateZ;
        line.startCoordinateZ = tempValue;

        auto tempStampValue = line.timestampEnd;
        line.timestampEnd = line.timestampStart;
        line.timestampStart = tempStampValue;

        emit stageProgressed();
    }

    return;

  //for (size_t i = 0; i < line.peaks.size(); i++) {
  //    if ((line.finalCoordinate > line.startCoordinate) &&(line.peaks.at(i).beginIndex > line.peaks.at(i).endIndex)) {
  //          line.peaks.erase(line.peaks.begin() + i);
  //    }
  //}
  //if (line.finalCoordinate < line.startCoordinate) {
  //  line = PeaksLine{ std::vector<Peak>(line.peaks.rbegin(), line.peaks.rend()),
  //                    line.finalCoordinate,
  //                    line.startCoordinate,
  //                    line.lineCoordinate };
  //}
  //emit stageProgressed();
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
    if (step == 0) {
        step = 1000;
        params.stepForSplitFrequencyRanges = step;
    }
    unsigned int maxFrequency = std::max(unsigned int(0), line.sampleRate / 2 - step);

    //for(auto startFrequency = 0;startFrequency<=47000;startFrequency+=step)
    for(auto startFrequency = step; startFrequency<= maxFrequency; startFrequency+=step)
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
      boost::transform(peaks, std::back_inserter(r[i].peaks), [](Peak p) {return p; });
      

      r[i].range = ranges[i];
      r[i].sourceLineSize = std::distance(line.samplesBegin, line.samplesEnd);
      r[i].lineCoordinate = line.lineCoordinate;
      r[i].finalLineCoordinate = line.finalLineCoordinate;
      r[i].startCoordinateZ = line.startCoordinateZ;
      r[i].finalCoordinateZ = line.finalCoordinateZ;
      r[i].timestampStart = line.timestampStart;
      r[i].timestampEnd = line.timestampEnd;
    }
  }

  emit stageProgressed();
  //return std::move(r);
  return r;
}

void ScanProcessingTask::rearrangeSpec(std::shared_ptr<Scan>& scan, double Xmin, double Xmax, double Ymin, double Ymax, double dX, double dY)
{
    boost::multi_array<float, 3Ui64> counOfPeakInCell;

    std::size_t lineCount = 0;
    if (dY != 0) {
        lineCount  = floor(0.5 + params.headAndScanCollectorParameters.height / std::abs(dY));
    }
    std::size_t rangeCount = scan->spec.begin()->size();
    //std::size_t peakCount = 1 + floor(0.5+params.headAndScanCollectorParameters.width / numArea);
    std::size_t peakCount = 0;
    if (dX != 0) {
        peakCount = floor(0.5 + params.headAndScanCollectorParameters.width / std::abs(dX));
    }
    //auto be = boost::extents[lineCount][peakCount];
    auto be = boost::extents[lineCount][rangeCount][peakCount];
    counOfPeakInCell.resize(be);         //*******


    auto oldFormedSpec = scan->spec;
    emit stageProgressed();

    for (auto line : scan->spec) {
        line.clear();
    }
    scan->spec.clear();

    emit stageProgressed();
    /////////////*************************               // формируем новую структуру scan->spec
    //std::vector<FrequencyRange> ranges;
    ba::accumulator_set<unsigned int, ba::stats<ba::tag::mean>> meanLineSizeAcc;
    unsigned int meanLineSize;

    for (std::size_t i = 0; i < oldFormedSpec.size(); i++) {
        for (std::size_t j = 0; j < oldFormedSpec[i].size(); j++) {
            meanLineSizeAcc(oldFormedSpec[i][j].sourceLineSize);
        }
    }
    meanLineSize = ba::mean(meanLineSizeAcc);

    for (std::size_t i = 0; i < lineCount; i++) {
        std::vector<RangeScanLine> r;
        for (std::size_t j = 0; j < rangeCount; j++) {
            RangeScanLine scanLine;
            scanLine.samples.resize(peakCount);
            scanLine.sampleIndexes.resize(peakCount);
            scanLine.peaks.resize(peakCount);
            //scanLine.samples.resize(oldFormedSpec[0][0].samples.size());
            //scanLine.sampleRate = oldFormedSpec[0][0].sampleRate;
            scanLine.sampleRate = params.headAndScanCollectorParameters.soundsSampleRate;
            scanLine.startCoordinate = Xmin;
            scanLine.finalCoordinate = Xmax;
            scanLine.sourceLineSize = meanLineSize;
            scanLine.lineCoordinate = double(Ymin + i * dY);
            //scanLine.finalLineCoordinate = double(Ymax + i * dY);
            scanLine.finalLineCoordinate = scanLine.lineCoordinate;
            scanLine.range = oldFormedSpec[0][j].range;
            r.push_back(scanLine);
            emit stageProgressed();
        }
        scan->spec.push_back(r);
    }

    for (std::size_t i = 0; i < oldFormedSpec.size(); i++) {
        for (std::size_t j = 0; j < rangeCount; j++) {
            for (std::size_t s = 0; s < oldFormedSpec[i][j].samples.size(); s++) {
                auto sample = oldFormedSpec[i][j].samples[s];
                auto sampleIndex = oldFormedSpec[i][j].sampleIndexes[s];
                auto peak = oldFormedSpec[i][j].peaks[s];
                int x = int((peak.x - Xmin + dX / 10.0) / dX);
                int y = int((peak.y - Ymin + dY / 10.0) / dY);
                if (((x >= 0) && (x < peakCount)) && ((y >= 0) && (y < lineCount))) {
                    counOfPeakInCell[y][j][x] += 1;
                    //size_t z = size_t(peak.z/numArea);
                    if (scan->spec[y][j].samples.size() <= x) {
                        scan->spec[y][j].samples.resize(x + 1);
                        scan->spec[y][j].sampleIndexes.resize(x + 1);
                        scan->spec[y][j].peaks.resize(x + 1);
                    }
                    scan->spec[y][j].samples.at(x) += sample;
                    scan->spec[y][j].sampleIndexes.at(x) = sampleIndex;
                    scan->spec[y][j].peaks.at(x) = peak;
                }
                emit stageProgressed();
            }
        }
    }

    for (std::size_t i = 0; i < lineCount; i++) {
        for (std::size_t j = 0; j < rangeCount; j++) {
            for (std::size_t s = 0; s < scan->spec[i][j].samples.size(); s++) {
                auto divider = counOfPeakInCell[i][j][s];
                divider = divider > 0.0 ? divider : 1.0;
                scan->spec[i][j].samples[s] /= divider;
                emit stageProgressed();
            }
        }
    }
}

void ScanProcessingTask::reArarngePeak(std::shared_ptr<Scan>  scan)
{
    //rearranged = true;

    //this->operator()();
    //emit stageProgressed();

    return;

    boost::multi_array<float, 3> counOfPeakInCell; // подсчитываем количество пиков в ячейке для усреднения
    auto numArea = params.headAndScanCollectorParameters.currentNumArea;
    //bool isScanArmReadyAndNeedPackInLine = scan->scanArm.isScanArmReady && scan->parameters.headAndScanCollectorParameters.needPackInLine && (numArea > 0);
    bool isScanArmReadyAndNeedPackInLine = scan->scanArm.isScanArmReady && scan->parameters.headAndScanCollectorParameters.needPackInLine;
    if (numArea == 0) numArea = 1;
    if (isScanArmReadyAndNeedPackInLine ) {
        double Xmin = 0.0;
        double Ymin = 0.0;
        double Xmax = params.headAndScanCollectorParameters.width;
        double Ymax = params.headAndScanCollectorParameters.height;

        rearrangeSpec(scan, Xmin, Xmax, Ymin, Ymax, numArea, numArea);
        return;

        std::size_t lineCount = params.headAndScanCollectorParameters.height / numArea;
        std::size_t rangeCount = scan->spec.begin()->size();
        std::size_t peakCount = params.headAndScanCollectorParameters.width / numArea;
        auto be = boost::extents[lineCount][rangeCount][peakCount];
        counOfPeakInCell.resize(be);         //*******

        auto oldFormedSpec = scan->spec;
        emit stageProgressed();

        for (auto line : scan->spec) {
            line.clear();
        }
        scan->spec.clear();

        emit stageProgressed();
        /////////////*************************               // формируем новую структуру scan->spec
        //std::vector<FrequencyRange> ranges;

        for (std::size_t i = 0; i < lineCount; i++) {
            std::vector<RangeScanLine> r;
            for (std::size_t j = 0; j < rangeCount; j++) {
                RangeScanLine scanLine;
                scanLine.samples.resize(peakCount);
                scanLine.sampleIndexes.resize(peakCount);
                scanLine.peaks.resize(peakCount);
                scanLine.sampleRate = oldFormedSpec[0][0].sampleRate;
                scanLine.startCoordinate = 0.0;
                scanLine.finalCoordinate = peakCount * numArea;
                scanLine.sourceLineSize = peakCount * numArea;
                //scanLine.sourceLineSize = (std::abs((int)oldFormedSpec[0][0].peaks[0].endIndex - (int)oldFormedSpec[0][0].peaks[0].beginIndex))* peakCount;
                scanLine.lineCoordinate = double(i * numArea);
                scanLine.finalLineCoordinate = double(i * numArea);
                scanLine.range = oldFormedSpec[0][j].range;
                r.push_back(scanLine);
                emit stageProgressed();
            }
            scan->spec.push_back(r);
        }
        //r[i].samples = std::move(bands[i]);
        //boost::transform(peaks, std::back_inserter(r[i].sampleIndexes), std::mem_fn(&Peak::beginIndex));
        //boost::transform(peaks, std::back_inserter(r[i].peaks), [](Peak p) {return p; });

        for (std::size_t i = 0; i < oldFormedSpec.size(); i++) {
            for (std::size_t j = 0; j < rangeCount; j++) {
                for (std::size_t s = 0; s < oldFormedSpec[i][j].samples.size(); s++) {
                    auto sample = oldFormedSpec[i][j].samples[s];
                    auto sampleIndex = oldFormedSpec[i][j].sampleIndexes[s];
                    auto peak = oldFormedSpec[i][j].peaks[s];
                    int x = int(peak.x / numArea);
                    int y = int(peak.y / numArea);
                    if (((x >= 0) && (x < peakCount)) && ((y >=0) && (y < lineCount))) {
                        counOfPeakInCell[y][j][x] += 1;
                        //size_t z = size_t(peak.z/numArea);
                        scan->spec[y][j].samples.at(x) += sample;
                        scan->spec[y][j].sampleIndexes.at(x) = sampleIndex;
                        scan->spec[y][j].peaks.at(x) = peak;
                    }
                    emit stageProgressed();
                }
            }
        }

        for (std::size_t i = 0; i < lineCount; i++) {
            for (std::size_t j = 0; j < rangeCount; j++) {
                for (std::size_t s = 0; s < scan->spec[i][j].samples.size(); s++) {
                    auto divider = counOfPeakInCell[i][j][s];
                    divider = divider > 0 ? divider : 1;
                    scan->spec[i][j].samples[s] /= divider;
                    //scan->spec[i][j].samples.at(s) = divider;
                    emit stageProgressed();
                }
            }
        }

        /////////////*************************               // конец формируем новую структуру scan->spec

    }
    else if (!scan->scanArm.isScanArmReady) { 
        //return;
        ba::accumulator_set<double, ba::stats<ba::tag::min, ba::tag::max>> XPeakAcc;
        ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::min, ba::tag::max>> YPeakAcc;
        ba::accumulator_set<double, ba::stats<ba::tag::mean>> distanceAcc;
        ba::accumulator_set<double, ba::stats<ba::tag::mean>> peakWidthAcc;
        ba::accumulator_set<int, ba::stats<ba::tag::mean>> lineSizeInIndexes;
        std::vector<double> Ypeak;
        double Ymin = 0.0;
        double Xmin = 0.0;
        double Ymax = 0.0;
        double Xmax = 0.0;

        for (auto& line : scan->peaks) {
            ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::min, ba::tag::max>> YPeakLineAcc;
            ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::min, ba::tag::max>> XPeakLineAcc;
            //ba::accumulator_set<double, ba::stats<ba::tag::mean>> peakWidthLineAcc;
            double peaksCount = (double) line.peaks.size();
            double deltaX = (line.finalCoordinate - line.startCoordinate) / peaksCount;
            peakWidthAcc(deltaX);
            int counter = 0;
            for (auto& peak : line.peaks) {
                double x = line.startCoordinate + deltaX * counter++;
                peak.x = x;
                double y = line.lineCoordinate;
                peak.y = y;
                XPeakLineAcc(peak.x);
                YPeakLineAcc(peak.y);
                double peakWidthLocal = 0.0;
                if (peak.endIndex > peak.beginIndex) {
                    peakWidthLocal = (double)(peak.endIndex - peak.beginIndex);
                }
                else {
                    peakWidthLocal = (double)(peak.beginIndex - peak.endIndex );
                }
            //    peakWidthAcc(peakWidthLocal);
            }
            int sizeInIndexes = 0;
            if (line.peaks.back().endIndex > line.peaks.front().beginIndex) {
                sizeInIndexes = (int)(line.peaks.back().endIndex - line.peaks.front().beginIndex);
            }
            else {
                sizeInIndexes = (int)(line.peaks.front().beginIndex - line.peaks.back().endIndex);
            }
            
            lineSizeInIndexes(sizeInIndexes);
            YPeakAcc(ba::min(YPeakLineAcc));
            YPeakAcc(ba::max(YPeakLineAcc));
            XPeakAcc(ba::min(XPeakLineAcc));
            XPeakAcc(ba::max(XPeakLineAcc));
//            peakWidthAcc((ba::max(XPeakLineAcc) - ba::min(XPeakLineAcc))/ line.peaks.size());
            Ypeak.push_back(ba::mean(YPeakLineAcc));
        }
        auto lineDistances = uts::dsp::difference(Ypeak);
        for (const auto& elem : lineDistances) distanceAcc(elem);
        auto dY = ba::mean(distanceAcc);
        auto meanLineSize = ba::mean(lineSizeInIndexes);


        Ymin = ba::min(YPeakAcc);
        Ymax = ba::max(YPeakAcc);
        Xmin = ba::min(XPeakAcc);
        Xmax = ba::max(XPeakAcc);

        double dX = ba::mean(peakWidthAcc);

        params.headAndScanCollectorParameters.height = floor((Ymax - Ymin) + 0.5);
        params.headAndScanCollectorParameters.width  = floor((Xmax - Xmin) + 0.5);
        params.headAndScanCollectorParameters.currentNumArea = floor(std::abs(dY+0.5));
        params.headAndScanCollectorParameters.deltaX = dX;
        params.headAndScanCollectorParameters.deltaY= dY;

        scan->parameters.headAndScanCollectorParameters.currentNumArea = params.headAndScanCollectorParameters.currentNumArea;
        scan->parameters.headAndScanCollectorParameters.height = params.headAndScanCollectorParameters.height;
        scan->parameters.headAndScanCollectorParameters.width = params.headAndScanCollectorParameters.width;
        scan->parameters.headAndScanCollectorParameters.deltaX = params.headAndScanCollectorParameters.deltaX;
        scan->parameters.headAndScanCollectorParameters.deltaY = params.headAndScanCollectorParameters.deltaY;

        numArea = params.headAndScanCollectorParameters.currentNumArea;
        if (numArea == 0) numArea = 1;

        rearrangeSpec(scan, Xmin, Xmax, Ymin, Ymax, dX, dY);

        /////////////*************************               // конец формируем новую структуру scan->spec

    }

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

    auto meanBa = ba::mean(backwardAcc);
    auto meanFo = ba::mean(forwardAcc);
    auto distance = (meanBa + meanFo);
    //auto distance = ba::mean(backwardAcc) + ba::mean(forwardAcc);

    for (auto& line : ranges) {
      if (! line.empty() && (!line.front().sampleIndexes.empty()))
        for (auto& range : line)
            if (range.sampleIndexes.front() > range.sampleIndexes.back()) {
                for (auto& si : range.sampleIndexes)
                    //si = std::abs(distance - si);
                    si = (distance - si);
            }
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
      if (line.empty()) {
          //break;
          continue;
      }
    if ((! line.front().samples.empty())&&(line.front().sampleIndexes.front()< line.front().sampleIndexes.back())) {
      std::vector<std::size_t> lineDistances;
      boost::adjacent_difference(line.front().sampleIndexes, std::back_inserter(lineDistances));
      lineDistances.erase(lineDistances.begin());
      auto minelem = *boost::min_element(lineDistances);
      distanceAcc(minelem);
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
  if (step == 0) {
      scan.parameters.stepForSplitFrequencyRanges = 1000;
      step = 1000;
  }
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

std::tuple<double, double, double, double> ScanProcessingTask::minMaxCoordinatesOfNormalizedRange(std::vector<std::vector<RangeScanLine>>& ranges, std::size_t rangeIndex)
{
    //auto bar = std::make_tuple("test", 3.1, 14, 'y');
    std::tuple<double, double, double, double> tupleOut;

    auto linesCount = ranges.size();
    if (linesCount > 0) {
        double minlico = ranges.front().front().lineCoordinate;
        double maxlico = ranges.front().front().lineCoordinate;
        double minCoord = ranges.front().front().startCoordinate;
        double maxCoord = ranges.front().front().finalCoordinate;
        double minlicoLocal;
        double maxlicoLocal;
        double minСoordLocal;
        double maxСoordLocal;
        for (std::size_t lineIndex = 0; lineIndex < linesCount; lineIndex++) {
            minlicoLocal = ranges[lineIndex][rangeIndex].lineCoordinate;
            maxlicoLocal = ranges[lineIndex][rangeIndex].finalLineCoordinate;
            minСoordLocal = ranges[lineIndex][rangeIndex].startCoordinate;
            maxСoordLocal = ranges[lineIndex][rangeIndex].finalCoordinate;
            minlico = std::min({ minlico, minlicoLocal, maxlicoLocal });
            maxlico = std::max({ maxlico, minlicoLocal, maxlicoLocal });
            minCoord = std::min({ minCoord , minСoordLocal, maxСoordLocal });
            maxCoord = std::max({ maxCoord , minСoordLocal, maxСoordLocal });
            // }
        }
        tupleOut = std::make_tuple(minCoord, maxCoord, minlico, maxlico);
    }
    return tupleOut;
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
  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::max, ba::tag::min>> acc;
  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::max>> accMax;
  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::min>> accMin;
  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::max, ba::tag::min>> accDiff;

  for (std::size_t lineIndex = 0; lineIndex < linesCount; lineIndex++) {
      if (rangeIndex < ranges[lineIndex].size()) {
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
                      //diffPeak = maxPeak - minPeak - 1; //******* Преобразуем разницу по заданию Сергея Ивановича begin
                      diffPeak = maxPeak - minPeak;

                      //******* Преобразуем среднее по заданию Сергея Ивановича begin
                      int N = 1;
                      for (auto iter = ranges[lineIndex][rangeIndex].subBegin; iter < ranges[lineIndex][rangeIndex].subEnd; iter++) {
                          N++;
                      }
                      //justPeak = justPeak * std::sqrt(N);
                      justPeak = justPeak / std::sqrt(N);

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
                  accMax(maxPeak);
                  accMin(minPeak);
                  //if (maxPeak > normalizedRange.max) normalizedRange.max = maxPeak;
                  //if (minPeak < normalizedRange.min) normalizedRange.min = minPeak;
              }
              normalizedRange.maxView[peakIndex][lineIndex] = maxPeak;
              normalizedRange.minView[peakIndex][lineIndex] = minPeak;
              normalizedRange.averView[peakIndex][lineIndex] = justPeak; //*******
              normalizedRange.diffView[peakIndex][lineIndex] = diffPeak; //*******
              //normalizedRange.aver = justPeak; //*******

          }
      }
  }
  auto average = ba::mean(acc);
  normalizedRange.aver = average; //******* среднее по всем средним
  ////normalizedRange.diff = ba::mean(accDiff); //******* среднее по всем разницам
  ////normalizedRange.diff = ba::max(accDiff); //******* максимум по всем разницам
  //normalizedRange.diff = (ba::max(accDiff)+ ba::mean(accDiff))/2; //*******  //TODO проверить (ba::max(accDiff)+ ba::min(accDiff))/2
  normalizedRange.diff = ba::mean(accDiff); //*******  //TODO проверить (ba::max(accDiff)+ ba::min(accDiff))/2
  normalizedRange.max = ba::max(accMax);
  normalizedRange.min = ba::min(accMin);
  //normalizedRange.max = (normalizedRange.max + normalizedRange.aver)/2;
  //normalizedRange.min = (normalizedRange.min + normalizedRange.aver )/2;
  //normalizedRange.diff = (ba::mean(accDiff)*3)/ average; //*******  приведение к случаю, когда среднее равно трем (т.е. к цветовому диапазону)
  normalizedRange.sampleRate = ranges.front().front().sampleRate / step;
  normalizedRange.startCoordinate = ranges.front().front().startCoordinate;
  normalizedRange.finalCoordinate = ranges.front().front().finalCoordinate;
  normalizedRange.beginIndex = startIndex;
  normalizedRange.endIndex = stopIndex;
  normalizedRange.step = step;
  normalizedRange.extremum = extremumOfRangesIn;
  normalizedRange.lineCoordinates.resize(linesCount);
  normalizedRange.finalLineCoordinates.resize(linesCount);
  normalizedRange.timestampsStart.resize(linesCount);
  normalizedRange.timestampsEnd.resize(linesCount);
  normalizedRange.startCoordinatesZ.resize(linesCount);
  normalizedRange.finalCoordinatesZ.resize(linesCount);
  //normalizedRange.finalLineCoordinate = ranges.front().front().finalLineCoordinate;
  
  if (linesCount > 0) {
    for (std::size_t lineIndex = 0; lineIndex < linesCount; lineIndex++) {
        normalizedRange.lineCoordinates[lineIndex] = ranges[lineIndex][rangeIndex].lineCoordinate;
        normalizedRange.finalLineCoordinates[lineIndex] = ranges[lineIndex][rangeIndex].finalLineCoordinate;
        normalizedRange.timestampsStart[lineIndex] = ranges[lineIndex][rangeIndex].timestampStart;
        normalizedRange.timestampsEnd[lineIndex] = ranges[lineIndex][rangeIndex].timestampEnd;
        normalizedRange.startCoordinatesZ[lineIndex] = ranges[lineIndex][rangeIndex].startCoordinateZ;
        normalizedRange.finalCoordinatesZ[lineIndex] = ranges[lineIndex][rangeIndex].finalCoordinateZ;

    }
    std::tie(normalizedRange.startCoordinate,
        normalizedRange.finalCoordinate,
        normalizedRange.lineCoordinate,
        normalizedRange.finalLineCoordinate) = minMaxCoordinatesOfNormalizedRange(ranges, rangeIndex);
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

        if (iidx != line.sampleIndexes.end()) {
            auto ijdx = std::distance(line.sampleIndexes.begin(), iidx);
            if (ijdx > 0) return line.samples[ijdx];
        }

    }
    return 0.0;
}

bool ScanProcessingTask::getPeakWithCoordAt(const RangeScanLine& line, std::size_t idx, Peak& peak)
{
    bool rezult = false;
    auto sizeOfPeaks = line.peaks.size();
    if ((line.sampleIndexes.size() > 0) && (sizeOfPeaks > 0) && (idx < sizeOfPeaks)) {
        auto iidx = boost::lower_bound(line.sampleIndexes, idx);
        if ((iidx != line.sampleIndexes.end()) && (*iidx == idx)) {
            auto ijdx = std::distance(line.sampleIndexes.begin(), iidx);
            peak = line.peaks[ijdx];
                //rezult = new Peak(line.peaks[ijdx]);
                rezult = true;

        }
        else if (iidx != line.sampleIndexes.end()) {
            auto ijdx = std::distance(line.sampleIndexes.begin(), iidx);
            peak = line.peaks[ijdx > 0 ? ijdx : 0];
            //rezult = new Peak(line.peaks[ijdx > 0 ? ijdx : 0]);
            rezult = true;
        }

    }
    return rezult;
}



std::tuple<bool, float> ScanProcessingTask::tryGetNonNANAndNonInfPeak(const RangeScanLine& line, std::size_t idx)
{
    float result = line.subBegin->samples[idx];
    bool resultFound = !(std::isnan(result) && (!std::isinf(result)));
    if (!resultFound) {
        for (auto iter = line.subBegin; iter < line.subEnd; iter++) {
            if ((!std::isnan(iter->samples[idx])) && (!std::isinf(iter->samples[idx]))) {
                result = iter->samples[idx];
                resultFound = true;
                break;
            }
        }
    } 
    return std::tuple(resultFound, result);
}

float ScanProcessingTask::getAverageSubrangePeak(const RangeScanLine& line, std::size_t idx)
{
  namespace ba = boost::accumulators;
  ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
  //ba::mean(acc);

  float result = line.subBegin->samples[idx];
  for(auto iter = line.subBegin; iter< line.subEnd; iter++){
      if ((!std::isnan(iter->samples[idx])) && (!std::isinf(iter->samples[idx]))) {
          acc(iter->samples[idx]);
      }
  }
  result = ba::mean(acc);
  return result;
}

float ScanProcessingTask::getMaxSubrangePeak(const RangeScanLine& line,std::size_t idx)
{
    bool resultFound;
    float result;
    std::tie(resultFound, result) = tryGetNonNANAndNonInfPeak(line, idx);
  if (resultFound) {
      for (auto iter = line.subBegin; iter < line.subEnd; iter++) {
          if ((!std::isnan(iter->samples[idx])) && (!std::isinf(iter->samples[idx])) && (iter->samples[idx] > result))
              result = iter->samples[idx];
      }
  }
  return result;
}

float ScanProcessingTask::getMinSubrangePeak(const RangeScanLine& line,std::size_t idx)
{
  //float result = line.subBegin->samples[idx];
    bool resultFound;
    float result;
    std::tie(resultFound, result) = tryGetNonNANAndNonInfPeak(line, idx);
    if (resultFound) {
        for (auto iter = line.subBegin; iter < line.subEnd; iter++) {
            if ((!std::isnan(iter->samples[idx])) && (!std::isinf(iter->samples[idx])) && (iter->samples[idx] < result))
                result = iter->samples[idx];
        }
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

  //if ((iidx != line.sampleIndexes.end()) && (*iidx == idx)) {
  //  auto idx = std::distance(line.sampleIndexes.begin(), iidx);
  //  max = getMaxSubrangePeak(line,idx);
  //  min = getMinSubrangePeak(line,idx);
  //  aver = getAverageSubrangePeak(line,idx);
  //} else if (iidx != line.sampleIndexes.end()) {
  //  auto idx2 = std::distance(line.sampleIndexes.begin(), iidx);
  //  max = getMaxSubrangePeak(line,idx2 > 0 ? idx2 : 0);
  //  min = getMinSubrangePeak(line,idx2 > 0 ? idx2 : 0);
  //  aver = getAverageSubrangePeak(line,idx2 > 0 ? idx2 : 0);
  //}
  if (iidx != line.sampleIndexes.end()) {
      auto idx2 = std::distance(line.sampleIndexes.begin(), iidx);
      auto idx3 = idx2 > 0 ? idx2 : 0;
      max = getMaxSubrangePeak(line, idx3);
      min = getMinSubrangePeak(line, idx3);
      aver = getAverageSubrangePeak(line, idx3);
      //max = getMaxSubrangePeak(line, idx2 > 0 ? idx2 : 0);
      //min = getMinSubrangePeak(line, idx2 > 0 ? idx2 : 0);
      //aver = getAverageSubrangePeak(line, idx2 > 0 ? idx2 : 0);
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
      bool a = (column[i] > -std::numeric_limits<double>::infinity());
      bool b = (column[i] < std::numeric_limits<double>::infinity());
      bool c = (a) && (b);
    if (c) {
    //if (! ((! (column[i] > -std::numeric_limits<double>::infinity())) && (! (column[i] < -std::numeric_limits<double>::infinity())))) {
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

std::vector<std::vector<double>> ScanProcessingTask::averageColumns(Scan& scan) //вычисление среднего значения для расширенного графика
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
  //int pointsCount = scan.parameters.smoothingPointsCount/2+1;
  int pointsCount = scan.parameters.smoothingPointsCount;
  if(pointsCount<2)
    return;
  std::vector<double> koeff;
  for (int nP = 0; nP < pointsCount; nP++) {
      double k = pointsCount - nP;
      koeff.push_back(k*k);
  }

  double sum = 0;
  auto koef = koeff.begin();
  sum += *koef;
  koef++;
  for (; koef < koeff.end(); koef++) {
      sum += 2 * (*koef);
  }
  double scaleForKoef = 1 / sum;

  
  for (auto koef = koeff.begin(); koef < koeff.end(); koef++) {
      *koef = (*koef)* scaleForKoef;
  }
  
  for (auto& line : scan.normalizedSpec) {
      for (auto& range : line) {
          auto point = range.samples.begin();
          int indx = 0;
          std::vector<float> result;
          for (; point < range.samples.end(); point++, indx++) {
              ba::accumulator_set<float, ba::stats<ba::tag::mean>> Acc;
              ba::accumulator_set<float, ba::stats<ba::tag::weighted_mean>, double> AccW;
              Acc(*point);
              AccW(*point, ba::weight = koeff[0]);
              for (int samplesIndx = 1; samplesIndx < pointsCount; samplesIndx++) {
                  if ((indx - samplesIndx) > 0) {
                      Acc(*(point - samplesIndx));
                      AccW(*(point - samplesIndx), ba::weight = koeff[(samplesIndx)]);
                  }
                  if ((indx + samplesIndx) < range.samples.size()) {
                      Acc(*(point + samplesIndx));
                      AccW(*(point + samplesIndx), ba::weight = koeff[(samplesIndx)]);
                  }

              }
              if (scan.parameters.smoothingPointsWeigted) {
                  auto re = ba::weighted_mean(AccW);
                  result.push_back(re);
              }
              else {
                  auto re = ba::mean(Acc);
                  result.push_back(re);
              }
          }
          range.samples = result;
      }
      emit stageProgressed();
  }

  //for(auto& line : scan.normalizedSpec){
  //  for(auto& range : line){
  //    auto point = range.samples.begin();
  //    std::vector<float> result;
  //    if (range.samples.size() >= pointsCount) {
  //        for (; point < range.samples.end(); point++) {
  //            ba::accumulator_set<float, ba::stats<ba::tag::mean>> Acc;
  //            ba::accumulator_set<float, ba::stats<ba::tag::weighted_mean>, int> AccW;

  //            for (int nP = pointsCount; nP > 0; nP--) {
  //                Acc(*point);                        //центральная точка имеет наибольший вклад
  //                //AccW(*point, ba::weight = 2);                        //центральная точка имеет наибольший вклад
  //                AccW(*point, ba::weight = pointsCount);                        //центральная точка имеет наибольший вклад
  //                auto begin = point;
  //                auto end = point;

  //                for (signed int i = 1; i < nP; i++) {	//вклад остальных точек зависит от удаленности от центральной
  //                    if (begin >= range.samples.begin()) {   //проверка на выход точек за заданный диапазон
  //                        //if((begin != point) && (begin>=range.samples.begin())){   //проверка на выход точек за заданный диапазон
  //                        --begin;
  //                        Acc(*begin);
  //                        AccW(*begin, ba::weight = nP - i);
  //                        //AccW(*begin, ba::weight = 1);
  //                    }
  //                    else if (begin != point && begin == range.samples.begin()) {
  //                        Acc(*begin);
  //                        AccW(*begin, ba::weight = nP - i);
  //                        //AccW(*begin, ba::weight = 1);
  //                    }
  //                    if (end <= (range.samples.end() - 1)) {
  //                        //if((end != point)&&(end<=(range.samples.end()-1))){
  //                        ++end;
  //                        Acc(*end);
  //                        AccW(*end, ba::weight = nP - i);
  //                        //AccW(*end, ba::weight = 1);
  //                    }
  //                    else if (end != point && (end == range.samples.end() - 1)) {
  //                        Acc(*end);
  //                        AccW(*end, ba::weight = nP - i);
  //                        //AccW(*end, ba::weight = 1);
  //                    }
  //                }
  //            }
  //            //std::cout << " ba::mean(Acc) = " <<ba::mean(Acc) << "ba::weighted_mean(AccW) = " << ba::weighted_mean(AccW) << "\r\n";
  //            if (scan.parameters.smoothingPointsWeigted) {

  //                result.push_back(ba::weighted_mean(AccW));
  //            }
  //            else {
  //                result.push_back(ba::mean(Acc));
  //            }
  //        }
  //        range.samples = result;
  //    }
  //  }
  //  emit stageProgressed();
  //}
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
  auto nRanges = scan.normalizedSpec.begin()->size();
  std::vector<SpecNormalizationParams> params;
  params.resize(nRanges);
  for (int nRange = 0; nRange < nRanges; nRange++) {
      ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::sum, ba::tag::variance>> acc;
      for (auto& line : scan.normalizedSpec)
          for (auto& point : line[nRange].samples)
              if (point != 0) 
                  acc(point);
      //SpecNormalizationParams specNormalizationParams;
      //specNormalizationParams.sigma = std::sqrt(ba::variance(acc));
      //specNormalizationParams.average = ba::mean(acc);
      //params.push_back(specNormalizationParams);
      params[nRange].sigma = std::sqrt(ba::variance(acc));
      params[nRange].average = ba::mean(acc);
      findRelativeSignals(scan, params);
  }

  //auto nRanges = scan.spec.begin()->size();
  //scan.normalizedSpec = scan.spec;
  //std::vector<SpecNormalizationParams> params;
  //for(int nRange = 0;nRange<nRanges;nRange++){
  //  ba::accumulator_set<double, ba::stats<ba::tag::mean, ba::tag::sum, ba::tag::variance>> acc;
  //  for(auto& line : scan.spec)
  //    for(auto& point : line[nRange].samples)
  //      acc(point);
  //  params[nRange].sigma = std::sqrt(ba::variance(acc));
  //  params[nRange].average = ba::mean(acc);
  //  findRelativeSignals(scan, params);
  //  //auto sigma = std::sqrt(ba::variance(acc));
  //  //auto average = ba::mean(acc);
  //  //auto sum = ba::sum(acc);
  //  //if (scan.parameters.shouldNormalize) sigma *= sum; // new 02/10/2024
  //  //for (auto& line : scan.normalizedSpec) {
  //  //    if (line.size() > nRange) {
  //  //        for (auto& point : line[nRange].samples)
  //  //            if (point != 0)
  //  //                point = (point - average) / (3 * sigma);
  //  //    }
  //  //}
  //  //stageProgressed();
  //}
}

void ScanProcessingTask::findRelativeSignals(Scan& scan, const std::vector<SpecNormalizationParams>& params)
{
    int nRanges = 0;
    if (scan.normalizedSpec.size() != 0) {
        nRanges = scan.normalizedSpec.begin()->size();
    }
    else
    {
        BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Description("Ошибка в findRelativeSignals: нулевой размер normalizedSpec"));
    }

    for (int nRange = 0; nRange < nRanges; nRange++) {
        auto sigma = params[nRange].sigma;
        auto average = params[nRange].average;
        if (sigma != 0) {
            for (auto& line : scan.normalizedSpec) {
                if (line.size() > nRange) {
                    for (auto& point : line[nRange].samples)
                        if (point != 0)
                            //point = (point - average) / (3 * sigma);
                            point = (point - average) / (1 * sigma);
                }
            }
        }
        stageProgressed();
    }
    //auto nRanges = scan.spec.begin()->size();
    //scan.normalizedSpec = scan.spec;
    //for(int nRange = 0;nRange<nRanges;nRange++){
    //  auto sigma = params[nRange].sigma;
    //  auto average = params[nRange].average;

    //  for(auto& line:scan.normalizedSpec){
    //      if (line.size() > nRange) {
    //          for (auto& point : line[nRange].samples)
    //              if (point != 0)
    //                  point = (point - average) / (3 * sigma);
    //      }
    //  }
    //  stageProgressed();
    //}
}

void ScanProcessingTask::restartProcessingTask(Scan& scan, const std::vector<SpecNormalizationParams>& params)
{

}

void ScanProcessingTask::normalizeSpectrogramNew(std::vector<std::vector<RangeScanLine>>& spec, float multiplierForSigma)
{
    auto rangeSize = 0;
    //auto rangeSize = spec.begin()->size();
    if (spec.size() > 0) {
        rangeSize = spec.begin()->size();
    }
    float normMean =1;
    for (auto rangeIndex = 0; rangeIndex < rangeSize; rangeIndex++) {
        ba::accumulator_set<float, ba::stats<ba::tag::mean, ba::tag::max, ba::tag::min, ba::tag::variance>> valuesInRange;
        for (auto& line : spec) {
            for (auto& value : line.at(rangeIndex).samples) {
                valuesInRange(value);
            }
        }
        normMean = ba::mean(valuesInRange);
        auto normMeanMax = ba::max(valuesInRange);
        auto normMeanMin = ba::min(valuesInRange);
        auto sigma = std::sqrt(ba::variance(valuesInRange));
        auto normMeanVar = sigma * multiplierForSigma;

        auto normSum = ba::sum(valuesInRange);

        ba::accumulator_set<float, ba::stats<ba::tag::mean, ba::tag::max, ba::tag::min, ba::tag::variance>> valuesInRangeClear;
        for (auto& line : spec) {
            for (auto& value : line.at(rangeIndex).samples) {
                if (std::abs(value- normMean)< (normMeanVar)) {
                    valuesInRangeClear(value);
                }
            }
        }
        normMean = ba::mean(valuesInRangeClear);
        //normMeanMax = ba::max(valuesInRangeClear);
        //normMeanMin = ba::min(valuesInRangeClear);
        sigma = std::sqrt(ba::variance(valuesInRangeClear));
        normMeanVar = sigma * multiplierForSigma;

        normSum = ba::sum(valuesInRangeClear);

        auto maxDiff = std::max(std::abs(normMeanMax - normMean), std::abs(normMean - normMeanMin));
        auto diff = maxDiff;
        if (normMean == 0) normMean = 1;
        for (auto& line : spec) {
            for (auto& value : line.at(rangeIndex).samples) {
                value = value / std::abs(normMean);
                //value = (value) / normMeanVar;
                //value = (value - normMean) / normMeanVar;
                //value = (value/ std::abs(value))* std::abs(value - normMean) / normMeanVar;
                //value = (value - normMean) / normMeanVar;
                //value = (value - normMean)*(std::abs(value - normMean)) / normMeanVar;
            }
        }
    }

    for (auto& line : spec) {
        ba::accumulator_set<float, ba::stats<ba::tag::mean, ba::tag::sum>> sumOfSumes;
        if (line.size() > 0) {
            std::vector<float> sums;
            auto lineSamplesSize = line.begin()->samples.size();
            sums.resize(lineSamplesSize);
            for (auto sampleNum = 0; sampleNum < lineSamplesSize; sampleNum++) {

                ba::accumulator_set<float, ba::stats<ba::tag::mean, ba::tag::sum>> sumOfSamples;
                for (auto& range : line) {
                    sumOfSamples(range.samples[sampleNum]);
                }
                auto normSum = ba::sum(sumOfSamples);
                sums[sampleNum] = normSum;
                sumOfSumes(normSum);
                //sums[sampleNum] = normSum/ normMean;
            }

            auto averSum = ba::mean(sumOfSumes);
            for (auto& val : sums) {
                val = val/ averSum;
            }

            auto fromLocal = line.front().range.from;
            auto toLocal = line.back().range.to;
            line.push_back(RangeScanLine(line.back()));
            line.back().samples = sums;
            line.back().range = FrequencyRange{ fromLocal , toLocal };
        }
        emit stageProgressed();
    }

}

//******************                               линии     ranges
void ScanProcessingTask::normalizeSpectrogram(std::vector<std::vector<RangeScanLine>>& spec)
{
  for(auto& line:spec){
    if (line.size() == 0) {
        //BOOST_THROW_EXCEPTION(uts::IndexOutOfBoundsException() << uts::ErrInfo_Description("Ошибка нормализации спектрограммы: число полученных линий в спектре равно 0"));
    }
    else {
        std::vector<float> sums;
        auto lineSamplesSize = line.begin()->samples.size();
        sums.resize(lineSamplesSize);
        for (auto sampleNum = 0; sampleNum < lineSamplesSize; sampleNum++) {

            ba::accumulator_set<float, ba::stats<ba::tag::sum, ba::tag::mean, ba::tag::max, ba::tag::min, ba::tag::variance>> normMean;
            //ba::accumulator_set<float, ba::stats<ba::tag::sum>> normMean;

            for (auto& range : line) {
                //sums[sampleNum] += range.samples[sampleNum];
                normMean(range.samples[sampleNum]);
            }

            auto normMeanMean = ba::mean(normMean);
            //auto a = ba::covariate2(normMean);
            auto normMeanMax = ba::max(normMean);
            auto normMeanMin = ba::min(normMean);
            auto sigma = std::sqrt(ba::variance(normMean));
            //auto normMeanVar  = ba::variance(normMean)*3.0;
            auto normMeanVar  = sigma*3.0;

            auto normSum = ba::sum(normMean);
            //sums[sampleNum] = normSum/ line.size();
            sums[sampleNum] = normSum;

            auto maxDiff = std::max(std::abs(normMeanMax - normMeanMean), std::abs(normMeanMean - normMeanMin));
            //auto maxDiff = normMeanMax - normMeanMin;
            auto diff = maxDiff;
            //auto a = 3 / diff;
            //auto a = 3* sigma / diff;
            auto a = normMeanVar;
            if ((diff != 0) && (!std::isnan(diff)) && (!std::isinf(diff)))
            { a = a / diff; }
            auto b = normMeanMean * a;
            //auto c = 3/(normMeanMax - normMeanMean);
            //auto c = normMeanVar /(normMeanMax - normMeanMean);
            auto c = normMeanVar;
            if ((normMeanMax - normMeanMean)!=0) c = c/(normMeanMax - normMeanMean);
            if (normSum != 0)
                for (auto& range : line) {
                    //range.samples[sampleNum] = (normMeanMean - range.samples[sampleNum])/normMeanVar;
                    //range.samples[sampleNum] = (b - range.samples[sampleNum]*a);
                    //range.samples[sampleNum] = (range.samples[sampleNum] - normMeanMean) *c;
                    //range.samples[sampleNum] = (range.samples[sampleNum] - normMeanMean)*a;
                    //range.samples[sampleNum] = (range.samples[sampleNum] - normMeanMean)/(normMeanVar);
                    range.samples[sampleNum] /= normSum; //заремарил 02/07/2025 - после сухого
                    //range.samples[sampleNum] /= (3 * sigma);
                };
            //for (auto& range : line) {
            //    range.samples[sampleNum] /= sums[sampleNum];
            //}
        }
        auto fromLocal = line.front().range.from;
        auto toLocal = line.back().range.to;
        line.push_back(RangeScanLine(line.back())); 
        line.back().samples = sums;  
        //line.back().range = FrequencyRange{ line.front().range.from, (line.end() - 2)->range.to }; //TODO почему -2?
        //line.back().range = FrequencyRange{ line.front().range.from, line.back().range.to }; 
        line.back().range = FrequencyRange{ fromLocal , toLocal };
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

  size_t indexInSound = 0;
  size_t curChunkIndex = 0;

  auto stagesNumber = 11 + (params.columnModelOrder > 0 ? 1 : 0)+(params.shouldNormalize ? 1 : 0)+(params.shouldRelate ? 1 : 0);

  stagesNumber = stagesNumber - static_cast<uint>(scan->processingStage);
  emit started("Обработка скана",stagesNumber);

  ::std::vector< ::PeaksLine > tempPeaks;

  scan->parameters = params;
  auto count_Local = 2;
  bool foundIt = false;

  switch(scan->processingStage){
  default:
  case ScanProcessingStage::RawDataObtained:
    scan->currentRange = 0;
    scan->lines = rawLines;

    emit stageStarted("Выравнивание строк - 1", scan->lines.size());
    scan->trimmedLines.clear();

    //дополнение первой строки до размера следующей // TODO проверить, надо ли?
    if (params.headAndScanCollectorParameters.needTrimFirstLine) {
        if (scan->lines.size() > 1) {
            //*******
            size_t j = 1;
            if (scan->lines.size() > 2) {
                j = 2;
            }
            auto difference = scan->lines[j].samples.size() - scan->lines[0].samples.size();
            //if (std::floor(scan->lines[0].samples.size() / 1000) < std::floor(scan->lines[j].samples.size() / 1000)) {
            if (scan->lines[0].samples.size() < scan->lines[j].samples.size()) {
                scan->lines[0].samples.insert(scan->lines[0].samples.begin(), difference, 0.0);
            }
            else {
                scan->lines[0].samples.erase(scan->lines[0].samples.begin(), scan->lines[0].samples.begin() - difference);
            }
            //*******
        }
    }
    //boost::transform(scan->lines,
    //                 std::back_inserter(scan->trimmedLines),
    //                 p::bind(&ScanProcessingTask::trimLine, this, pa::_1, params.initialSkip));
    for (auto& line : scan->lines) {
        auto tl = trimLine(line, params.initialSkip);
        scan->trimmedLines.push_back(tl);
    }
    if (params.headAndScanCollectorParameters.needIgnoreFirstLine) {
        scan->trimmedLines.erase(scan->trimmedLines.begin());
    }
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
    //for (auto trimLineLocal : scan->trimmedLines) {
    //    //auto pl = findPeaks(trimLine,
    //    //                    static_cast<float>(params.peakMagnitudeLimit),
    //    //                    static_cast<float>(params.peakBackstep),
    //    //                    static_cast<float>(params.peakForestep),
    //    //                    static_cast<unsigned int>(params.peakPauseCount));
    //    const auto backstep = static_cast<std::size_t>(std::floor(params.peakBackstep * trimLineLocal.sampleRate + 0.5));
    //    const auto forestep = static_cast<std::size_t>(std::floor(params.peakForestep * trimLineLocal.sampleRate + 0.5));
    //    const auto pauseCount = static_cast<std::size_t>(std::floor(params.peakPauseCount * trimLineLocal.sampleRate + 0.5));
    //    auto pl = PeaksLine{ ::findPeaks(trimLineLocal.samplesBegin, trimLineLocal.samplesEnd, trimLineLocal.sampleRate, params.peakMagnitudeLimit, backstep, forestep, pauseCount),
    //                    trimLineLocal.startCoordinate,
    //                    trimLineLocal.finalCoordinate,
    //                    trimLineLocal.lineCoordinate,
    //                    trimLineLocal.finalLineCoordinate,
    //                    trimLineLocal.timestampStart,
    //                    trimLineLocal.timestampEnd,
    //                    trimLineLocal.startCoordinateZ,
    //                    trimLineLocal.finalCoordinateZ,
    //    };
    //    scan->peaks.push_back(pl);
    //}
                             //static_cast<float>(params.peakPauseCount)));
     scan->processingStage = ScanProcessingStage::PeaksDetected;
     //******* вставим звук 1.5 на границе пиков
     //for (auto i = 0; i < scan->trimmedLines.size(); i++) {
     //    auto& line = scan->lines.at(i);
     //    auto peakLine = scan->peaks.at(i);
     //    for (auto peak: peakLine.peaks) {
     //        *(line.samples.begin() + peak.beginIndex) = 1.5;
     //    }
     //}
     //******* конец вставим звук 1.5 на границе пиков
  case ScanProcessingStage::PeaksDetected: 
    emit stageStarted("Определение координат пиков", scan->peaks.size());
    //boost::for_each(scan->peaks, p::bind(&ScanProcessingTask::getCoordinateOfPeaks, this, pa::_1));
    getCoordinateOfPeaks(scan);
    scan->processingStage = ScanProcessingStage::PeaksCoordinateFinded;
  case ScanProcessingStage::PeaksCoordinateFinded:
    emit stageStarted("Нормализация направления сканирования", scan->peaks.size());
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
  case ScanProcessingStage::FrequencyRangesSplited:

      for (auto line: scan->spec) {
          for (auto item : line) {
              auto localSize = item.sampleIndexes.size();
              if (foundIt = (localSize > 0)) {
                  count_Local = scan->spec.size() * line.size() * localSize;
              };
              if (foundIt) break;
          }
      }
      //count_Local = scan->spec.size() * scan->spec.begin()->size() * scan->spec.begin()->begin()->sampleIndexes.size();
      emit stageStarted("Реорганизация пиков согласно найденным координатам", count_Local);
      
      if (rearranged == true) { return; };
      reArarngePeak(scan);
      //scan->processingStage = ScanProcessingStage::SpecNormalized;

      scan->processingStage = ScanProcessingStage::SpectreRestructuredOnPeaksCoordinates;
  case ScanProcessingStage::SpectreRestructuredOnPeaksCoordinates:
    emit stageStarted("Нормализация спектра",scan->spec.size());
    scan->normalizedSpec = scan->spec;
    
    if(scan->parameters.shouldNormalize){
      ////normalizeSpectrogram(scan->spec);
      //normalizeSpectrogram(scan->normalizedSpec);
      if (scan->parameters.shouldNormalizeNew) {
          //normalizeSpectrogram(scan->spec);
          float multiplier = scan->parameters.multiplierForSigma;
          normalizeSpectrogramNew(scan->normalizedSpec, multiplier);
      }
      else normalizeSpectrogram(scan->normalizedSpec);

    }
    scan->processingStage = ScanProcessingStage::SpecNormalized; 
  case ScanProcessingStage::SpecNormalized:
    emit stageStarted("Поиск относительного уровня сигнала",scan->normalizedSpec.size());
    if (scan->parameters.shouldRelate)
    {
        if ((scan->spec.size() > 0) && (scan->parameters.specNormalization.size() < scan->spec.begin()->size()))
            findRelativeSignals(*scan);
        else
            findRelativeSignals(*scan, scan->parameters.specNormalization);
    }
    scan->processingStage = ScanProcessingStage::RelativeSignalsFounded;
  case ScanProcessingStage::RelativeSignalsFounded:
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
    emit stageStarted("Нормализация пиков", scan->parameters.ranges.size()+scan->commonRanges.front().size());
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
      emit stageStarted("Усреднение столбцов", scan->parameters.ranges.size());
      scan->averageColumns = averageColumns(*scan);
      scan->rangesResiduals.clear();
      scan->rangesResiduals = scan->normalizedRanges;
    }
    scan->processingStage = ScanProcessingStage::ModelSubstracted;
  case ScanProcessingStage::ModelSubstracted:
    emit stageStarted("Выделение точек неоднородностей", scan->parameters.defectPoints.size());
    //emit stageStarted("Выделение дефектных точек", scan->parameters.defectPoints.size());
    scan->rawDefectPoints.clear();

    for (auto const& dp : scan->parameters.defectPoints) {
      scan->rawDefectPoints.push_back(selectDefectPoints(dp, scan->rangesResiduals));
      emit stageProgressed();
    }

    emit stageStarted("Рендеринг точек неоднородностей(пропорциональный цвет)", scan->parameters.defectPoints.size());
    //emit stageStarted("Рендеринг дефектных точек (пропорциональный цвет)", scan->parameters.defectPoints.size());
    scan->renderedDefectPoints.clear();
    for (auto const& dp : scan->parameters.defectPoints) {
      scan->renderedDefectPoints.push_back(renderDefectPoints(dp, scan->rangesResiduals, scan->parameters.defectRendering));
      emit stageProgressed();
    }

    emit stageStarted("Рендеринг точек неоднородностей(фиксированный цвет)", scan->parameters.defectPoints.size());
    //emit stageStarted("Рендеринг дефектных точек (фиксированный цвет)", scan->parameters.defectPoints.size());
    scan->renderedDefectPointsFixedColor.clear();
    for (auto const& dp : scan->parameters.defectPoints) {
      scan->renderedDefectPointsFixedColor.push_back(renderDefectPointsWithFixedColor(dp, scan->rangesResiduals, scan->parameters.defectRendering));
      emit stageProgressed();
    }
  }

  emit finished();
  emit newScanReady(scan);
}
