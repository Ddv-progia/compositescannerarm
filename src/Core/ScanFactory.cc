/*
 * Core/ScanFactory.cc
 */


#include "Core/ScanFactory.hh"
#include "Core/ScanProcessingTask.hh"
#include "Core/ScanIO.hh"
#include <iostream>


ScanFactory::ScanFactory(BackgroundTaskExecutor& taskExecutor)
  : taskExecutor(taskExecutor)
{ }

void ScanFactory::startNewScan(const ProcessingParameters& newProcessingParameters)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
  this->processingParameters = newProcessingParameters;
  rangeScanLines.clear();
}

void ScanFactory::addRangeScanLine(const SourceScanLine& newLine)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
  rangeScanLines.push_back(newLine);
}

void ScanFactory::finishScan(boost::optional<QString &> name)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);

  try{
	  auto scan = std::make_shared<Scan>();
	  scan->processingStage = ScanProcessingStage::RawDataObtained;
    if(!name){
	    scan->scanName = "New scan";
      saveToTempDirectory(rangeScanLines);
    } else
      scan->scanName = name->toStdString();
	  auto task = new ScanProcessingTask(rangeScanLines, processingParameters,scan);
	  connect(task, SIGNAL(newScanReady(const std::shared_ptr<Scan>&)), this, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), Qt::DirectConnection);
	  taskExecutor.enqueue(task);
  }
  catch(...) {
	  throw std::exception("Error while finishing scan");
  }
}

void ScanFactory::recalculateScan(std::shared_ptr<Scan>& scan)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
  processingParameters.specNormalization = scan->parameters.specNormalization;
  auto task = new ScanProcessingTask(rangeScanLines, processingParameters,scan);
  connect(task, SIGNAL(newScanReady(const std::shared_ptr<Scan>&)), this, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), Qt::DirectConnection);
  taskExecutor.enqueue(task);
}