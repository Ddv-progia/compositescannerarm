/*
 * Gui/StandardColorMap.hh
 */

#pragma once

#include <qwt_color_map.h>

class StandardColorMap : public QwtLinearColorMap
{
public:
  StandardColorMap() 
    : QwtLinearColorMap(Qt::darkBlue, Qt::red)
  {
    addColorStop(0.1, Qt::blue);
    addColorStop(0.35, Qt::green);
  }
};

class FixedColorMap : public QwtColorMap
{
  QwtInterval globalInterval;
  QwtLinearColorMap* linearMap;
public:
  FixedColorMap(std::vector<ColorStop> stopsList)
  {
    linearMap = new QwtLinearColorMap(Qt::blue,Qt::red);
    double nullPoint = 0.0;
    
    if(!stopsList.empty()){
      double max = stopsList.begin()->val;
      double min = stopsList.begin()->val;

      std::for_each(stopsList.begin(),stopsList.end(),
        [&](ColorStop stop){if(stop.val>max) max = stop.val; if(stop.val<min) min = stop.val;});

      if((min*max)<0){
        nullPoint = std::abs(min)/std::abs(max-min);
      }
      else
        nullPoint = std::abs(min);

      for(auto& stop : stopsList)
        linearMap->addColorStop(nullPoint + stop.val/std::abs(max-min),QColor(QString::fromStdString(stop.color)));
      globalInterval.setInterval(min,max);
    } else {
      globalInterval.setInterval(-1,1);
    }
  }

  QRgb rgb(const QwtInterval &interval, double value) const
  {
    return linearMap->rgb(globalInterval,value);
  }
  unsigned char colorIndex (const QwtInterval &interval, double value) const
  {
    //return linearMap->colorIndex(globalInterval, value);
    return linearMap->colorIndex(0,globalInterval,value);
  }
};
