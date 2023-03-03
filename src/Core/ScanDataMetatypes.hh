/*
 * Core/ScanDataMetatypes.hh
 */

#pragma once

#include <memory>
#include <QtCore/QMetaType>

#include "Core/ScanData.hh"

Q_DECLARE_METATYPE(SourceScanLine)
Q_DECLARE_METATYPE(std::shared_ptr<Scan>)
Q_DECLARE_METATYPE(std::vector<float>)