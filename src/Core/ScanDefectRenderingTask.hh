/*
 * Core/ScanDefectRenderingTask.hh
 */

#pragma once

#include "Core/ScanData.hh"

DefectsView renderDefectPoints(const DefectKindView& kind, 
                               const std::vector<NormalizedRange>& ranges,
                               const DefectRenderingParameters& rendering);

DefectsView renderDefectPointsWithFixedColor(const DefectKindView& kind, 
                                             const std::vector<NormalizedRange>& ranges,
                                             const DefectRenderingParameters& rendering);

DefectsView selectDefectPoints(const DefectKindView& kind, 
                               const std::vector<NormalizedRange>& ranges);