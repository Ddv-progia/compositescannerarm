/*
 * Core/WMACompression.hh - сжатие данных кодеком Windows Media Audio Lossless
 */

#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>
#include <db_cxx.h>
#include <UCL/Exception.hh>

class WMACodecException : public virtual uts::Exception { };

struct WMACompressedData
{
  std::vector<std::uint8_t> compressedSamples;
  std::vector<std::uint8_t> serializedAttributes;

  WMACompressedData(std::vector<std::uint8_t>&& compressedSamples,
                    std::vector<std::uint8_t>&& serializedAttributes)
    : compressedSamples(std::move(compressedSamples)), serializedAttributes(std::move(serializedAttributes))
  { }
};

WMACompressedData wmaLosslessCompress(const std::vector<float>& xs, unsigned sampleRate);
std::vector<float> wmaLosslessDecompress(const WMACompressedData& compressedData, unsigned sampleRate);
