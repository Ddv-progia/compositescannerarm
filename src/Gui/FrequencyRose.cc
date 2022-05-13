#include "Gui/FrequencyRose.hh"


class FrequencyRoseDataAnalyzer
{
protected:
  const RangedMultiArray& points;
public:
  FrequencyRoseDataAnalyzer(const RangedMultiArray& rangedArray):points(rangedArray)
  {
  }
  virtual std::vector<std::pair<FrequencyRange,double>> operator ()() = 0;
  size_t sizeOfData() const
  {
    return points.size();
  }
};

class MinFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
public:
  MinFrequencyRoseDataAnalyzer(const RangedMultiArray& data):FrequencyRoseDataAnalyzer(data)
  {}

  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    auto size = points.size();
    int i = 0;
    for(auto& range : points){
      double min = range.second[0][0];
      for(auto& row : range.second)
        for(auto& point : row)
          if(point<min) min = point;
      result.push_back(std::make_pair(range.first,min));
      i++;
    }
    return result;
  }
};


class MaxFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
public:
  MaxFrequencyRoseDataAnalyzer(const RangedMultiArray& data):FrequencyRoseDataAnalyzer(data){}
  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    
    for(auto& range : points){
      double max = range.second[0][0];
      for(auto& row : range.second)
        for(auto& point : row)
          if(point>max) max = point;
      result.push_back(std::make_pair(range.first,max));
    }
    return result;
  }
};


class NegativeAverageFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
  double threshold;
public:
  NegativeAverageFrequencyRoseDataAnalyzer(const RangedMultiArray& data, double threshold):threshold(threshold),FrequencyRoseDataAnalyzer(data)
  {}

  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    namespace ba=boost::accumulators;

    for(auto& range : FrequencyRoseDataAnalyzer::points){
      bool isEmpty = true;
      ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
      for(auto& row : range.second){
        for(auto& point : row){
          double value = point;
          if(value<=threshold){
            acc(value);
            isEmpty = false;
          }
        }
      }
      result.push_back(std::make_pair(range.first,isEmpty ? 0.0:ba::mean(acc)));
    }
    return result;
  }
};


class PositiveAverageFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
  double threshold;
public:
  PositiveAverageFrequencyRoseDataAnalyzer(const RangedMultiArray& data, double threshold):threshold(threshold),FrequencyRoseDataAnalyzer(data)
  {}

  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    namespace ba=boost::accumulators;

    for(auto& range : FrequencyRoseDataAnalyzer::points){
      bool isEmpty = true;
      ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
      for(auto& row : range.second){
        for(auto& point : row){
          double value = point;
          if(value>=threshold){
            acc(value);
            isEmpty = false;
          }
        }
      }
      result.push_back(std::make_pair(range.first,isEmpty ? 0.0:ba::mean(acc)));
    }
    return result;
  }
};


class AverageFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
  double positiveThreshold;
  double negativeThreshold;
public:
  AverageFrequencyRoseDataAnalyzer(const RangedMultiArray& data,double positiveThreshold,double negativeThreshold):
    positiveThreshold(positiveThreshold),negativeThreshold(negativeThreshold),FrequencyRoseDataAnalyzer(data){}
  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    namespace ba=boost::accumulators;

    for(auto& range : FrequencyRoseDataAnalyzer::points){
      bool isEmpty = true;
      ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
      for(auto& row : range.second){
        for(auto& point : row){
          double value = point;
          if(value>=positiveThreshold || value<=negativeThreshold){
            acc(value);
            isEmpty = false;
          }
        }
      }
      result.push_back(std::make_pair(range.first,isEmpty ? 0.0:ba::mean(acc)));
    }
    return result;
  }
};

class MaxAverageFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
public:
  MaxAverageFrequencyRoseDataAnalyzer(const RangedMultiArray& data):FrequencyRoseDataAnalyzer(data){}
  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    namespace ba=boost::accumulators;
    for(auto& range : points){
	  bool isEmpty = true;
      double max = range.second[0][0];
      for(auto& row : range.second)
        for(auto& point : row)
          if(point>max) max = point;
	  auto border = 2*max/3.0;
	  ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
	  for(auto& row : range.second){
        for(auto& point : row){
			if(point>=border){
				acc(point);
				isEmpty = false;
			}
		}
	  }
	  result.push_back(std::make_pair(range.first,isEmpty?0.0 : ba::mean(acc)));
    }
    return result;
  }
};

class MinAverageFrequencyRoseDataAnalyzer: public FrequencyRoseDataAnalyzer
{
public:
  MinAverageFrequencyRoseDataAnalyzer(const RangedMultiArray& data):FrequencyRoseDataAnalyzer(data){}
  std::vector<std::pair<FrequencyRange,double>> operator ()() override
  {
    std::vector<std::pair<FrequencyRange,double>> result;
    namespace ba=boost::accumulators;
	
    for(auto& range : points){
	  bool isEmpty = true;
      double min = range.second[0][0];
      for(auto& row : range.second)
        for(auto& point : row)
          if(point<min) min = point;
	  auto border = 2*min/3.0;
	  ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
	  for(auto& row : range.second){
        for(auto& point : row){
			if(point<=border){
				acc(point);
				isEmpty = false;
			}
		}
	  }
	  result.push_back(std::make_pair(range.first,isEmpty?0.0 : ba::mean(acc)));
    }
    return result;
  }
};


/////////////////////////////////////////

void FrequencyRosePoint::focusInEvent(QFocusEvent* event)
{
  state = PointState::Selected;
  repaint();
  emit pointSelected(range,value);
}

void FrequencyRosePoint::focusOutEvent(QFocusEvent* event)
{
  state =  PointState::Default;
  repaint();
  emit pointUnselected();
}

void FrequencyRosePoint::paintEvent(QPaintEvent* event)
{
  auto center = QPointF(0,0);
  if(QWidget* parent = dynamic_cast<QWidget*>(this->parent()))
    center = (parent->rect().center());
  move(center.x()+coordinates.x()-1,center.y()+coordinates.y()-1);

  auto painter = new QPainter(this);
  painter->setPen(QPen(Qt::transparent));

  if(state == Selected)
    painter->setBrush(QBrush(Qt::GlobalColor::blue));
  else
    painter->setBrush(QBrush(Qt::black));
  auto rectangle = rect();
  painter->drawEllipse(rect());
  QFrame::paintEvent(event);
}


FrequencyRosePoint::FrequencyRosePoint(QPointF newCoordinates, FrequencyRange newRange,double newValue,QWidget* parent):
  state(Default),range(newRange),value(newValue),QFrame(parent)
{
  setAutoFillBackground(true);
  setBackgroundRole(QPalette::ColorRole::Window);
  this->setBaseSize(QSize(5,5));
  auto palette = this->palette();
  palette.setBrush(QPalette::All,QPalette::Window,QBrush(Qt::transparent));
  this->setPalette(palette);
  coordinates = QPointF(newCoordinates.x(),newCoordinates.y());
  setMouseTracking(true);
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  resize(baseSize());
  hide();
}

void FrequencyRosePoint::shift(int xS,int yS)
{
  coordinates.setX(coordinates.x()+xS);
  coordinates.setY(coordinates.y()+yS);
}

QPointF FrequencyRosePoint::point() const
{
  return coordinates;
}

  ///////////////////////

void FrequencyRoseGrid::splitInterval(QwtInterval interval,int splitDepth)
{
	if(splitDepth==0){
		marks.push_back(interval.minValue());
		return;
	}
	auto middle = (interval.minValue()+interval.maxValue())/2;
	splitInterval(QwtInterval(interval.minValue(),middle),splitDepth-1);
	splitInterval(QwtInterval(middle,interval.maxValue()),splitDepth-1);
    return;
}

FrequencyRoseGrid::FrequencyRoseGrid(int radius,std::vector<FrequencyRange> ranges,QWidget* parent):QWidget(parent),r(radius)
{
	QwtInterval interval(ranges.front().from,ranges.back().to);
	splitInterval(interval,4);
	std::sort(marks.begin(),marks.end());
}

void FrequencyRoseGrid::paintGrid(QPainter* painter, bool isNeedToRoundTextMarks)
{
	QPointF center = rect().center();
	painter->setBackground(QBrush(Qt::transparent));

	auto angleStep = 2*M_PI/marks.size();
	auto oldPen = painter->pen();
	auto oldBrush = painter->brush();
	painter->setPen(Qt::PenStyle::DotLine);
	painter->setBrush(QBrush(Qt::black));
	for(auto i = 0;i<marks.size();i++){
		QPointF point = FrequencyRose::findCoordinates(i*angleStep,r);
		painter->drawLine(QPoint(0,0),point);
        //*******
		QString text = QString::number(isNeedToRoundTextMarks? std::floor(marks[i]): marks[i]);
		//QString text = QString::number(marks[i]);
        //*******

        QPointF textPoint(point.x()-this->fontMetrics().width(text)/2,point.y()+this->fontMetrics().height()/2);// текст отрисовывается строго по центру линии

 		painter->setBrush(QBrush(Qt::white));
		painter->setPen(QPen(Qt::white));
		painter->drawRect(QRectF(textPoint,QPointF(textPoint.x()+this->fontMetrics().width(text),textPoint.y()-this->fontMetrics().height())));
		painter->setBrush(QBrush(Qt::black));
		painter->setPen(QPen(Qt::black,1,Qt::PenStyle::DotLine));
		painter->drawText(textPoint,text);
	}
	painter->setPen(oldPen);
	painter->setBrush(oldBrush);
}

///////////////

QPointF FrequencyRose::findCoordinates(double angle,double value)
{
  auto Ox = sin(angle)*(value);
  auto Oy = cos(angle)*(value);

  if(angle>M_PI && angle<2*M_PI)
    if(Ox>0)
      Ox=-Ox;
  if(angle>M_PI_2 && angle<3*M_PI_2)
    if(Oy>0)
      Oy=-Oy;

  return QPointF(Ox,Oy);
}

void FrequencyRose::fillPointList(FrequencyRoseDataAnalyzer* analyser,std::map<double,FrequencyRosePoint*>& pointList)
{
  if(analyser->sizeOfData()==0) return;

  auto angleStep = 2*M_PI/analyser->sizeOfData();
  int i = 0;

  for(auto& value : (*analyser)()){  
    auto point = new FrequencyRosePoint(findCoordinates(i*angleStep,convert(value.second,nullCircleRadius)),value.first,value.second,this);
    connect(point,SIGNAL(pointSelected(FrequencyRange,double)),SIGNAL(rangeSelected(FrequencyRange,double)));
    connect(point,SIGNAL(pointUnselected()),SIGNAL(rangeUnselected()));
	auto r = connect(this,SIGNAL(pointsReady()),point,SLOT(show()));
    pointList[i*angleStep]=point;
    i++;
  }
}


double FrequencyRose::convert(double value,double null)
{
  if(value<=0){
    return(null/(1-(value*0.5)));
  }
  else
    return(factor*value+null);
}

void FrequencyRose::drawRose(QPainter* painter,std::map<double,FrequencyRosePoint*>& data)
{
  QPolygonF rose;
  for(auto& point:data){
    rose<<point.second->point();
  }
  painter->drawPolygon(rose);
}

FrequencyRose::FrequencyRose(std::vector<ColorStop>& colorList, double factor, QWidget* parent):QWidget(parent),nullCircleRadius(100),colors(colorList),factor(factor)
{
  setAutoFillBackground(true);
  setBackgroundRole(QPalette::ColorRole::Window);
  auto palette = this->palette();
  palette.setBrush(QPalette::All,QPalette::Window,QBrush(Qt::white));
  setPalette(palette);
}

Q_SLOT void FrequencyRose::selectRange(FrequencyRange range ,double value)
{
  emit rangeSelected(range,value);
}

void FrequencyRose::createGrid(std::map<double,FrequencyRosePoint*>& pointList)
{
	std::vector<FrequencyRange> ranges;
	for(auto& point : pointList)
		ranges.push_back(point.second->getRange());
	grid = new FrequencyRoseGrid(sizeLimit+20,ranges,this);
}



////////////////



void MinMaxFrequencyRose::fillPointList(FrequencyRoseDataAnalyzer* analyser,std::map<double,FrequencyRosePoint*>& pointList)
{
  if(analyser->sizeOfData()==0) return;

  auto angleStep = 2*M_PI/analyser->sizeOfData();
  int i = 0;

  for(auto& value : (*analyser)()){  
    auto point = new FrequencyRosePoint(findCoordinates(i*angleStep,convert(value.second,nullCircleRadius)),value.first,value.second,this);
    connect(point,SIGNAL(pointSelected(FrequencyRange,double)),SIGNAL(rangeSelected(FrequencyRange,double)));
    connect(point,SIGNAL(pointUnselected()),SIGNAL(rangeUnselected()));
    pointList[i*angleStep]=point;
	connect(this,SIGNAL(pointsReady()),point,SLOT(show()));
    i++;
  }
}

MinMaxFrequencyRose::MinMaxFrequencyRose(RangedMultiArray data, double factor, std::vector<ColorStop>& colorList,QWidget* parent):FrequencyRose(colorList,factor,parent)
{
  fillPointList(&MinFrequencyRoseDataAnalyzer(data),minPoints);
  fillPointList(&MaxFrequencyRoseDataAnalyzer(data),maxPoints);

  sizeLimit = std::max<qreal>(std::abs(maxPoints.begin()->second->point().x()),std::abs(maxPoints.begin()->second->point().y()));
  std::for_each(maxPoints.begin(),maxPoints.end(),
    [&](std::pair<double,FrequencyRosePoint*> point){
      auto currentLimit = std::max<qreal>(std::abs(point.second->point().x()),std::abs(point.second->point().y()));
      sizeLimit = std::max<qreal>(sizeLimit,currentLimit);});


  this->setMinimumSize(2*sizeLimit+75,2*sizeLimit+75);
  this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  emit pointsReady();
  createGrid(minPoints);
}

  
void MinMaxFrequencyRose::paintEvent(QPaintEvent* event)
{
  auto painter = new QPainter(this);
  QPointF center = rect().center();
  double max = colors.begin()->val;
  double min = colors.begin()->val;

  std::for_each(colors.begin(),colors.end(),
    [&](ColorStop stop){if(stop.val>max) max = stop.val; if(stop.val<min) min = stop.val;});

  painter->setBackground(QBrush(Qt::white));
  painter->translate(center.x(),center.y());
  double distance = rect().width();
  QRadialGradient gradient(0,0,distance);

  for(auto& stop : colors){
    auto valStop = convert(stop.val,nullCircleRadius)/distance;
    gradient.setColorAt(valStop,QColor(QString::fromStdString(stop.color)));
  }

  painter->setBrush(QBrush(gradient));
  painter->setPen(QPen(Qt::transparent));
  drawRose(painter,maxPoints);
  painter->setBrush(QBrush(Qt::white));
  drawRose(painter,minPoints);

  painter->setPen(Qt::darkGreen);
  painter->setBrush(QBrush(Qt::transparent));
  painter->drawEllipse(QPointF(0,0),nullCircleRadius,nullCircleRadius);
  currentPoint = maxPoints.begin()->second;
  grid->paintGrid(painter,true);
}

///

AverageFrequencyRose::AverageFrequencyRose(RangedMultiArray data,double positiveThreshold,double negativeThreshold, double factor, std::vector<ColorStop>& colorList,QWidget* parent):FrequencyRose(colorList,factor,parent)
{
  fillPointList(&AverageFrequencyRoseDataAnalyzer(data,positiveThreshold,negativeThreshold),points);

  sizeLimit = std::max<qreal>(std::abs(points.begin()->second->point().x()),std::abs(points.begin()->second->point().y()));
  std::for_each(points.begin(),points.end(),
    [&](std::pair<double,FrequencyRosePoint*> point){
      auto currentLimit = std::max<qreal>(std::abs(point.second->point().x()),std::abs(point.second->point().y()));
      sizeLimit = std::max<qreal>(sizeLimit,currentLimit);});

  setAutoFillBackground(true);
  setBackgroundRole(QPalette::ColorRole::Window);
    
  auto palette = this->palette();
  palette.setBrush(QPalette::Active,QPalette::Window,QBrush(Qt::white));
  this->setMinimumSize(2*sizeLimit+75,2*sizeLimit+75);
  this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  createGrid(points);
  setPalette(palette);
  emit pointsReady();
}

  
void AverageFrequencyRose::paintEvent(QPaintEvent* event)
{
  QPainter painter;
  QPointF center = rect().center();
  double max = colors.begin()->val;
  double min = colors.begin()->val;
  auto rectangle = this->rect();

  std::for_each(colors.begin(),colors.end(),
    [&](ColorStop stop){if(stop.val>max) max = stop.val; if(stop.val<min) min = stop.val;});

  double distance = rect().width();
  QRadialGradient gradient(0,0,distance);

  for(auto& stop : colors){
    auto valueStop = convert(stop.val,nullCircleRadius)/distance;
    gradient.setColorAt(valueStop, QColor(QString::fromStdString(stop.color)));
  }

  QImage source(rectangle.size(),QImage::Format_ARGB32_Premultiplied);
  source.fill(Qt::transparent);
   
  painter.begin(&source);
  painter.translate(center.x(),center.y());
  painter.setRenderHint(QPainter::Antialiasing,true);
  painter.setBrush(QBrush(gradient));
  painter.setPen(QPen(Qt::transparent));
  painter.drawEllipse(QPointF(0,0),nullCircleRadius,nullCircleRadius);
  painter.end();

  QImage result(rectangle.size(),QImage::Format_ARGB32_Premultiplied);
  result.fill(Qt::transparent);
  painter.begin(&result);
  painter.translate(center.x(),center.y());
  painter.setRenderHint(QPainter::Antialiasing,true);
  painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_SourceOver);
  painter.setPen(QPen(Qt::transparent));
  painter.setBrush(QBrush(gradient));
  drawRose(&painter,points);
  auto shiftRect = rectangle;
  shiftRect.setTopLeft(QPoint(0-center.x(),0-center.y()));
  shiftRect.setBottomRight(QPoint(center.x(),center.y()));
  painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_Xor);
  painter.drawImage(shiftRect,source);
  painter.end();

  painter.begin(this);
  painter.drawPixmap(rectangle,QPixmap::fromImage(result));
  painter.end();

  currentPoint = points.begin()->second;
  grid->paintGrid(&painter, true);
}

SplitedAverageFrequencyRose::SplitedAverageFrequencyRose(RangedMultiArray data,double positiveThreshold,double negativeThreshold, double factor,std::vector<ColorStop>& colorList,QWidget* parent):FrequencyRose(colorList,factor,parent)
{
  fillPointList(&PositiveAverageFrequencyRoseDataAnalyzer(data,positiveThreshold),positivePoints);
  fillPointList(&NegativeAverageFrequencyRoseDataAnalyzer(data,negativeThreshold),negativePoints);

  sizeLimit = std::max<qreal>(std::abs(positivePoints.begin()->second->point().x()),std::abs(positivePoints.begin()->second->point().y()));
  std::for_each(positivePoints.begin(),positivePoints.end(),
    [&](std::pair<double,FrequencyRosePoint*> point){
      auto currentLimit = std::max<qreal>(std::abs(point.second->point().x()),std::abs(point.second->point().y()));
      sizeLimit = std::max<qreal>(sizeLimit,currentLimit);});

  setAutoFillBackground(true);
  setBackgroundRole(QPalette::ColorRole::Window);
    
  auto palette = this->palette();
  palette.setBrush(QPalette::Active,QPalette::Window,QBrush(Qt::white));
  this->setMinimumSize(2*sizeLimit+75,2*sizeLimit+75);
  this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  createGrid(positivePoints);
  setPalette(palette);
  emit pointsReady();
}

void SplitedAverageFrequencyRose::paintEvent(QPaintEvent* event)
{
  auto painter = new QPainter(this);
  QPointF center = rect().center();
  double max = colors.begin()->val;
  double min = colors.begin()->val;

  std::for_each(colors.begin(),colors.end(),
    [&](ColorStop stop){if(stop.val>max) max = stop.val; if(stop.val<min) min = stop.val;});

  painter->setBackground(QBrush(Qt::white));
  painter->translate(center.x(),center.y());
  double distance = rect().width();
  QRadialGradient gradient(0,0,distance);

  for(auto& stop : colors){
    auto valueStop = convert(stop.val,nullCircleRadius)/distance;
    gradient.setColorAt(valueStop, QColor(QString::fromStdString(stop.color)));
  }

  painter->setBrush(QBrush(gradient));
  painter->setPen(QPen(Qt::transparent));
  drawRose(painter,positivePoints);
  painter->setBrush(QBrush(Qt::white));
  drawRose(painter,negativePoints);

  painter->setPen(Qt::darkGreen);
  painter->setBrush(QBrush(Qt::transparent));
  painter->drawEllipse(QPointF(0,0),nullCircleRadius,nullCircleRadius);
  currentPoint = positivePoints.begin()->second;

  grid->paintGrid(painter, true);
}

////////////////////////

Q_SLOT void FrequencyRoseWidget::selectRoseType()
{
  auto idx = typeCombo->currentIndex();
  this->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding));
  rose->hide();
  layout->removeWidget(rose);
  rose->deleteLater();
  switch (idx)
  {
  case 0:
    rose = new MinMaxFrequencyRose(data,factorSpinBox->value(),colorList);
    positiveThreshold->setEnabled(false);
    negativeThreshold->setEnabled(false);
    break;
  default:
    rose = new SplitedAverageFrequencyRose(data,positiveThreshold->value(),negativeThreshold->value(),factorSpinBox->value(),colorList); 
    positiveThreshold->setEnabled(true);
    negativeThreshold->setEnabled(true);
    break;
  }
    
  connect(rose,SIGNAL(rangeSelected(FrequencyRange,double)),SLOT(setLabelText(FrequencyRange,double)));
  connect(rose,SIGNAL(rangeUnselected()),SLOT(clearText()));

  layout->addWidget(rose);
    
  this->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding));
  this->setLayout(layout);
}

FrequencyRoseWidget::FrequencyRoseWidget(RangedMultiArray& newData,std::vector<ColorStop>& newColorList,QWidget* parent):data(newData),colorList(newColorList),QDialog(parent)
{
  setWindowTitle(parent->windowTitle());
  layout = new QVBoxLayout;
  frequency = new QLabel;
  level = new QLabel;
  positiveThreshold = new QDoubleSpinBox;
  negativeThreshold = new QDoubleSpinBox;
  positiveThreshold->setValue(10.0);
  positiveThreshold->setSingleStep(0.1);
  negativeThreshold->setSingleStep(0.05);
  negativeThreshold->setMinimum(-10.0);
  negativeThreshold->setMaximum(0.0);
  negativeThreshold->setValue(-1.0);

  factorSpinBox = new QDoubleSpinBox;
  factorSpinBox->setSingleStep(1.0);

  //*******
  factorSpinBox->setValue(10.0);
  //factorSpinBox->setValue(1.0);
  //*******

  typeCombo = new QComboBox;
  auto thresholdLayout = new QHBoxLayout;
  thresholdLayout->addWidget(new QLabel(QString("До ")));
  thresholdLayout->addWidget(negativeThreshold);
  thresholdLayout->addWidget(new QLabel(QString(" от ")));
  thresholdLayout->addWidget(positiveThreshold);

  auto infoLayout = new QGridLayout;
  infoLayout->setContentsMargins(0,0,0,0);
  infoLayout->setSpacing(2);

  rose = new MinMaxFrequencyRose(data,factorSpinBox->value(),colorList,this);
    
  infoLayout->addWidget(new QLabel(QString("Диапазон")),0,0);
  infoLayout->addWidget(new QLabel(QString("Значение")),1,0);
  infoLayout->addWidget(new QLabel(QString("Тип диаграммы")),2,0);
  infoLayout->addWidget(new QLabel(QString("Пороги")),3,0);
  infoLayout->addWidget(new QLabel(QString("Множитель")),4,0);
  infoLayout->addWidget(frequency,0,1);
  infoLayout->addWidget(level,1,1);
  infoLayout->addWidget(typeCombo,2,1);
  infoLayout->addLayout(thresholdLayout,3,1);
  infoLayout->addWidget(factorSpinBox,4,1);
  layout->addLayout(infoLayout);
  layout->addWidget(rose);
  layout->setSizeConstraint(QLayout::SizeConstraint::SetNoConstraint);

  connect(typeCombo,SIGNAL(currentIndexChanged(int)),SLOT(selectRoseType()));
  connect(positiveThreshold,SIGNAL(valueChanged(double)),SLOT(selectRoseType()));
  connect(negativeThreshold,SIGNAL(valueChanged(double)),SLOT(selectRoseType()));
  connect(factorSpinBox,SIGNAL(valueChanged(double)),SLOT(selectRoseType()));

  typeCombo->addItems(QStringList() << QString("Экстремумы") << QString("Среднее"));

  layout->setStretch(1,3);
  this->setLayout(layout);
}

Q_SLOT void FrequencyRoseWidget::setLabelText(FrequencyRange range,double value)
{
  frequency->setText(QString("%1 - %2").arg(range.from).arg(range.to));
  level->setText(QString::number(value));
}
Q_SLOT void FrequencyRoseWidget::clearText()
{
  frequency->clear();
  level->clear();
}
  