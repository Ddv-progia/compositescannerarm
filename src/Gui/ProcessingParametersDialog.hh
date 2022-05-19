/*
 * Gui/ProcessingParametersDialog.hh
 */

#pragma once

#include <QtWidgets/QDialog>

#include "Core/ScanData.hh"
#include "ui_ProcessingParametersDialog.h"
#include "ui_KernelDialog.h"
#include "Gui/AssignColorForColorBarDialog.hh"


class KernelDialog:public QDialog
	{
		Q_OBJECT
		Ui::kernelDialog ui;
		Kernel params;
	public:
		KernelDialog(Kernel& params,QWidget* parent = 0);
		Q_SLOT void updateParameters();
		Kernel getKernelParams()
		{
			return params;
		}
	};

class ProcessingParametersDialog : public QDialog
{
  Q_OBJECT

public:
  ProcessingParametersDialog(const ProcessingParameters& params, bool technological, QWidget* parent = 0);
  ProcessingParameters getProcessingParameters() const;
  Ui::ProcessingParametersDialog ui;
  AssignColorForColorBarDialog* colorDialog;

private:
  ProcessingParameters params;

  void fillWidgets();
  Q_SLOT void updateParameters();

  Q_SLOT void rangesAdd();
  Q_SLOT void rangesSplit();
  Q_SLOT void rangesChangeExtremum();
  Q_SLOT void rangesRemove();
  Q_SLOT void rangesClear();
  Q_SLOT void rangesMoveTop();
  Q_SLOT void rangesMoveUp();
  Q_SLOT void rangesMoveDown();
  Q_SLOT void rangesMoveBottom();
  Q_SLOT void controlSmoothingSpinBox();

  Q_SLOT void defectsAdd();
  Q_SLOT void defectsRemove();
  Q_SLOT void defectsClear();
  Q_SLOT void defectsMoveTop();
  Q_SLOT void defectsMoveUp();
  Q_SLOT void defectsMoveDown();
  Q_SLOT void defectsMoveBottom(); 
  
  Q_SLOT void defectSearchingRangesAdd();
  Q_SLOT void defectSearchingRangesRemove();
  Q_SLOT void defectSearchingRangesClear();
  Q_SLOT void defectSearchingRangesMoveTop();
  Q_SLOT void defectSearchingRangesMoveUp();
  Q_SLOT void defectSearchingRangesMoveDown();
  Q_SLOT void defectSearchingRangesMoveBottom();

  Q_SLOT void showUnionKernelDialog();
  Q_SLOT void showSeparationKernelDialog();
};
