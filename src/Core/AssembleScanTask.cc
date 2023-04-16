/*
 * Core/AssembleScanTask.cc
 */

#include <cstdint>
#include <boost/scope_exit.hpp>

#include <comdef.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Propvarutil.h>
#include <regex>
#include <QFile>
#include <QDir>
//#include <UCL/Reflection/Binding/ElementNameTransformer.hh>
//#include <UCL/Reflection/Binding/QtXml/FromStream/FromStream.hh>
//#include <UCL/Reflection/Binding/QtXml/FromStream/ReflectedStructure.hh>
//#include <UCL/Reflection/Binding/StringConversion.hh>
//#include <UCL/Reflection/Binding/QtXml/FromStream/StandardSequence.hh>
#include "CommonXmlLoad.hh"
//#include "Core/ScanDataReflection.hh"
#include "PersistentVariable.hh"


#include "Core/AssembleScanTask.hh"

_COM_SMARTPTR_TYPEDEF(IMFMediaType, __uuidof(IMFMediaType));
_COM_SMARTPTR_TYPEDEF(IMFSourceReader, __uuidof(IMFSourceReader));

namespace {

  SourceScanLine loadWavFile(const QString& filename)
  {
    IMFSourceReaderPtr reader;
    HRESULT res;
  
    if (FAILED((res = MFCreateSourceReaderFromURL(reinterpret_cast<LPCWSTR>(filename.utf16()), NULL, &reader)))) {
      throw QString("Невозможно прочитать файл %1: %2").arg(filename).arg(_com_error(res).ErrorMessage());
    }

    IMFMediaTypePtr mediaType;
    if (FAILED((res = reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &mediaType)))) {
      throw QString("Невозможно прочитать файл %1: %2").arg(filename).arg(_com_error(res).ErrorMessage());
    }

    UINT32 numberOfChannels;
    mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numberOfChannels);

    UINT32 sampleRate;
    mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);

    //if (numberOfChannels > 1) {
    //  throw QString("Неподдерживаемое количество каналов: %1").arg(numberOfChannels);
    //}

    UINT32 bitsPerSample;
    mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

    if (bitsPerSample != 16) {
      throw QString("Неподдерживаемое количество бит на выборку: %1").arg(bitsPerSample);
    }

    UINT32 samplesPerSecond;
    mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond);

    std::vector<float> result;
    do {
      DWORD actualIndex;
      DWORD streamFlags;
      LONGLONG timestamp;
      IMFSample* sample;

      reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &actualIndex, &streamFlags, &timestamp, &sample);
      if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) break;

      IMFMediaBuffer* mediaBuffer;
      sample->ConvertToContiguousBuffer(&mediaBuffer);

      BYTE* buffer;
      DWORD maxLength;
      DWORD currentLength;
      mediaBuffer->Lock(&buffer, &maxLength, &currentLength);

      result.reserve(result.size() + currentLength / sizeof(INT16));
      for (UINT32 i = 0; i < currentLength / sizeof(INT16); i++) {
        result.push_back(float(*(reinterpret_cast<INT16*>(buffer) + i)) / float(0x8000));
      }

      mediaBuffer->Unlock();
      mediaBuffer->Release();

      sample->Release();
      sample = 0;
    } while (true);

    return SourceScanLine{ std::move(result), sampleRate, 0.0, 0.0, -1.0 };
  }

}

AssembleScanTask::AssembleScanTask(const std::vector<QString>& sourceFilenames,
                                   ScanFactory& scanFactory,
                                   ProcessingParameters const& params)
                                   : sourceFilenames(sourceFilenames), scanFactory(scanFactory), params(params)
{ }

void AssembleScanTask::operator()()
{
  /*namespace urb = uts::reflection::binding;
  
  try {
    //вариант для xml
    auto text = sourceFilenames[0].toStdString();
    if(std::regex_match(text,std::regex("(.*)(xml)"))){
      emit started("Загрузка сборок", sourceFilenames.size());
      
      for(auto& name : sourceFilenames){
        auto dir = QDir(name);
        dir.cdUp();
        auto dirName = dir.absolutePath();
        PersistentVariable<RawScanInfo> rawScan(name.toStdString(), "Raw-Scan");
        rawScan.load();
        //может быть загружено несколько сканов
        scanFactory.startNewScan(params);
        emit stageStarted("Загрузка исходных файлов", rawScan->lines.size() - 1);
        for (auto& line : rawScan->lines){
          auto path = dirName + '/' + QString::fromStdString(line.fileName);
          if(!QFile(path).exists())
           continue;
          auto rawLine = loadWavFile(path);
          rawLine.finalCoordinate = line.finalCoordinate;
          rawLine.lineCoordinate = line.lineCoordinate;
          rawLine.sampleRate = line.sampleRate;
          rawLine.startCoordinate = line.startCoordinate;
        
          scanFactory.addRangeScanLine(rawLine);
          
          emit stageProgressed();
        }
        auto scanName = sourceFilenames.begin()->section("-",0);
        scanFactory.finishScan(boost::optional<QString &>(scanName));
      }
      emit finished();
    //вариант для обычных wav
    } else {
      scanFactory.startNewScan(params);
      emit started("Сборка скана", 1);
      emit stageStarted("Загрузка исходных файлов", sourceFilenames.size() - 1);
      for (auto i = sourceFilenames.begin(); i != sourceFilenames.end(); ++i) {
        scanFactory.addRangeScanLine(loadWavFile(*i));
        emit stageProgressed();
      }
     emit finished();
     auto scanName = sourceFilenames.begin()->section("-",0);
     scanFactory.finishScan(boost::optional<QString &>(scanName));
    }
  } catch (QString& err) {
    emit terminated(err);
    return;
  }*/
}