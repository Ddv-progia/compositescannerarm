/*
 * Core/ScanCollector.cc
 */


#include "Core/ScanCollector.hh"
#include "Core/ScanProcessingTask.hh"
#include "Core/ScanIO.hh"
#include <iostream>


ScanCollector::ScanCollector(){ 
}

void ScanCollector::share() {
	emit ready(m_scan);
}

std::shared_ptr<Scan> ScanCollector::ScanArmToScan(std::shared_ptr<ScanArm> scanArm)
{
    std::shared_ptr<Scan> scan;
    scan = std::make_shared<Scan>();
    scan->parameters.initialSkip        = scanArm->parameters.initialSkip ;
    scan->parameters.peakMagnitudeLimit = scanArm->parameters.peakMagnitudeLimit;
    scan->parameters.peakBackstep       = scanArm->parameters.peakBackstep;
    scan->parameters.peakForestep       = scanArm->parameters.peakForestep;
    scan->parameters.peakPauseCount     = scanArm->parameters.peakPauseCount;

    ::std::vector< ::FrequencyRange > ranges;

    auto scanArmParametersRangesSize = scanArm->parameters.ranges.size();
    for (std::size_t i = 0; i < scanArmParametersRangesSize; i++) {
        FrequencyRange frequencyRange;
        frequencyRange.from = scanArm->parameters.ranges[i].from;
        frequencyRange.to = scanArm->parameters.ranges[i].to;
        ranges.push_back(frequencyRange);
    }
    scan->parameters.ranges = ranges;

    ::std::vector< ::Extremum > extremumOfRanges;
    scan->parameters.extremumOfRanges = extremumOfRanges;

    ::std::vector< ::SpecNormalizationParams > specNormalization;
    scan->parameters.specNormalization = specNormalization;
    ::std::vector< ::ColorStop > colorStopsList;
    scan->parameters.colorStopsList = colorStopsList;
    ::std::vector< ::DefectType > defectClassification;
    scan->parameters.defectClassification = defectClassification;

    ::std::vector< ::DefectKindView > defectPoints;
    scan->parameters.defectPoints = defectPoints;

    //putVal<std::uint32_t>(db, "parameters.defectPoints.@size", scan.parameters.defectPoints.size());
    //for (std::size_t i = 0; i < scan.parameters.defectPoints.size(); i++) {
    //    putStr(db, (boost::format("parameters.defectPoints.@%1%.title") % i).str(), scan.parameters.defectPoints[i].title);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.red.range") % i).str(), scan.parameters.defectPoints[i].red.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.limit") % i).str(), scan.parameters.defectPoints[i].red.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.red.amplification") % i).str(), scan.parameters.defectPoints[i].red.amplification);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.blue.range") % i).str(), scan.parameters.defectPoints[i].blue.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.limit") % i).str(), scan.parameters.defectPoints[i].blue.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.blue.amplification") % i).str(), scan.parameters.defectPoints[i].blue.amplification);
    //    putVal<unsigned>(db, (boost::format("parameters.defectPoints.@%1%.green.range") % i).str(), scan.parameters.defectPoints[i].green.range);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.limit") % i).str(), scan.parameters.defectPoints[i].green.limit);
    //    putVal<double>(db, (boost::format("parameters.defectPoints.@%1%.green.amplification") % i).str(), scan.parameters.defectPoints[i].green.amplification);
    //}

    scan->parameters.columnModelOrder = 0;

    auto linesSize = scanArm->lines.size();
    for (std::size_t i = 0; i < linesSize; i++) {
        auto lineArm = scanArm->lines.at(i);
        SourceScanLine line;
        line.finalCoordinate = lineArm.finalCoordinate;
        line.finalLineCoordinate= lineArm.finalCoordinate;
        line.lineCoordinate= lineArm.lineCoordinate;
        line.sampleRate= lineArm.sampleRate;
        line.startCoordinate= lineArm.startCoordinate;
        line.timestampEnd= lineArm.timestampEnd;
        line.timestampStart= lineArm.timestampStart;
        for (std::size_t j = 0; j < lineArm.samples.size(); j++) {
            line.samples.push_back(lineArm.samples.at(j));
        }
        scan->lines.push_back(line);
    }
    return scan;
}
