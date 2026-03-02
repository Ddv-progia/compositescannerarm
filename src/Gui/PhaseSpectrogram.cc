#include "Gui/PhaseSpectrogram.hh"
#include "qwt.h"
#include "qwt_plot.h"
#include "qwt_scale_engine.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_layout.h"
#include "qwt_legend.h"
#include "qwt_legend_label.h"


#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <QGridLayout>
#include <QPushButton>
#include <QLayout>
#include <QFile>

#include "UCL/SignalProcessing/RankFilter.hh"
#include "UCL/RegressionAnalysis/LeastSquares.hh"
#include <QtGui/qpen.h>

//#include <amp.h>

PhaseSpectorgram::PhaseSpectorgram(const boost::multi_array<std::complex<double>, 2>&  newData, const std::vector<qreal>& newFrequencyData,QWidget* parent):data(newData),frequencyData(newFrequencyData),QDialog(parent)
  {
    QPushButton* forward = new QPushButton(QString("->"));
    QPushButton* backward = new QPushButton(QString("<-"));
    auto layout = new QGridLayout;
    auto buttonsLayout = new QHBoxLayout;

    

    plot = new QwtPlot;
    plot->setCanvasBackground(Qt::white);
    plot->enableAxis(QwtPlot::yLeft);
    plot->enableAxis(QwtPlot::xBottom);
    plot->plotLayout()->setCanvasMargin(-1);
    plot->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating, true);

    curve = new QwtPlotCurve;
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve->setPen(QPen(Qt::black));
    curve->attach(plot);

    originalCurve = new QwtPlotCurve;
    originalCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    originalCurve->setPen(QPen(Qt::darkGreen,4));
    originalCurve->attach(plot);
    originalCurve->setStyle(QwtPlotCurve::CurveStyle::Dots);

    deviationsCurve = new QwtPlotCurve;
    deviationsCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    deviationsCurve->setPen(QPen(Qt::red));
    deviationsCurve->attach(plot);

    koeffsCurve = new QwtPlotCurve;
    koeffsCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    koeffsCurve->setPen(QPen(Qt::blue));
    koeffsCurve->attach(plot);

    auto grid = new QwtPlotGrid;
    grid->setPen(QPen(Qt::DotLine));
    grid->attach(plot);

	
    zoom = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft,plot->canvas());
    zoom->setRubberBandPen(QPen(Qt::black));


    buttonsLayout->addWidget(backward);
    buttonsLayout->addWidget(forward);

    layout->addWidget(plot,0,1);
    layout->setRowStretch(1,0);
    layout->setColumnStretch(0,0);

    layout->addLayout(buttonsLayout,2,1);

    connect(forward,SIGNAL(clicked()),SLOT(stepForward()));
    connect(backward,SIGNAL(clicked()),SLOT(stepBackward()));

    setLayout(layout);

    current = data.begin();
    currentIndex = 0;
    recalculatePlot();
  }

double nearest(double base,double first, double second)
{
  return std::abs(base - first)<=std::abs(base - second)? first : second;
}

//функция апроксимирует кривую,описанную yVals с помощью регрессии. в функцию передаются параметры:
//step - число точек для построения регрессии, overlap - перекрытие(общие точки у соседних регрессий)
//функция вычисляет значения рекурсивно, двигаясь по вектору на участки step, строит на каждом этапе промежуточный мектор размера step-overlap
QVector<double> findApproximateCurve(const std::vector<double>& yVals,const std::vector<double>& xVals, int step = 60, int overlap=25)
{
  QVector<double> result;
  if(step<=overlap)
    return result;
  if(xVals.size()<=step){             //если вектор короче либо равен числу шагов,происходит построение регрессии
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(xVals,yVals,1);
    if(xVals.size()<=2*overlap){
      for(auto& x : xVals)
        result.push_back(coeffs[1]*x+coeffs[0]);
    }
    else
      for(auto iter = xVals.begin();iter<xVals.end()-(overlap);iter++)
        result.push_back(coeffs[1]*(*iter)+coeffs[0]);
  }else{                     //иначе, нарезаем вектор на части и анализируем их
    auto firstPart = findApproximateCurve(std::vector<double> (yVals.begin(),yVals.begin()+step),std::vector<double> (xVals.begin(),xVals.begin()+step),step,overlap);
    std::copy(firstPart.begin(),firstPart.end(),std::back_inserter(result));
    auto secondPart = findApproximateCurve(std::vector<double> (yVals.begin()+step-overlap,yVals.end()),std::vector<double> (xVals.begin()+step-overlap,xVals.end()),step,overlap);
    std::copy(secondPart.begin(),secondPart.end(),std::back_inserter(result));
  }
  return result;
}

struct CurveParams
{
  double averageDeviation;
  double maximumDeviation;
  double curveCoeff;
  CurveParams():averageDeviation(0.0), maximumDeviation(0.0), curveCoeff(0.0){}
};

//функция в каждой точке исходной кривой строит регрессию используя 2*border значений и по регрессии находит ожидаемое положение точки
//в переменную params запишутся параметры кривой: среднее и максимальное отклонения от исходного сигнала, коэффициент наклона
QVector<double> findFilteredByRegressionCurve(std::vector<double>& yVals, std::vector<double>& xVals, int border, CurveParams params = CurveParams{})
{
  QVector<double> result;
  namespace ba=boost::accumulators;
  ba::accumulator_set<double, ba::stats<ba::tag::mean>> coeffAcc;
  ba::accumulator_set<double, ba::stats<ba::tag::mean>> deviationAcc;
  size_t currentElement = 0;
  if(2*border>yVals.size() || yVals.size()!=xVals.size())
    return result;

  auto fill = [&](std::vector<qreal>& coeffs){coeffAcc(coeffs[1]);
                                              result.push_back(coeffs[1]*xVals[currentElement]+coeffs[0]);
                                              auto deviation = std::abs(result.back()-yVals[currentElement]);
                                              deviationAcc(deviation);
                                              if(deviation>params.maximumDeviation)  params.maximumDeviation = deviation;};

  for(;currentElement<border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin(),xVals.begin()+(border+currentElement)),
      std::vector<double>(yVals.begin(),yVals.begin()+(border+currentElement)),1);
    fill(coeffs);
  }

  for(;currentElement<yVals.size()-border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.begin()+(currentElement+border)),
      std::vector<double>(yVals.begin()+(currentElement-border),yVals.begin()+(currentElement+border)),1);
    fill(coeffs);
  }

  for(;currentElement<yVals.size();currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.end()),
      std::vector<double>(yVals.begin()+(currentElement-border),yVals.end()),1);
    fill(coeffs);
  }

  params.averageDeviation = ba::mean(deviationAcc);
  params.curveCoeff = ba::mean(coeffAcc);

  return result;
}


QVector<double> findCurveAngles(std::vector<double>& yVals, std::vector<double>& xVals, int border)//работает очень медленно
{
  QVector<double> result;
  size_t currentElement = 0;
  if(2*border>yVals.size() || yVals.size()!=xVals.size())
    return result;


  for(;currentElement<border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin(),xVals.begin()+(border+currentElement)),
      std::vector<double>(yVals.begin(),yVals.begin()+(border+currentElement)),1);
    result.push_back(coeffs[1]*90);
  }

  for(;currentElement<yVals.size()-border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.begin()+(currentElement+border)),
      std::vector<double>(yVals.begin()+(currentElement-border),yVals.begin()+(currentElement+border)),1);
    result.push_back(coeffs[1]*90);
  }

  for(;currentElement<yVals.size();currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.end()),
      std::vector<double>(yVals.begin()+(currentElement-border),yVals.end()),1);
    result.push_back(coeffs[1]*90);
  }

  uts::dsp::RankFilter<double> filter{ 40,2,36 };

  auto beginIterator = result.begin();
  if(result.size()<=41)
    return result;
  auto endIterator = beginIterator + 40;
  QVector<double> filteredResult;

  for(;endIterator!=result.end();beginIterator++,endIterator++){
    auto sorted = std::vector<double>(beginIterator,endIterator);
    std::sort(sorted.rbegin(),sorted.rend()); 
    filter.setInitialState(sorted.begin(),sorted.end());
    filteredResult.push_back(filter(*endIterator));
  }

  return filteredResult;
}



std::pair<QVector<double>,QVector<double>> findCurveParamsFast(std::vector<double>& yVals, std::vector<double>& xVals, int border)
{
  std::pair<QVector<double>,QVector<double>> result;
  size_t currentElement = 0;
  if(2*border>yVals.size() || yVals.size()!=xVals.size())
    return result;

  auto fill = 
      [&](const std::pair<double,double>& coeffs,const std::vector<qreal>& x)
      {
        result.first.push_back(coeffs.first*90);
        double sum = 0.0;
        for(int i = 0; i< x.size();i++){
		    auto deviation = (coeffs.first*x[i]+coeffs.second - yVals[currentElement]);
            sum+= deviation*deviation/x.size();
        }
        result.second.push_back(std::sqrt(sum));
      };
{
  double Sxy = 0.0;
  double Sx = 0.0;
  double Sy = 0.0;
  double Sxx = 0.0;
  double n = border;

  auto ls = [&](){double A = (Sxy)/(Sxx);
				  return std::make_pair(A,(Sy-A*Sx)/n);};  //коэффициенты регрессии

  for(auto i = 0;i<border-1;i++){
	Sxy += xVals[i]*yVals[i];
	Sx += xVals[i];
	Sy += yVals[i];
	Sxx += xVals[i]*xVals[i];
  }

  for(;currentElement < border; currentElement++){
	Sxy += xVals[currentElement+border]*yVals[currentElement+border];
	Sx += xVals[currentElement+border];
	Sy += yVals[currentElement+border];
	Sxx += xVals[currentElement+border]*xVals[currentElement+border];
	n++;
	fill(ls(),std::vector<double>(xVals.begin(),xVals.begin()+(currentElement+border)));
  }
  for(;currentElement<xVals.size()-border;currentElement++){
	Sxy -= xVals[currentElement-border]*yVals[currentElement-border];
	Sx -= xVals[currentElement-border];
	Sy -= yVals[currentElement-border];
	Sxx -= xVals[currentElement-border]*xVals[currentElement-border];

	Sxy += xVals[currentElement+border]*yVals[currentElement+border];
	Sx += xVals[currentElement+border];
	Sy += yVals[currentElement+border];
	Sxx += xVals[currentElement+border]*xVals[currentElement+border];
	fill(ls(),std::vector<double>(xVals.begin()+(currentElement-border),xVals.begin()+(currentElement+border)));
  }
  for(;currentElement<xVals.size();currentElement++){
	  
	Sxy -= xVals[currentElement-border]*yVals[currentElement-border];
	Sx -= xVals[currentElement-border];
	Sy -= yVals[currentElement-border];
	Sxx -= xVals[currentElement-border]*xVals[currentElement-border];
	n--;
	fill(ls(),std::vector<double>(xVals.begin()+(currentElement-border),xVals.end()));
  }

}
  uts::dsp::RankFilter<double> filter(30,15,10);

  auto beginIterator = result.second.begin();
  if(result.first.size()<=31)
    return result;
  auto endIterator = beginIterator + 30;
  QVector<double> filteredResult;
  
  auto sorted = std::vector<double>(beginIterator,endIterator);
  std::sort(sorted.rbegin(),sorted.rend());
  filter.setInitialState(sorted.begin(),sorted.end());

  for(auto i = 0;i<30;i++)
    filteredResult.push_back(filter(*(beginIterator+i)));

  for(;endIterator!=result.second.end();beginIterator++,endIterator++){
    sorted = std::vector<double>(beginIterator,endIterator);
    std::sort(sorted.rbegin(),sorted.rend()); 
    filter.setInitialState(sorted.begin(),sorted.end());
    filteredResult.push_back(filter(*endIterator));
  }

  return std::make_pair(result.first,result.second);
}


//функция возвращает пару векторов - значения угла k касательной к каждой точке и значения шума в каждой точке
QVector<double> findCurveParams(const std::vector<double>& yVals,const std::vector<double>& xVals, int border)
{
  std::pair<QVector<double>,QVector<double>> result;
  size_t currentElement = 0;
  QVector<double> filteredResult;


  if(2*border>yVals.size() || yVals.size()!=xVals.size())
    return QVector<double>(0);
  result.first.resize(xVals.size());
  result.second.resize(xVals.size());

  auto fillParamsVector = [&](const std::vector<qreal>& coeffs,std::vector<qreal>::const_iterator xBegin,std::vector<qreal>::const_iterator xEnd){
	  result.first[currentElement] = (coeffs[1]*90);
      double sum = 0.0;
	  auto size = std::distance(xBegin,xEnd);
	  for(int i = 0; i<size ;i++){
         auto deviation = (coeffs[1]*(*(xBegin+i))+coeffs[0] - yVals[currentElement]);
         sum+= deviation*deviation/size;
      }
      result.second[currentElement] = (std::sqrt(sum));};

  for(;currentElement<border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin(),xVals.begin()+(border+currentElement)),
												  std::vector<double>(yVals.begin(),yVals.begin()+(border+currentElement)),1);
    fillParamsVector(coeffs,xVals.begin(),xVals.begin()+(border+currentElement));
  }

  auto xIter = xVals.begin()+(currentElement-border);
  auto yIter = yVals.begin()+(currentElement-border);
  //concurrency::parallel_for(currentElement,static_cast<size_t>(yVals.size()-border-1),[&](size_t ii){
  for(;currentElement<yVals.size()-border;currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.begin()+(currentElement+border)),
		std::vector<double>(yVals.begin()+(currentElement-border),yVals.begin()+(currentElement+border)),1);
	fillParamsVector(coeffs,xIter,xIter+2*border);
	xIter++;
	yIter++;
  //});
  }

  for(;currentElement<yVals.size();currentElement++){
    auto coeffs = uts::regression::leastSquares<std::vector<double>>(std::vector<double>(xVals.begin()+(currentElement-border),xVals.end()),
												std::vector<double>(yVals.begin()+(currentElement-border),yVals.end()),1);
    fillParamsVector(coeffs,xVals.begin()+(currentElement-border),xVals.end());
  }

  // Фильтрация данных
  auto size = 100;
  uts::dsp::RankFilter<double> filter(size,1,100);

  auto beginIterator = result.second.begin();
  if(result.first.size()<=size+1)
    return QVector<double>(0);
  auto endIterator = beginIterator + size;
  
  auto sorted = std::vector<double>(beginIterator,endIterator);
  std::sort(sorted.rbegin(),sorted.rend());
  filter.setInitialState(sorted.begin(),sorted.end());

  for(auto i = 0;i<size;i++){
	auto value = filter(*(beginIterator+i));
    filteredResult.push_back(value>=0.8? value : 0.0);
  }

  for(;endIterator!=result.second.end();beginIterator++,endIterator++){
    sorted = std::vector<double>(beginIterator,endIterator);
    std::sort(sorted.rbegin(),sorted.rend()); 
    filter.setInitialState(sorted.begin(),sorted.end());
	auto value = filter(*endIterator);
    filteredResult.push_back(value);
  }

  return filteredResult;
}

  Q_SLOT void PhaseSpectorgram::recalculatePlot()
  {
    std::vector<qreal> values;
    QList<qreal> originalValues;
    QList<QPointF> curveData;
    QList<QPointF> originalData;
    QList<QPointF> deviationsData;
    QList<QPointF> koeffsData;
    CurveParams params;
    
    for(auto iter = current->rbegin(); iter!=current->rend();iter++){
      auto value = *iter;
      originalValues.push_back(std::atan2(value.imag(),value.real()));
    }

    auto k = 0;
    auto trend = 1;
	for(auto value : originalValues){
      if(!values.empty()){
#ifdef USE_TREND_VERSION
		k++;
        auto dx = value - originalValues[k-1];
        auto delta = dx + 2*M_PI;

		if(std::abs(dx)+(trend) < std::abs(delta)){
          delta = dx;
        }
        if(std::abs(delta) >= std::abs(dx - 2*M_PI) + (trend))
          delta = dx - 2*M_PI;
		value = (values.back()+delta);
#else 
        auto delta = value + 2*k*M_PI - values.back();
        if(delta<-M_PI){
          k++;
          value=value+2*k*M_PI;
        }else if(delta>=M_PI){
          value+=2*(k-1)*M_PI;
        }else
          value+=2*k*M_PI;
#endif
      }
      values.push_back(value);
    }

    for(;frequencyData.size()!=values.size();){
      if(frequencyData.size()>values.size())
        frequencyData.pop_back();
      else
        values.pop_back();
    }

    for(int i = 0;i!=values.size();i++){
       curveData.push_back(QPointF(frequencyData[i],values[i]));
       originalData.push_back(QPointF(frequencyData[i],originalValues[i]));
    }

    auto koeffsValues = findCurveParams(values/*.toStdVector()*/, frequencyData/*.toStdVector()*/, 6);

    for(auto i = 0;i<koeffsValues.size();i++){
      koeffsData.push_back(QPointF(frequencyData[i],koeffsValues[i]));
    }

    auto max = std::max_element(koeffsValues.begin(),koeffsValues.end());
    auto min = std::min_element(koeffsValues.begin(),koeffsValues.end());

    originalCurve->detach();
    originalCurve->setSamples(originalData);
    originalCurve->attach(plot);
    /*
    deviationsCurve->detach();
    deviationsCurve->setSamples(curveData);
    deviationsCurve->attach(plot);*/
    koeffsCurve->detach();
    koeffsCurve->setSamples(koeffsData);
    koeffsCurve->attach(plot);
    plot->setAxisScale(QwtPlot::yLeft,*min,7);
    plot->setAxisScale(QwtPlot::xBottom, frequencyData.front(),frequencyData.back());

    zoom->setZoomBase(QRectF(QPointF(frequencyData.front(),7),QPointF(frequencyData.back(),*min)));
    plot->updateAxes();

    plot->replot();
  }

  void PhaseSpectorgram::stepForward()
  {
    if(current<(data.end()-1)){
      current+=1;
      recalculatePlot();
      emit positionChanged(1);
    }
  }

  void PhaseSpectorgram::stepBackward()
  {
    if(current>data.begin()){
      current-=1;
      recalculatePlot();
      emit positionChanged(-1);
    }
  }