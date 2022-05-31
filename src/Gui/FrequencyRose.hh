#pragma once

#include <QWidget>
#include <QEvent>
#include <QPainter>
#include <QDialog>
#include <QLayout>
#include <QLabel>
#include <QString>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <qmath.h>
#include <boost/multi_array.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <map>
#include "Core\ScanData.hh"
#include <QMessageBox>


#include <qwt_interval.h>



typedef boost::multi_array<double,2> DDArray;
typedef std::vector<std::pair<FrequencyRange,DDArray>> RangedMultiArray;


class FrequencyRosePoint: public QFrame //описание точки,хранит данные о диапазоне,соответствующем точке, уровне сигнала, координатах на плоскости
{
  enum PointState{Default,Selected};

  Q_OBJECT
  QPointF coordinates;
  FrequencyRange range;
  double value;
  PointState state;

  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
  inline void paintEvent(QPaintEvent* event) override;

public:
  FrequencyRosePoint(QPointF newCoordinates, FrequencyRange newRange,double newValue,QWidget* parent = 0);
  void shift(int xS,int yS);
  QPointF point() const;
  FrequencyRange getRange() const
  {
	  return range;
  }

  Q_SIGNAL void pointSelected(FrequencyRange,double);
  Q_SIGNAL void pointUnselected();
};

class FrequencyRoseDataAnalyzer;


class FrequencyRoseGrid: public QWidget
{
	Q_OBJECT
	std::vector<double> marks;
	int r;

	void splitInterval(QwtInterval interval,int splitDepth);
public:
	FrequencyRoseGrid(int radius,std::vector<FrequencyRange> ranges,QWidget* parent = 0);
	void paintGrid(QPainter* painter,bool roundTextMarks = false);
};



class FrequencyRose: public QWidget	   //базовый класс розы
{
  Q_OBJECT
  
protected:
  FrequencyRosePoint* currentPoint;
  FrequencyRoseGrid* grid;
  int nullCircleRadius;
  int sizeLimit;
  double factor;
  std::vector<ColorStop> colors;

  void fillPointList(FrequencyRoseDataAnalyzer* analyser,std::map<double,FrequencyRosePoint*>& pointList);
  double convert(double value,double null);
  void drawRose(QPainter* painter,std::map<double,FrequencyRosePoint*>& data);
  void createGrid(std::map<double,FrequencyRosePoint*>& pointList);

public:

  FrequencyRose(std::vector<ColorStop>& colorList, double factor, QWidget* parent = 0);
  ~FrequencyRose(){}
  
  static QPointF findCoordinates(double angle,double value);

  Q_SLOT void selectRange(FrequencyRange range ,double value);
  Q_SIGNAL void rangeSelected(FrequencyRange,double);
  Q_SIGNAL void rangeUnselected();
  Q_SIGNAL void pointsReady();
};


class MinMaxFrequencyRose: public FrequencyRose //роза минимумов и максимумов
{
  Q_OBJECT
  std::map<double,FrequencyRosePoint*> maxPoints;
  std::map<double,FrequencyRosePoint*> minPoints;

  void fillPointList(FrequencyRoseDataAnalyzer* analyser,std::map<double,FrequencyRosePoint*>& pointList);
public:
  MinMaxFrequencyRose(RangedMultiArray data, double factor, std::vector<ColorStop>& colorList,QWidget* parent = 0);
  void paintEvent(QPaintEvent* event);
};

///

class AverageFrequencyRose: public FrequencyRose //роза средних значений по минимумам и максимумам
{
  Q_OBJECT
  std::map<double,FrequencyRosePoint*> points;

public:
  AverageFrequencyRose(RangedMultiArray data,double positiveThreshold,double negativeThreshhold, double factor, std::vector<ColorStop>& colorList,QWidget* parent = 0);
  void paintEvent(QPaintEvent* event);
};


class SplitedAverageFrequencyRose: public FrequencyRose 
{
  Q_OBJECT
  std::map<double,FrequencyRosePoint*> positivePoints;
  std::map<double,FrequencyRosePoint*> negativePoints;

public:
  SplitedAverageFrequencyRose(RangedMultiArray data,double positiveThreshold,double negativeThreshhold, double factor, std::vector<ColorStop>& colorList,QWidget* parent = 0);
  void paintEvent(QPaintEvent* event);
};


class FrequencyRoseWidget:public QDialog //отображение окна с розой на экране, дополнительная информация
{
  Q_OBJECT

  QLabel* frequency;
  QLabel* level;
  FrequencyRose* rose;
  QComboBox* typeCombo;
  QDoubleSpinBox* positiveThreshold;
  QDoubleSpinBox* negativeThreshold;
  QVBoxLayout* layout;
  RangedMultiArray data;
  std::vector<ColorStop> colorList;

  Q_SLOT void selectRoseType();
public:
	QDoubleSpinBox* factorSpinBox;
	FrequencyRoseWidget(RangedMultiArray& newData,std::vector<ColorStop>& newColorList, double& factor, QWidget* parent = 0);

  Q_SLOT void setLabelText(FrequencyRange range,double value);
  Q_SLOT void clearText();
};