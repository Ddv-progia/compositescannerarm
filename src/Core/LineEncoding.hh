/*
 * Core/LineEncoding.hh
 */

#pragma once

#include <cstdint>

enum class LineEncoding : std::uint32_t {
  Float = 0,
  Int16 = 1,
  WindowsMediaLossless = 2
};
