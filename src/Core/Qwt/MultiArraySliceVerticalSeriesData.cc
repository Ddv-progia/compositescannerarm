/*
 * Core/Qwt/MultiArraySliceVerticalSeriesData.cc
 */

#include "Core/Qwt/MultiArraySliceVerticalSeriesData.hh"
#include <qmessagebox.h>

MultiArraySliceVerticalSeriesData::MultiArraySliceVerticalSeriesData(const boost::multi_array<float, 2>::const_array_view<1>::type& line, 
                                                                     const std::vector<double>& lineCoordinates)
  : line(line), lineCoordinates(lineCoordinates)
{
  //rect = qwtBoundingRect(*this);
  try
  {
      rect = qwtBoundingRect(*this);
  }
  catch (const std::exception&)
  {

  }

}

size_t MultiArraySliceVerticalSeriesData::size() const
{
  return line.size();
}

QPointF MultiArraySliceVerticalSeriesData::sample(size_t i) const
{
    if ((!line.empty()) && (!lineCoordinates.empty())) {
        if ((line.num_elements() > 0) && (line.size() > i) && (lineCoordinates.size() > i)) {
            return QPointF(line[i], lineCoordinates[i]);
            //if(line.begin()!= line.end())        return QPointF(line[i], lineCoordinates[i]);
            //return QPointF();
            
            //if (line[i]) {

            //}
        }

    }
    return QPointF();

    //    return QPointF(line[i], lineCoordinates[i]);
    ////if ((!line.empty())&&(!lineCoordinates.empty())) {
    ////}
    ////else
    ////    return QPointF();
}

QRectF MultiArraySliceVerticalSeriesData::boundingRect() const
{
  return rect;
}
