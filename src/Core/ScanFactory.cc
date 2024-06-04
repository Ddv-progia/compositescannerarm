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

/*void ScanFactory::startNewScan(const ProcessingParameters& newProcessingParameters)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
  this->processingParameters = newProcessingParameters;

}

void ScanFactory::addRangeScanLine(const SourceScanLine& newLine)
{
  std::lock_guard<std::mutex> lock(rangeScanLinesMutex);

}*/

void ScanFactory::finishScan(std::shared_ptr<Scan> scan)
{
	std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
	try {
		scan->processingStage = ScanProcessingStage::RawDataObtained;
		scan->scanName = "New scan";
		//saveToTempDirectory(rangeScanLines);
		auto task = new ScanProcessingTask(scan);
		connect(task, SIGNAL(newScanReady(const std::shared_ptr<Scan>&)), this, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), Qt::DirectConnection);
		taskExecutor.enqueue(task);
	}
	catch (...) {
		throw std::exception("Error while finishing scan");
	}
}

void ScanFactory::recalculateScan(std::shared_ptr<Scan>& scan)
{
	std::lock_guard<std::mutex> lock(rangeScanLinesMutex);
	auto task = new ScanProcessingTask(scan);
	connect(task, SIGNAL(newScanReady(const std::shared_ptr<Scan>&)), this, SIGNAL(newScanPublished(const std::shared_ptr<Scan>&)), Qt::DirectConnection);
	taskExecutor.enqueue(task);
}