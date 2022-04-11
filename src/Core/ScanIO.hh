/*
 * Core/ScanIO.hh
 */

#pragma once

#include <QtCore/QString>
#include "Core/ScanData.hh"

void saveToWaveDirectory(const QString& dirname, const Scan& scan);
void saveToTempDirectory(const std::vector<SourceScanLine>& lines);