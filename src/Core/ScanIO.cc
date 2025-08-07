/*
 * Core/ScanIO.cc
 */

#include <cstdint>
#include <db_cxx.h>
#include <boost/format.hpp>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QFile>
#include "Core/ScanIO.hh"
#include "PersistentVariable.hh"
#include "Core/CommonXmlSave.hh"
#include "Core/CommonXmlLoad.hh"
#include "Core/ScanDataReflection.hh"

namespace {

  void saveLineToWaveFile(const QString& filename, const SourceScanLine& line)
  {
    QFile out(filename);
    if (! out.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;

    out.write("RIFF", 4);
    auto fileSizePos = out.pos();
    quint32 size = 0;
    out.write(reinterpret_cast<char*>(&size), 4);
    out.write("WAVE", 4);

    // Write fmt chunk
    out.write("fmt ", 4);

    size = 16;
    out.write(reinterpret_cast<char*>(&size), 4);

    quint16 compressionMode = 1; // PCM/uncompressed
    out.write(reinterpret_cast<char*>(&compressionMode), 2);

    quint16 numberOfChannels = 1;
    out.write(reinterpret_cast<char*>(&numberOfChannels), 2);
  
    quint32 sampleRate = line.sampleRate;
    out.write(reinterpret_cast<char*>(&sampleRate), 4);
  
    quint32 avgBytesPerSecond = sampleRate * 2;
    out.write(reinterpret_cast<char*>(&avgBytesPerSecond), 4);
  
    quint16 blockAlign = 2;
    out.write(reinterpret_cast<char*>(&blockAlign), 2);
  
    quint16 significantBitsPerSample = 16;
    out.write(reinterpret_cast<char*>(&significantBitsPerSample), 2);
  
    out.write("data", 4);

    size = 0;
    auto dataSizePos = out.pos();
    out.write(reinterpret_cast<char*>(&size), 4);

    QByteArray wavData;
    for (auto x : line.samples) {
      qint16 ix = x * 0x8000;
      wavData.push_back(ix & 0xFF);
      wavData.push_back((ix >> 8) & 0xFF);
    }
    out.write(wavData);

    size = out.size() - 8;
    out.seek(fileSizePos);
    out.write(reinterpret_cast<char*>(&size), 4);

    size = out.size() - dataSizePos - 4;
    out.seek(dataSizePos);
    out.write(reinterpret_cast<char*>(&size), 4);

    out.close();
  }

}

void saveToTempDirectory(const std::vector<SourceScanLine>& lines)
{
  auto dir = QDir::current();
  if(dir.exists("temp"))
    dir.rmdir("temp");
  dir.mkdir("temp");
  dir.cd("temp");
  Scan scan;
  scan.lines = lines;
  saveToWaveDirectory(dir.absolutePath(), scan);
}


void saveToWaveDirectory(const QString& dirName, const Scan& scan)
{
  namespace urb = uts::reflection::binding;

  //информация о сырых данных хранится в отдельном xml файле. 
  //впоследствии сборка осуществляется согласно этому файлу
  RawScanInfo info;
  auto& lines = scan.lines;
  for (std::size_t i = 0; i < lines.size(); i++) {
    auto fileName = QDir(dirName).filePath(QString("line-%1.wav").arg(i));
    saveLineToWaveFile(fileName, lines[i]);
    info.lines.push_back(RawScanLineInfo{ QString("line-%1.wav").arg(i).toStdString(), lines[i].sampleRate,
        lines[i].startCoordinate, lines[i].finalCoordinate, lines[i].lineCoordinate, lines[i].finalLineCoordinate,
        lines[i].timestampStart, lines[i].timestampEnd,
        lines[i].startCoordinateZ , lines[i].finalCoordinateZ,
                                        });
  }

  PersistentVariable<RawScanInfo> rawScan(info, QDir(dirName).filePath("line.xml").toStdString(), "Raw-Scan");
  rawScan.save();
}