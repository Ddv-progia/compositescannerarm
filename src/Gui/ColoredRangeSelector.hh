#pragma once

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <algorithm>

#include <qwt_color_map.h>

#include "Core/ScanData.hh"




class RangeViewFrame: public QFrame
{
  Q_OBJECT

  bool eventFilter(QObject* watched, QEvent* event);
  FrequencyRange range;

public:
  RangeViewFrame(QRgb rangeColor, FrequencyRange newRange, QWidget* parent = 0);

  void select();
  void deleteSelection();
  QSize sizeHint () const;
  FrequencyRange getRange() const;

signals:
  void rangeSelected(RangeViewFrame*);
};


class ColoredRangeSelector: public QFrame
{
  Q_OBJECT

  std::vector<std::vector<RangeViewFrame*>> views;
  std::pair<int, Extremum> currentRange;
  QLabel* frequency;
  void setFrequencyText(FrequencyRange range);
  bool eventFilter(QObject* watched, QEvent* event);

public:
  ColoredRangeSelector(std::vector<float> maxs, std::vector<float> mins, std::vector<float> avers, std::vector<float> diffs, std::vector<float> diffOnTable, std::vector<FrequencyRange> ranges, QwtColorMap* newColorMap,
                       QWidget* parent = 0);
public slots:
  void selectRangeMax(RangeViewFrame* view);
  void selectRangeMin(RangeViewFrame* view);
  void selectRangeAver(RangeViewFrame* view);
  void selectRangeDiff(RangeViewFrame* view);
  void selectRangeDiffOnTable(RangeViewFrame* view);
  void selectRange(RangeViewFrame* view, Extremum ex);

  void selectRangeByIndex(int idx, Extremum ex);
  void leave();
signals:
  void selectRange(int idx, Extremum ex);
};

