#pragma once

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <algorithm>

#include <qwt_color_map.h>

#include "Core/ScanData.hh"




class RangeView: public QFrame
{
  Q_OBJECT

  bool eventFilter(QObject* watched, QEvent* event);
  FrequencyRange range;

public:
  RangeView(QRgb rangeColor, FrequencyRange newRange, QWidget* parent = 0);

  void select();
  void deleteSelection();
  QSize sizeHint () const;
  FrequencyRange getRange() const;

signals:
  void rangeSelected(RangeView*);
};


class ColoredRangeSelector: public QFrame
{
  Q_OBJECT

  std::vector<std::vector<RangeView*>> views;
  std::pair<int, Extremum> currentRange;
  QLabel* frequency;
  void setFrequencyText(FrequencyRange range);
  bool eventFilter(QObject* watched, QEvent* event);

public:
  ColoredRangeSelector(std::vector<float> maxs, std::vector<float> mins, std::vector<float> avers, std::vector<float> diffs, std::vector<FrequencyRange> ranges, QwtColorMap* newColorMap,
                       QWidget* parent = 0);
public slots:
  void selectRangeMax(RangeView* view);
  void selectRangeMin(RangeView* view);
  void selectRangeAver(RangeView* view);
  void selectRangeDiff(RangeView* view);
  void selectRange(RangeView* view, Extremum ex);

  void selectRangeByIndex(int idx, Extremum ex);
  void leave();
signals:
  void selectRange(int idx, Extremum ex);
};

