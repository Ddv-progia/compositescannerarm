#include "Gui/ColoredRangeSelector.hh"

#include <QLayout>
#include <QLabel>
#include <QKeyEvent>
#include <qwt_interval.h>


bool RangeViewFrame::eventFilter(QObject* watched, QEvent* event)
{
  if(event->type() == QEvent::MouseButtonRelease) {
    select();
    emit rangeSelected(this);
    return true;
  } else
    return QFrame::eventFilter(watched, event);
}


RangeViewFrame::RangeViewFrame(QRgb rangeColor, FrequencyRange newRange, QWidget* parent): QFrame(parent), range(newRange)
{
  this->setFrameStyle(QFrame::Box | QFrame::Raised);
  this->setLineWidth(1);
  this->setMidLineWidth(0);
  this->setFrameShape(QFrame::NoFrame); // выключаем рамку
  setBackgroundRole(QPalette::Window);
  setAutoFillBackground(true);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, QColor::fromRgb(rangeColor));
  this->setPalette(palette);
  this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

  installEventFilter(this);
}

void RangeViewFrame::select()
{
  //this->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  this->setFrameStyle(QFrame::Box | QFrame::Raised);
  this->setLineWidth(1);
}
void RangeViewFrame::deleteSelection()
{
  this->setFrameShape(QFrame::NoFrame);
}

QSize RangeViewFrame::sizeHint () const
{
  return QSize(12, 12);
}

FrequencyRange RangeViewFrame::getRange() const
{
  return range;
}



void ColoredRangeSelector::setFrequencyText(FrequencyRange range)
{
  frequency->clear();
  frequency->setText(QString("%1 - %2").arg(range.from).arg(range.to));
}

bool ColoredRangeSelector::eventFilter(QObject* watched, QEvent* event)
{
  if (event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent*>(event);
    auto& view = views[static_cast<uint>(currentRange.second)][currentRange.first];
    int extremumSize = static_cast<int>(Extremum::Count);
    int currentRangeSecond = static_cast<int>(currentRange.second);
    bool viewChanged = false;
    if (keyEvent->modifiers() == Qt::NoModifier) {
      switch (keyEvent->key()) {
      case Qt::Key_Down:
        if (currentRangeSecond < (extremumSize - 1)) {
            currentRange.second = static_cast<Extremum>(currentRangeSecond+1);
            viewChanged = true;
        }
        break;
      case Qt::Key_Up:
        if (currentRangeSecond > 0) {
            currentRange.second = static_cast<Extremum>(currentRangeSecond - 1);
            viewChanged = true;
        }
        break;
      case Qt::Key_Left:
        if(currentRange.first > 0) {
          currentRange.first--;
          viewChanged = true;
        }
        break;
      case Qt::Key_Right:
        if(currentRange.first < views[0].size() - 1) {
          currentRange.first++;
          viewChanged = true;
        }
        break;
      }
      if (viewChanged) {
          view->deleteSelection();
          auto& viewNew = views[static_cast<uint>(currentRange.second)][currentRange.first];
          viewNew->select();
          setFrequencyText(viewNew->getRange());
          emit selectRange(currentRange.first, currentRange.second);
      }
      else
          return true;
    }
  }
  return QObject::eventFilter(watched, event);
}


ColoredRangeSelector::ColoredRangeSelector(std::vector<float> maxs, std::vector<float> mins, std::vector<float> avers, std::vector<float> diffs, std::vector<float> diffOnTable, std::vector<FrequencyRange> ranges,
    QwtColorMap* newColorMap, QWidget* parent) : QFrame(parent)
{
  QwtInterval interval(static_cast<double>(*std::min_element(mins.begin(), mins.end())), static_cast<double>(*std::max_element(maxs.begin(),
                       maxs.end())));
  auto minLayout = new QHBoxLayout;
  auto maxLayout = new QHBoxLayout;
  auto averLayout = new QHBoxLayout;
  auto diffLayout = new QHBoxLayout;
  auto diffOnTableLayout = new QHBoxLayout; // разница по двум диапазонам из таблицы диапазонов пользователя
  auto selectorLayout = new QVBoxLayout;
  auto layout = new QHBoxLayout;
  auto headerLayout = new QVBoxLayout;
  auto freqLayout = new QHBoxLayout;
  auto commonLayout = new QVBoxLayout;


  minLayout->setSpacing(0);
  minLayout->setContentsMargins(2, 0, 0, 0);
  maxLayout->setSpacing(0);
  maxLayout->setContentsMargins(2, 0, 0, 0);
  averLayout->setSpacing(0);
  averLayout->setContentsMargins(2, 0, 0, 0);
  diffLayout->setSpacing(0);
  diffLayout->setContentsMargins(2, 0, 0, 0);
  diffOnTableLayout->setSpacing(0);
  diffOnTableLayout->setContentsMargins(2, 0, 0, 0);
  selectorLayout->setContentsMargins(0, 0, 0, 0);
  selectorLayout->setSpacing(3);
  headerLayout->setContentsMargins(0, 0, 0, 2);
  headerLayout->setSpacing(3);
  freqLayout->setContentsMargins(0, 0, 0, 0);
  freqLayout->setSpacing(3);
  commonLayout->setContentsMargins(0, 0, 0, 0);
  commonLayout->setSpacing(3);

  headerLayout->addWidget(new QLabel("Max. "));
  headerLayout->addWidget(new QLabel("Min. "));
  headerLayout->addWidget(new QLabel("Aver. "));
  headerLayout->addWidget(new QLabel("Diff. "));
  headerLayout->addWidget(new QLabel("Diff.OnTable "));

  minLayout->setStretch(0, 0);
  maxLayout->setStretch(0, 0);
  averLayout->setStretch(0, 0);
  diffLayout->setStretch(0, 0);
  diffOnTableLayout->setStretch(0, 0);

  views.resize(5);
  //views.resize(4);

  this->setFrameStyle(QFrame::StyledPanel);

  for(int i = 0; i < ranges.size(); i++) {
      //float val = mins[i];
      //if (isnan(mins[i])) {
      //    val = 0;
      //}
    auto minView = new RangeViewFrame(newColorMap->rgb(interval, mins[i]), ranges[i]);
    //val = maxs[i];
    //if (isnan(maxs[i])) {
    //    val = 0;
    //}
    auto maxView = new RangeViewFrame(newColorMap->rgb(interval, maxs[i]), ranges[i]);
    //val = avers[i];
    //if (isnan(avers[i])) {
    //    val = 0;
    //}
    auto averView = new RangeViewFrame(newColorMap->rgb(interval, avers[i]), ranges[i]);
    //val = diffs[i];
    //if (isnan(diffs[i])) {
    //    val = 0;
    //}
    auto diffView = new RangeViewFrame(newColorMap->rgb(interval, diffs[i]), ranges[i]);
    
    auto diffOnTableView = new RangeViewFrame(newColorMap->rgb(interval, diffOnTable[i]), ranges[i]);

    views[0].push_back(maxView);
    views[1].push_back(minView);
    views[2].push_back(averView);
    views[3].push_back(diffView);
    views[4].push_back(diffOnTableView);

    maxLayout->addWidget(maxView);
    minLayout->addWidget(minView);
    averLayout->addWidget(averView);
    diffLayout->addWidget(diffView);
    diffOnTableLayout->addWidget(diffOnTableView);

    connect(maxView, SIGNAL(rangeSelected(RangeViewFrame*)), SLOT(selectRangeMax(RangeViewFrame*)));
    connect(minView, SIGNAL(rangeSelected(RangeViewFrame*)), SLOT(selectRangeMin(RangeViewFrame*)));
    connect(averView, SIGNAL(rangeSelected(RangeViewFrame*)), SLOT(selectRangeAver(RangeViewFrame*)));
    connect(diffView, SIGNAL(rangeSelected(RangeViewFrame*)), SLOT(selectRangeDiff(RangeViewFrame*)));
    //connect(diffOnTableView, SIGNAL(rangeSelected(RangeView*)), SLOT(selectRangeDiffOnTable(RangeView*)));
    //connect(diffOnTableView, &RangeView::rangeSelected,[=](RangeView* rangeView) {selectRangeDiffOnTable(rangeView);});
    connect(diffOnTableView, &RangeViewFrame::rangeSelected, this,&ColoredRangeSelector::selectRangeDiffOnTable);
    //connect(diffOnTableView, &RangeView::rangeSelected,[=](RangeView* rangeView) {selectRange(rangeView, Extremum::DiffOnTable);});
  }

  frequency = new QLabel;

  freqLayout->addWidget(new QLabel(QString("Диапазон")));
  freqLayout->addWidget(frequency);

  selectorLayout->addLayout(maxLayout);
  selectorLayout->addLayout(minLayout);
  selectorLayout->addLayout(averLayout);
  selectorLayout->addLayout(diffLayout);
  selectorLayout->addLayout(diffOnTableLayout);

  layout->addLayout(headerLayout);
  layout->addLayout(selectorLayout);

  commonLayout->addLayout(freqLayout);
  commonLayout->addLayout(layout);

  installEventFilter(this);
  setLayout(commonLayout);

  currentRange = std::make_pair(0, Extremum::Max);

  this->setFocusPolicy(Qt::StrongFocus);
  leave();
}

void ColoredRangeSelector::selectRangeMax(RangeViewFrame* view)
{
    selectRange(view, Extremum::Max); //*******
}

//*******
void ColoredRangeSelector::selectRangeAver(RangeViewFrame* view)
{
    selectRange(view, Extremum::Aver);
}

void ColoredRangeSelector::selectRangeDiffOnTable(RangeViewFrame* view)
{
    selectRange(view, Extremum::DiffOnTable); 
}

void ColoredRangeSelector::selectRangeDiff(RangeViewFrame* view)
{
    selectRange(view, Extremum::Diff); 
}

void ColoredRangeSelector::selectRange(RangeViewFrame* viewIn, Extremum ex)
{
  auto currView = views[static_cast<uint>(currentRange.second)][currentRange.first];
  currView->deleteSelection();
  for (auto view: views)
  {
      for (int i = 0; i < view.size(); i++)
      {
          view[i]->deleteSelection();
          if (viewIn == view[i]) {
              currentRange = std::make_pair(i, ex);
              emit selectRange(i, ex);
              viewIn->select();
              setFrequencyText(viewIn->getRange());
          }
      }
  }
}
//*******

void ColoredRangeSelector::selectRangeMin(RangeViewFrame* view)
{
    selectRange(view, Extremum::Min); //*******
}

void ColoredRangeSelector::selectRangeByIndex(int idx, Extremum ex)
{
  auto currView = views[static_cast<uint>(currentRange.second)][currentRange.first];
  (currView)->deleteSelection();
  if(idx >= 0 && idx < views[0].size()) {
    (currView)->deleteSelection();
    if(ex == Extremum::Max) {
      (currView) = views[0][idx];
      (currView)->select();
    } else {
      (currView) = views[1][idx];
      (currView)->select();
    }
    currentRange = std::make_pair(idx, ex);
    setFrequencyText(currView->getRange());
  }
  else {
    views[1][idx]->deleteSelection();
    views[0][idx]->deleteSelection();
  }
}

void ColoredRangeSelector::leave()
{
  views[static_cast<uint>(currentRange.second)][currentRange.first]->deleteSelection();
  frequency->setText(QString("Не выбран"));
}