/*
 * Core/ScanFactory.hh
 */

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>
#include <QtCore/QObject>
#include "boost/optional.hpp"
#include "Core/BackgroundTaskExecutor.hh"
#include "Core/ScanData.hh"
#include "Core/ScanDataMetatypes.hh"

class ScanFactory : public QObject
{
	Q_OBJECT
public:
	explicit ScanFactory(BackgroundTaskExecutor& taskExecutor);

	Q_SLOT void startNewScan(const ProcessingParameters& newProcessingParameters);
	Q_SLOT void addRangeScanLine(const SourceScanLine& newLine);
	Q_SLOT void finishScan(boost::optional<QString &> name = boost::optional<QString &>());
	Q_SLOT void finishScan(std::shared_ptr<Scan>& scan, boost::optional<QString &> name = boost::optional<QString &>());
	Q_SLOT void finishScan(std::shared_ptr<ScanArm> scan);
	Q_SLOT void recalculateScan(std::shared_ptr<Scan>& scan);
	Q_SLOT void recalculateScan(std::shared_ptr<ScanArm>& scan);

protected:
	Q_SIGNAL void newScanPublished(const std::shared_ptr<Scan>& scan);
	Q_SIGNAL void newScanPublished(const std::shared_ptr<ScanArm>& scan);

private:
	BackgroundTaskExecutor& taskExecutor;
	ProcessingParameters processingParameters;

	std::mutex rangeScanLinesMutex;
	std::vector<SourceScanLine> rangeScanLines;
};
