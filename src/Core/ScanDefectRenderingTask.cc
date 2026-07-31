/*
 * Core/ScanDefectRenderingTask.cc
 */

#include "Core/ScanDefectRenderingTask.hh"
#include <functional>
#include <boost/numeric/interval.hpp>


namespace {

	namespace morphologic{

		typedef boost::numeric::interval<size_t> Interval;


		template <class T, class Func>
		boost::multi_array<T, 2> base(const boost::multi_array<T, 2>& original, Func func)
		{

			/*будем считать,что ядро имеет вид    | 0 1 0 |
			*									  | 1 1 1 |
			*									  | 0 1 0 |
			*/

			boost::multi_array<T, 2> result = original;
			size_t rowsCount = original.size();
			size_t columnsCount = original.begin()->size();

			for(auto i = 0; i < rowsCount; i++){
				for(auto j = 0; j < columnsCount; j++){
					std::vector<T> kernel;
					kernel.push_back(original[i][j]);
					for(auto iK = 0; iK < 3; iK++)							//размеры ядра 3х3
						for(auto jK = 0; jK < 3; jK++)
							if((iK+jK)%2 != 0){								//нечетные элементы матрицы, составляющей ядро
								if((i-1+iK >=0) && (i-1+iK < rowsCount) && (j-1+jK >=0) && (j-1+jK < columnsCount))
									kernel.push_back(original[i-1+iK][j-1+jK]); //смещение центра ядра в исследуемую точку
							}
					result[i][j] = func(kernel.begin(),kernel.end());
				}
			}
			return result;
		}

		inline int32_t fromRgbToInt32(const RgbColor& color)
		{
			int32_t result = 0;
			result = result & color.red; result = result<<8;
			result = result & color.green; result = result<<8;
			result = result & color.blue;

			return result;
		}

		boost::multi_array<RgbColor, 2> erode(const boost::multi_array<RgbColor, 2>& original)
		{
			return base(original,[](std::vector<RgbColor>::iterator begin,std::vector<RgbColor>::iterator end){
				auto minElement = begin;
				for(;begin<end;begin++){
					if(fromRgbToInt32(*begin)<fromRgbToInt32(*minElement))
						minElement = begin;
				}
				return *minElement;
			});
		}

		boost::multi_array<RgbColor, 2> dilate(const boost::multi_array<RgbColor, 2>& original)
		{
			return base(original,[](std::vector<RgbColor>::iterator begin,std::vector<RgbColor>::iterator end){
				auto maxElement = begin;
				for(;begin<end;begin++){
					if(fromRgbToInt32(*begin)>fromRgbToInt32(*maxElement))
						maxElement = begin;
				}
				return *maxElement;
			});
		}
	};


  int rawChannelValue(float x, const DefectChannel& defect)
  {
    //if (x > defect.limit && defect.limit > 0) {
    if (x > defect.limit ) {
      return x * defect.amplification;
    } else {
      return 0;
    }
  }

  int fixedChannelValue(float x, const DefectChannel& defect, int value)
  {
    if ((!defect.useLeastValues && x > defect.limit) || (defect.useLeastValues && x < defect.limit)) {
      return (value);
    } else {
      return 0;
    }
  }

  RgbColor channelValue(float x, const DefectChannel& defect, const DefectChannelRenderingParameters& rendering, uint8_t RgbColor::*mem)
  {
	  if ((!defect.useLeastValues && x > defect.limit) || (defect.useLeastValues && x < defect.limit)) {
      RgbColor c{ 0, 0, 0 };
		  auto raw = x * defect.amplification;
		  raw += rendering.startValue;
		  if (raw > rendering.endValue) {
			auto delta =  rendering.endValue - raw;
			c.red = c.green = c.blue = rendering.lightnessStart + delta;
			c.*mem = rendering.endValue;
		  } else {
			c.*mem = raw;
		  }
		  return c;
    } else {
      return RgbColor{ 0, 0, 0 };
    }
  }

}

DefectsView renderDefectPoints(const DefectKindView& kind, 
                               const std::vector<NormalizedRange>& ranges,
                               const DefectRenderingParameters& rendering)
{
  DefectsView result;
  std::size_t rowCount = 0;
  std::size_t columnCount = 0;
  if (ranges.size() > 0) {
	  rowCount = ranges.front().view.val.shape()[0];
	  columnCount = ranges.front().view.val.shape()[1];
	  result.sampleRate = ranges.front().sampleRate;
	  result.startCoordinate = ranges.front().startCoordinate;
	  result.finalCoordinate = ranges.front().finalCoordinate;
	  result.lineCoordinates = ranges.front().lineCoordinates;

	  result.view.resize(boost::extents[rowCount][columnCount]);
	  for (std::size_t columnIndex = 0; columnIndex < columnCount; columnIndex++)
		  for (std::size_t rowIndex = 0; rowIndex < rowCount; rowIndex++) {
			  auto r = channelValue(ranges[kind.red.range].view.val[rowIndex][columnIndex], kind.red, rendering.red, &RgbColor::red);
			  auto g = channelValue(ranges[kind.green.range].view.val[rowIndex][columnIndex], kind.green, rendering.green, &RgbColor::green);
			  auto b = channelValue(ranges[kind.blue.range].view.val[rowIndex][columnIndex], kind.blue, rendering.blue, &RgbColor::blue);

			  auto sumR = r.red + g.red + b.red;
			  auto sumG = r.green + g.green + b.green;
			  auto sumB = r.blue + g.blue + b.blue;

			  if ((sumR == 0 && sumG == 0) && sumB == 0)
				  result.view[rowIndex][columnIndex] = RgbColor{ 255, 255, 255 };
			  else
				  result.view[rowIndex][columnIndex] = RgbColor{ static_cast<uint8_t>(std::min(sumR, 220)),
																  static_cast<uint8_t>(std::min(sumG, 220)),
																  static_cast<uint8_t>(std::min(sumB, 220)) };
		  }

	  //замыкание
	  result.view = morphologic::dilate(result.view);
	  result.view = morphologic::erode(result.view);
  }
  return result;
}

DefectsView renderDefectPointsWithFixedColor(const DefectKindView& kind, 
                                             const std::vector<NormalizedRange>& ranges,
                                             const DefectRenderingParameters& rendering)
{
  DefectsView result;

  std::size_t rowCount = 0;
  std::size_t columnCount = 0;
  if (ranges.size() > 0) {
	  rowCount = ranges.front().view.val.shape()[0];
	  columnCount = ranges.front().view.val.shape()[1];
	  result.sampleRate = ranges.front().sampleRate;
	  result.startCoordinate = ranges.front().startCoordinate;
	  result.finalCoordinate = ranges.front().finalCoordinate;
	  result.lineCoordinates = ranges.front().lineCoordinates;
	  result.finalLineCoordinates = ranges.front().finalLineCoordinates;

	  result.view.resize(boost::extents[rowCount][columnCount]);
	  for (std::size_t columnIndex = 0; columnIndex < columnCount; columnIndex++)
		  for (std::size_t rowIndex = 0; rowIndex < rowCount; rowIndex++) {
			  uint8_t r = fixedChannelValue(ranges[kind.red.range].view.val[rowIndex][columnIndex], kind.red, rendering.redFixed);
			  uint8_t g = fixedChannelValue(ranges[kind.green.range].view.val[rowIndex][columnIndex], kind.green, rendering.greenFixed);
			  uint8_t b = fixedChannelValue(ranges[kind.blue.range].view.val[rowIndex][columnIndex], kind.blue, rendering.blueFixed);
			  if ((r == 0 && g == 0) && b == 0)
				  result.view[rowIndex][columnIndex] = RgbColor{ 255, 255, 255 };
			  else
				  result.view[rowIndex][columnIndex] = RgbColor{ r, g, b };
		  }

	  //замыкание
	  result.view = morphologic::dilate(result.view);
	  result.view = morphologic::erode(result.view);
  }
  return result;
}

DefectsView selectDefectPoints(const DefectKindView& kind, 
                               const std::vector<NormalizedRange>& ranges)
{
  DefectsView result;
  std::size_t rowCount = 0;
  std::size_t columnCount = 0;
  if (ranges.size() > 0) {
	  rowCount = ranges.front().view.val.shape()[0];
	  columnCount = ranges.front().view.val.shape()[1];
	  result.sampleRate = ranges.front().sampleRate;
	  result.startCoordinate = ranges.front().startCoordinate;
	  result.finalCoordinate = ranges.front().finalCoordinate;
	  result.lineCoordinates = ranges.front().lineCoordinates;
  }
  result.view.resize(boost::extents[rowCount][columnCount]);

  for (std::size_t columnIndex = 0; columnIndex < columnCount; columnIndex++)
    for (std::size_t rowIndex = 0; rowIndex < rowCount; rowIndex++) {
      int r = rawChannelValue(ranges[kind.red.range].view.val[rowIndex][columnIndex], kind.red);
      int g = rawChannelValue(ranges[kind.green.range].view.val[rowIndex][columnIndex], kind.green);
      int b = rawChannelValue(ranges[kind.blue.range].view.val[rowIndex][columnIndex], kind.blue);

      result.view[rowIndex][columnIndex] = RgbColor{ (uint8_t)std::min(r, 255), (uint8_t)std::min(g, 255), (uint8_t)std::min(b, 255) };
    }

  return result;
}
