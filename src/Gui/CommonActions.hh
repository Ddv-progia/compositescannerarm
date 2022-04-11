#pragma once

#include <QObject>
#include <UCL/PlotView/PlotItemAction.hh>
#include <QProgressDialog>

class ProgressedPlotItemAction : public uts::plotting::AbstractPlotItemAction
{
  Q_OBJECT
  std::unique_ptr<QProgressDialog> progress;
public:
  ProgressedPlotItemAction(QObject* parent = 0);

  Q_SLOT void onStart(int i);

  Q_SLOT void onFinished() const;

  Q_SLOT void onProgressed(int i) const;

  virtual void stop() = 0;
};
