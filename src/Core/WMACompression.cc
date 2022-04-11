/*
 * Core/WMACompression.cc - сжатие данных кодеком Windows Media Audio Lossless
 */

#include <iterator>
#include <boost/format.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/scope_exit.hpp>

#include <ObjBase.h>
#include <comdef.h>
#include <mfapi.h>
#include <Mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mftransform.h>
#include <Propvarutil.h>
#include <wmcodecdsp.h>

#include "Core/WMACompression.hh"

_COM_SMARTPTR_TYPEDEF(IMFMediaBuffer, __uuidof(IMFMediaBuffer));
_COM_SMARTPTR_TYPEDEF(IMFMediaEvent, __uuidof(IMFMediaEvent));
_COM_SMARTPTR_TYPEDEF(IMFMediaType, __uuidof(IMFMediaType));
_COM_SMARTPTR_TYPEDEF(IMFSample, __uuidof(IMFSample));
_COM_SMARTPTR_TYPEDEF(IMFTransform, __uuidof(IMFTransform));
_COM_SMARTPTR_TYPEDEF(IPropertyStore, __uuidof(IPropertyStore));

using boost::format;

namespace {

  const std::uint32_t ENCODER_OUTPUT_BUFFER_SIZE = 4096 * 1024;
  const std::uint32_t ENCODER_INPUT_BUFFER_SIZE = 4096 * 1024;
  const std::uint32_t ENCODER_INPUT_BUFFER_SAMPLE_COUNT = ENCODER_INPUT_BUFFER_SIZE / sizeof(std::uint32_t);

  const std::uint32_t DECODER_INPUT_BUFFER_SIZE = 4096 * 1024;
  const std::uint32_t DECODER_OUTPUT_BUFFER_SIZE = 4096 * 1024;

  IMFTransformPtr createWMAEncoder()
  {
    IMFTransformPtr wmaEncoder;
    HRESULT hr = wmaEncoder.CreateInstance(CLSID_CWMAEncMediaObject, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(hr)) {
      switch (hr) {
      case REGDB_E_CLASSNOTREG:
        BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Класс энкодера Window Media Audio не зарегистрирован"));
      default:
        BOOST_THROW_EXCEPTION(WMACodecException() 
          << uts::ErrInfo_Description((format("Ошибка создания энкодера Window Media Audio: %x") % hr).str()));
      }
    }

    return wmaEncoder;
  }

  void setupVBREncoding(IMFTransformPtr encoder)
  {
    IPropertyStorePtr encoderPropertires;
    HRESULT hr = encoder.QueryInterface(IID_IPropertyStore, &encoderPropertires);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Энкодер Window Media Audio не поддерживает интерфейс IPropertyStore"));
    }

    PROPVARIANT vbren;
    InitPropVariantFromBoolean(TRUE, &vbren);
    hr = encoderPropertires->SetValue(MFPKEY_VBRENABLED, vbren);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Ошибка включения кодирования с переменным битрейтом"));
    }

    PROPVARIANT vbrqual;
    InitPropVariantFromUInt32(100, &vbrqual);
    hr = encoderPropertires->SetValue(MFPKEY_DESIRED_VBRQUALITY, vbrqual);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Ошибка задания качества кодирования кодирования"));
    }

    PROPVARIANT encComplexity;
    InitPropVariantFromUInt32(100, &encComplexity);
    hr = encoderPropertires->SetValue(MFPKEY_ENCCOMPLEXITY, encComplexity);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Ошибка задания сложности алгоритма кодирования"));
    }
  }

  IMFMediaTypePtr findEncoderOutputType(IMFTransformPtr encoder, unsigned sampleRate)
  {
    DWORD outputMediaTypeIndex = 0;
    IMFMediaTypePtr outputMediaType;
    HRESULT hr;

    while (SUCCEEDED(hr = encoder->GetOutputAvailableType(0, outputMediaTypeIndex, &outputMediaType))) {
      GUID subtype;
      hr = outputMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
      if (FAILED(hr)) continue;

      UINT32 samplesPerSecond;
      hr = outputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
      if (FAILED(hr)) continue;

      UINT32 numChannels;
      hr = outputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
      if (FAILED(hr)) continue;

      if ((subtype == MFAudioFormat_WMAudio_Lossless) && (numChannels == 2) && (samplesPerSecond == sampleRate)) {
        return outputMediaType;
      }

      outputMediaTypeIndex++;
    }

    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Не найден тип сжатия без потерь"));
  }

  IMFMediaTypePtr findEncoderInputType(IMFTransformPtr encoder, unsigned sampleRate)
  {
    HRESULT hr;
    DWORD inputMediaTypeIndex = 0;
    IMFMediaTypePtr inputMediaType;
    while (SUCCEEDED(encoder->GetInputAvailableType(0, inputMediaTypeIndex, &inputMediaType))) {
      GUID subtype;
      hr = inputMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
      if (FAILED(hr)) continue;

      UINT32 samplesPerSecond;
      hr = inputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
      if (FAILED(hr)) continue;

      UINT32 numChannels;
      hr = inputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
      if (FAILED(hr)) continue;

      UINT32 bitsPerSample;
      hr = inputMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
      if (FAILED(hr)) continue;

      if ((subtype == MFAudioFormat_PCM) && (samplesPerSecond == sampleRate) && (bitsPerSample == 32)) {
        return inputMediaType;
      }

      inputMediaTypeIndex++;
    }

    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Не найден тип PCM 32 бит"));
  }

  void processAvailableEncoderOutput(IMFTransformPtr encoder, IMFSamplePtr outputSample, IMFMediaBufferPtr outputBuffer, 
                                     std::vector<std::uint8_t>& compressedData)
  {
    DWORD status;

    MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
    outputDataBuffer.dwStreamID = 0;
    outputDataBuffer.dwStatus = 0;
    outputDataBuffer.pEvents = NULL;
    outputDataBuffer.pSample = outputSample;

    HRESULT hr;
    while (SUCCEEDED(hr = encoder->ProcessOutput(0, 1, &outputDataBuffer, &status))) {
      BYTE* buf;
      DWORD maxLen, curLen;
      outputBuffer->Lock(&buf, &maxLen, &curLen);
      std::copy(buf, buf + curLen, std::back_inserter(compressedData));
      outputBuffer->Unlock();
    }

    if (FAILED(hr) && (hr != MF_E_TRANSFORM_NEED_MORE_INPUT)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка обработки выходного потока: %x") % hr).str()));
    }
  }

  bool processEncoderInputSample(IMFTransformPtr encoder, IMFSamplePtr inputSample, IMFMediaBufferPtr inputBuffer,
                                 std::vector<std::int32_t>::const_iterator sampleBegin, 
                                 std::vector<std::int32_t>::const_iterator sourceEnd)
  {
    std::int32_t* buf;
    DWORD maxLen, curLen;
    inputBuffer->Lock(reinterpret_cast<BYTE**>(&buf), &maxLen, &curLen);

    auto lastFrame = false;
    if (sourceEnd - sampleBegin >= ENCODER_INPUT_BUFFER_SAMPLE_COUNT) {
      std::copy(sampleBegin, sampleBegin + ENCODER_INPUT_BUFFER_SAMPLE_COUNT, buf);
      inputBuffer->SetCurrentLength(ENCODER_INPUT_BUFFER_SIZE);
    } else {
      std::copy(sampleBegin, sourceEnd, buf);
      inputBuffer->SetCurrentLength((sourceEnd - sampleBegin) * sizeof(std::int32_t));
      lastFrame = true;
    }

    inputBuffer->Unlock();

    HRESULT hr = encoder->ProcessInput(0, inputSample, 0);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка обработки входного потока: %x") % hr).str()));
    }
    return lastFrame;
  }

  std::vector<std::uint8_t> compressData(const std::vector<float>& xs, IMFTransformPtr encoder) 
  {
    HRESULT hr;

    std::vector<std::int32_t> src;
    boost::transform(xs, std::back_inserter(src), [] (float x) { return std::int32_t(x * 0x8000) << 16; });
  
    IMFSamplePtr outputSample;
    hr = MFCreateSample(&outputSample);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания выходной выборки: %x") % hr).str()));
    }

    IMFMediaBufferPtr outputBuffer;
    hr = MFCreateMemoryBuffer(ENCODER_OUTPUT_BUFFER_SIZE, &outputBuffer);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания выходного буфера: %x") % hr).str()));
    }
    outputSample->AddBuffer(outputBuffer);

    IMFSamplePtr inputSample;
    hr = MFCreateSample(&inputSample);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания входной выборки: %x") % hr).str()));
    }

    IMFMediaBufferPtr inputBuffer;
    hr = MFCreateMemoryBuffer(ENCODER_INPUT_BUFFER_SIZE, &inputBuffer);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания входного буфера: %x") % hr).str()));
    } 
    inputSample->AddBuffer(inputBuffer);

    std::vector<std::uint8_t> compressedData;

    hr = encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка начала передачи потока данных: %x") % hr).str()));
    }
    BOOST_SCOPE_EXIT(encoder) { encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0); } BOOST_SCOPE_EXIT_END;

    hr = encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка начала первого потока данных: %x") % hr).str()));
    }
    BOOST_SCOPE_EXIT(encoder) { 
      encoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
      encoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
    } BOOST_SCOPE_EXIT_END;

    processAvailableEncoderOutput(encoder, outputSample, outputBuffer, compressedData);

    for (auto i = src.begin(); i < src.end(); i += ENCODER_INPUT_BUFFER_SAMPLE_COUNT) {
      auto lastFrame = processEncoderInputSample(encoder, inputSample, inputBuffer, i, src.end());
      processAvailableEncoderOutput(encoder, outputSample, outputBuffer, compressedData);

      if (lastFrame) break;
    }

    encoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0);
    processAvailableEncoderOutput(encoder, outputSample, outputBuffer, compressedData);

    return compressedData;
  }
}

WMACompressedData wmaLosslessCompress(const std::vector<float>& xs, unsigned sampleRate)
{
  HRESULT hr;

  auto wmaEncoder = createWMAEncoder();
  setupVBREncoding(wmaEncoder);
  
  auto outputType = findEncoderOutputType(wmaEncoder, sampleRate);
  hr = wmaEncoder->SetOutputType(0, outputType, 0);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка задания выходного типа энкодера: %x") % hr).str()));
  }

  auto inputType = findEncoderInputType(wmaEncoder, sampleRate);
  hr = wmaEncoder->SetInputType(0, inputType, 0);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка задания входного типа энкодера: %x") % hr).str()));
  }

  UINT32 attributesBufferSize;
  hr = MFGetAttributesAsBlobSize(outputType, &attributesBufferSize);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка определения размера буфера сериализованных аттрибутов энкодера: %x") % hr).str()));
  }

  std::vector<std::uint8_t> serializedAttributes(attributesBufferSize);
  hr = MFGetAttributesAsBlob(outputType, serializedAttributes.data(), attributesBufferSize);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка сериализации аттрибутов энкодера: %x") % hr).str()));
  }

  return WMACompressedData(compressData(xs, wmaEncoder), std::move(serializedAttributes));
}

namespace {

  IMFTransformPtr createWMADecoder()
  {
    IMFTransformPtr wmaDecoder;
    HRESULT hr = wmaDecoder.CreateInstance(CLSID_CWMADecMediaObject, NULL, CLSCTX_INPROC_SERVER);
    if (FAILED(hr)) {
      switch (hr) {
      case REGDB_E_CLASSNOTREG:
        BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Класс декодера Window Media Audio не зарегистрирован"));
      default:
        BOOST_THROW_EXCEPTION(WMACodecException() 
          << uts::ErrInfo_Description((format("Ошибка создания декодера Window Media Audio: %x") % hr).str()));
      }
    }

    return wmaDecoder;
  }

  void setupDecoderInputType(IMFTransformPtr decoder, const WMACompressedData& compressedData)
  {
    IMFMediaTypePtr inputMediaType;
    HRESULT hr = MFCreateMediaType(&inputMediaType);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания входного типа декодера: %x") % hr).str()));
    }

    hr = MFInitAttributesFromBlob(inputMediaType, compressedData.serializedAttributes.data(), compressedData.serializedAttributes.size());
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка десириализации аттрибутов входного типа декодера: %x") % hr).str()));
    }

    hr = decoder->SetInputType(0, inputMediaType, 0);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка задания входного типа декодера: %x") % hr).str()));
    }
  }

  IMFMediaTypePtr findDecoderOutputType(IMFTransformPtr decoder, unsigned sampleRate)
  {
    HRESULT hr;
    DWORD outputMediaTypeIndex = 0;
    IMFMediaTypePtr outputMediaType;
    while (SUCCEEDED(hr = decoder->GetOutputAvailableType(0, outputMediaTypeIndex, &outputMediaType))) {
      GUID subtype;
      hr = outputMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
      if (FAILED(hr)) continue;

      UINT32 samplesPerSecond;
      hr = outputMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);
      if (FAILED(hr)) continue;

      UINT32 numChannels;
      hr = outputMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
      if (FAILED(hr)) continue;

      UINT32 bitsPerSample;
      hr = outputMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
      if (FAILED(hr)) continue;

      if ((subtype == MFAudioFormat_Float) && (numChannels == 2) && (samplesPerSecond == sampleRate)) {
        return outputMediaType;
      }

      outputMediaTypeIndex++;
    }

    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description("Не найден тип PCM Float"));
  }

  void processAvailableDecoderOutput(IMFTransformPtr decoder, IMFSamplePtr outputSample, IMFMediaBufferPtr outputBuffer, 
                                     std::vector<float>& decompressedData)
  {
    HRESULT hr;
    DWORD status;

    MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
    outputDataBuffer.dwStreamID = 0;
    outputDataBuffer.dwStatus = 0;
    outputDataBuffer.pEvents = NULL;
    outputDataBuffer.pSample = outputSample;

    while (SUCCEEDED(hr = decoder->ProcessOutput(0, 1, &outputDataBuffer, &status))) { 
      BYTE* buf;
      DWORD maxLen, curLen;
      outputBuffer->Lock(&buf, &maxLen, &curLen);

      auto floatBuf = reinterpret_cast<float*>(buf);
      std::copy(floatBuf, floatBuf + curLen / sizeof(float), std::back_inserter(decompressedData));
      outputBuffer->Unlock();
    }

    if (FAILED(hr) && hr != MF_E_TRANSFORM_NEED_MORE_INPUT) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка обработки выходного потока: %x") % hr).str()));
    }
  }

  bool processDecoderInput(IMFTransformPtr decoder, IMFSamplePtr inputSample, IMFMediaBufferPtr inputBuffer, 
                           std::vector<std::uint8_t>::const_iterator sampleBegin,
                           std::vector<std::uint8_t>::const_iterator sourceEnd)
  {
    std::uint8_t* buf;
    DWORD maxLen, curLen;
    inputBuffer->Lock(&buf, &maxLen, &curLen);

    auto lastFrame = false;
    if (sourceEnd - sampleBegin >= DECODER_INPUT_BUFFER_SIZE) {
      std::copy(sampleBegin, sampleBegin + DECODER_INPUT_BUFFER_SIZE, buf);
      inputBuffer->SetCurrentLength(DECODER_INPUT_BUFFER_SIZE);
    } else {
      auto distance = std::distance(sampleBegin, sourceEnd);
      std::copy(sampleBegin, sourceEnd, buf);
      inputBuffer->SetCurrentLength((sourceEnd - sampleBegin));
      lastFrame = true;
    }

    inputBuffer->Unlock();

    HRESULT hr = decoder->ProcessInput(0, inputSample, 0);
    if (FAILED(hr)) {
      BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка обработки входного потока: %x") % hr).str()));
    }
    return lastFrame;
  }
}

std::vector<float> wmaLosslessDecompress(const WMACompressedData& compressedData, unsigned sampleRate)
{
  HRESULT hr;

  auto wmaDecoder = createWMADecoder();
  setupDecoderInputType(wmaDecoder, compressedData);
  
  auto outputType = findDecoderOutputType(wmaDecoder, sampleRate);
  hr = wmaDecoder->SetOutputType(0, outputType, 0);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка задания выходного типа декодера: %x") % hr).str()));
  }
  
  IMFSamplePtr inputSample;
  hr = MFCreateSample(&inputSample);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания входной выборки: %x") % hr).str()));
  }

  IMFMediaBufferPtr inputBuffer;
  hr = MFCreateMemoryBuffer(DECODER_INPUT_BUFFER_SIZE, &inputBuffer);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания входного буфера: %x") % hr).str()));
  }
  inputSample->AddBuffer(inputBuffer);

  IMFSamplePtr outputSample;
  hr = MFCreateSample(&outputSample);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания выходной выборки: %x") % hr).str()));
  }

  IMFMediaBufferPtr outputBuffer;
  hr = MFCreateMemoryBuffer(DECODER_OUTPUT_BUFFER_SIZE, &outputBuffer);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка создания выходного буфера: %x") % hr).str()));
  }
  outputSample->AddBuffer(outputBuffer);

  std::vector<float> decompressedData;

  hr = wmaDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка начала передачи потока данных: %x") % hr).str()));
  }
  BOOST_SCOPE_EXIT(wmaDecoder) { wmaDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0); } BOOST_SCOPE_EXIT_END;

  hr = wmaDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
  if (FAILED(hr)) {
    BOOST_THROW_EXCEPTION(WMACodecException() << uts::ErrInfo_Description((format("Ошибка начала первого потока данных: %x") % hr).str()));
  }
  BOOST_SCOPE_EXIT(wmaDecoder) { 
    wmaDecoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    wmaDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
  } BOOST_SCOPE_EXIT_END;

  processAvailableDecoderOutput(wmaDecoder, outputSample, outputBuffer, decompressedData);

  for (auto i = compressedData.compressedSamples.begin(); i < compressedData.compressedSamples.end(); i += DECODER_INPUT_BUFFER_SIZE) {
    auto lastFrame = processDecoderInput(wmaDecoder, inputSample, inputBuffer, i, compressedData.compressedSamples.end());
    processAvailableDecoderOutput(wmaDecoder, outputSample, outputBuffer, decompressedData);

    if (lastFrame) break;
  }
  wmaDecoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0);
  processAvailableDecoderOutput(wmaDecoder, outputSample, outputBuffer, decompressedData);

  return decompressedData;
}