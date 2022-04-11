#include "Gui/CommonActions.hh"


ProgressedPlotItemAction::ProgressedPlotItemAction(QObject* parent)
{
}

void ProgressedPlotItemAction::onStart(int i)
{
  progress.reset(new QProgressDialog((QString("%1: прогресс выполнения").arg(getTitle())), ("Отмена"), 0, 0, nullptr));
  progress->setAutoClose(true);
  progress->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  progress->setWindowModality(Qt::WindowModal);
  progress->setMinimumDuration(500);
  progress->setMaximum(i);
  progress->setVisible(true);
  connect(progress.get(), &QProgressDialog::canceled, this, &ProgressedPlotItemAction::stop);
}

void ProgressedPlotItemAction::onFinished() const
{
  if (progress)
    progress->cancel();
}

void ProgressedPlotItemAction::onProgressed(int i) const
{
  if (progress)
    progress->setValue(i);
}
