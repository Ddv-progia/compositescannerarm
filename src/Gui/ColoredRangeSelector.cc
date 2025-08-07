#include "Gui/ColoredRangeSelector.hh"

#include <QLayout>
#include <QLabel>
#include <QKeyEvent>
#include <qwt_interval.h>


bool RangeView::eventFilter(QObject* watched, QEvent* event)
{
  if(event->type() == QEvent::MouseButtonRelease) {
    select();
    emit rangeSelected(this);
    return true;
  } else
    return QFrame::eventFilter(watched, event);
}


RangeView::RangeView(QRgb rangeColor, FrequencyRange newRange, QWidget* parent): QFrame(parent), range(newRange)
{
  this->setFrameStyle(QFrame::Box | QFrame::Plain);
  setBackgroundRole(QPalette::Window);
  setAutoFillBackground(true);
  this->setFrameShape(QFrame::NoFrame);
  this->setLineWidth(0);
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, QColor::fromRgb(rangeColor));
  this->setPalette(palette);
  this->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

  installEventFilter(this);
}

void RangeView::select()
{
  this->setFrameShape(QFrame::Panel);
  this->setLineWidth(1);
}
void RangeView::deleteSelection()
{
  this->setFrameShape(QFrame::NoFrame);
  this->setLineWidth(0);
}

QSize RangeView::sizeHint () const
{
  return QSize(12, 12);
}

FrequencyRange RangeView::getRange() const
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
    if (keyEvent->modifiers() == Qt::NoModifier) {
      switch (keyEvent->key()) {
      case Qt::Key_Down:
        //*******
        //if(currentRange.second != Extremum::Min) { 
        //  view->deleteSelection();
        //  currentRange.second = Extremum::Min;
        //  emit selectRange(currentRange.first, currentRange.second);
        //  view->select();
        //  setFrequencyText(view->getRange());
        //}
        if (currentRange.second != Extremum::Diff) {

            view->deleteSelection();
            if (currentRange.second == Extremum::Aver) {
                currentRange.second = Extremum::Diff;
            }else if (currentRange.second == Extremum::Min) {
            currentRange.second = Extremum::Aver;
            }
            else currentRange.second = Extremum::Min;
            setFrequencyText(view->getRange());
            auto& viewNew = views[static_cast<uint>(currentRange.second)][currentRange.first];
            viewNew->select();
            emit selectRange(currentRange.first, currentRange.second);
        }
        //*******

        return true;
      case Qt::Key_Up:
        //*******
        //if(currentRange.second != Extremum::Max) {
        //  view->deleteSelection();
        //  currentRange.second = Extremum::Max;
        //  emit selectRange(currentRange.first, currentRange.second);
        //  view->select();
        //  setFrequencyText(view->getRange());
        //}
        if (currentRange.second != Extremum::Max) {
            view->deleteSelection();
            if (currentRange.second == Extremum::Diff) {
                currentRange.second = Extremum::Aver;
            } else if (currentRange.second == Extremum::Min) {
                currentRange.second = Extremum::Max;
            }
            else currentRange.second = Extremum::Min;
            setFrequencyText(view->getRange());
            auto& viewNew = views[static_cast<uint>(currentRange.second)][currentRange.first];
            viewNew->select();
            emit selectRange(currentRange.first, currentRange.second);
        }
        //*******
        return true;
      case Qt::Key_Left:
        if(currentRange.first > 0) {
          view->deleteSelection();
          currentRange.first--;
          emit selectRange(currentRange.first, currentRange.second);
          //*******
          //view->select(); 
          auto& viewNew = views[static_cast<uint>(currentRange.second)][currentRange.first];
          viewNew->select();
          //*******
          setFrequencyText(viewNew->getRange());
        }
        return true;
      case Qt::Key_Right:
        if(currentRange.first < views[0].size() - 1) {
          view->deleteSelection();
          currentRange.first++;
          emit selectRange(currentRange.first, currentRange.second);
          //*******
          //view->select();
          auto& viewNew = views[static_cast<uint>(currentRange.second)][currentRange.first];
          viewNew->select();
          //*******
          setFrequencyText(viewNew->getRange());
        }
        return true;
      }
    }
  }
  return QObject::eventFilter(watched, event);
}


ColoredRangeSelector::ColoredRangeSelector(std::vector<float> maxs, std::vector<float> mins, std::vector<float> avers, std::vector<float> diffs, std::vector<FrequencyRange> ranges,
    QwtColorMap* newColorMap, QWidget* parent) : QFrame(parent)
{
  QwtInterval interval(static_cast<double>(*std::min_element(mins.begin(), mins.end())), static_cast<double>(*std::max_element(maxs.begin(),
                       maxs.end())));
  auto minLayout = new QHBoxLayout;
  auto maxLayout = new QHBoxLayout;
  auto averLayout = new QHBoxLayout;
  auto diffLayout = new QHBoxLayout;
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

  minLayout->setStretch(0, 0);
  maxLayout->setStretch(0, 0);
  averLayout->setStretch(0, 0);
  diffLayout->setStretch(0, 0);

  views.resize(4);

  this->setFrameStyle(QFrame::StyledPanel);

  for(int i = 0; i < ranges.size(); i++) {
      float val = mins[i];
      if (isnan(mins[i])) {
          val = 0;
      }
    auto minView = new RangeView(newColorMap->rgb(interval, mins[i]), ranges[i]);
    val = maxs[i];
    if (isnan(maxs[i])) {
        val = 0;
    }
    auto maxView = new RangeView(newColorMap->rgb(interval, maxs[i]), ranges[i]);
    val = avers[i];
    if (isnan(avers[i])) {
        val = 0;
    }
    auto averView = new RangeView(newColorMap->rgb(interval, avers[i]), ranges[i]);
    val = diffs[i];
    if (isnan(diffs[i])) {
        val = 0;
    }
    auto diffView = new RangeView(newColorMap->rgb(interval, diffs[i]), ranges[i]);

    views[0].push_back(maxView);
    views[1].push_back(minView);
    views[2].push_back(averView);
    views[3].push_back(diffView);

    maxLayout->addWidget(maxView);
    minLayout->addWidget(minView);
    averLayout->addWidget(averView);
    diffLayout->addWidget(diffView);

    connect(maxView, SIGNAL(rangeSelected(RangeView*)), SLOT(selectRangeMax(RangeView*)));
    connect(minView, SIGNAL(rangeSelected(RangeView*)), SLOT(selectRangeMin(RangeView*)));
    connect(averView, SIGNAL(rangeSelected(RangeView*)), SLOT(selectRangeAver(RangeView*)));
    connect(diffView, SIGNAL(rangeSelected(RangeView*)), SLOT(selectRangeDiff(RangeView*)));
  }

  frequency = new QLabel;

  freqLayout->addWidget(new QLabel(QString("Диапазон")));
  freqLayout->addWidget(frequency);

  selectorLayout->addLayout(maxLayout);
  selectorLayout->addLayout(minLayout);
  selectorLayout->addLayout(averLayout);
  selectorLayout->addLayout(diffLayout);

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

void ColoredRangeSelector::selectRangeMax(RangeView* view)
{
    selectRange(view, Extremum::Max); //*******
}

//*******
void ColoredRangeSelector::selectRangeAver(RangeView* view)
{
    selectRange(view, Extremum::Aver);
}

void ColoredRangeSelector::selectRangeDiff(RangeView* view)
{
    selectRange(view, Extremum::Diff); 
}

void ColoredRangeSelector::selectRange(RangeView* viewIn, Extremum ex)
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

void ColoredRangeSelector::selectRangeMin(RangeView* view)
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