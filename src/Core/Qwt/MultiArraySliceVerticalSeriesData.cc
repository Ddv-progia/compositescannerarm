/*
 * Core/Qwt/MultiArraySliceVerticalSeriesData.cc
 */

#include <windows.h>
#include <stdio.h>
//#include <excpt.h>
#include <iostream>
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

int filterException(int code, PEXCEPTION_POINTERS ex) {
    std::cout << "Filtering " << std::hex << code << std::endl;
    return EXCEPTION_EXECUTE_HANDLER;
}

//int filterException2(unsigned int code, struct _EXCEPTION_POINTERS* ep)
//{
//    puts("in filter.");
//    if (code == EXCEPTION_ACCESS_VIOLATION)
//    {
//        puts("caught AV as expected.");
//        return EXCEPTION_EXECUTE_HANDLER;
//    }
//    else
//    {
//        puts("didn't catch AV, unexpected.");
//        return EXCEPTION_CONTINUE_SEARCH;
//    };
//}
                //typedef struct _EXCEPTION_POINTERS {
                //    PEXCEPTION_RECORD ExceptionRecord;
                //    PCONTEXT ContextRecord;
                //} EXCEPTION_POINTERS, * PEXCEPTION_POINTERS;

QPointF MultiArraySliceVerticalSeriesData::sample(size_t i) const
{
    if ((!line.empty()) && (!lineCoordinates.empty())) {
        if ((line.num_elements() > 0) && (line.size() > i) && (lineCoordinates.size() > i)) {
            qreal x, y;
            bool allOk = true;
            __try
            {
                x = line[i];
            }
            //__except (filterException2(GetExceptionCode(), GetExceptionInformation())) {
            __except (filterException(GetExceptionCode(), GetExceptionInformation())) {
                std::cout << "caught x = line[i];" << std::endl;
                allOk = false;
            }
                y = lineCoordinates[i];
            //try
            //{
            //    y = lineCoordinates[i];
            //}
            //catch (const std::exception&)
            //{
            //    allOk = false;
            //}
            if (allOk) return QPointF(x,y);

            //return QPointF(line[i], lineCoordinates[i]);
            ////if(line.begin()!= line.end())        return QPointF(line[i], lineCoordinates[i]);
            ////return QPointF();
            ////if (line[i]) {
            ////}
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
