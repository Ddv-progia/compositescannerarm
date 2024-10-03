/*
 * Gui/ProcessingParametersDialog.cc
 */

#include <QtCore/QAbstractTableModel>
#include <QtWidgets/QComboBox>
#include "Gui/RangeDelegate.hh"
#include "Gui/ExtremumDelegate.hh"
#include "opencv2/opencv.hpp"

#include "Gui/ProcessingParametersDialog.hh"



KernelDialog::KernelDialog(Kernel& params,QWidget* parent):params(params),QDialog(parent)
{
	ui.setupUi(this);
	ui.centerX->setText(QString::number(params.centerx));
	ui.centerY->setText(QString::number(params.centery));
	ui.heightEdit->setText(QString::number(params.height));
	ui.widthEdit->setText(QString::number(params.width));
	if(params.kernelType == cv::MORPH_ELLIPSE)
		ui.shapeComboBox->setCurrentText(QString("Эллипс"));
	else if (params.kernelType == cv::MORPH_CROSS)
		ui.shapeComboBox->setCurrentText(QString("Крест"));
	else if (params.kernelType == cv::MORPH_RECT)
		ui.shapeComboBox->setCurrentText(QString("Прямоугольник"));
	else ui.shapeComboBox->setCurrentIndex(0);

	connect(this, SIGNAL(accepted()),SLOT(updateParameters()));
}

void KernelDialog::updateParameters()
{
	params.centerx = ui.centerX->text().toInt();
	params.centery = ui.centerY->text().toInt();
	params.height = ui.heightEdit->text().toInt();
	params.width = ui.widthEdit->text().toInt();

	auto text = ui.shapeComboBox->currentText();
	if(text == QString("Эллипс"))
		params.kernelType = cv::MORPH_ELLIPSE;
	else if (text == QString("Крест"))
		params.kernelType = cv::MORPH_CROSS;
	else if (text == QString("Прямоугольник"))
		params.kernelType = cv::MORPH_RECT;
	else 
		params.kernelType = cv::MORPH_ELLIPSE;
}


namespace {
  class RangesTableModel : public QAbstractTableModel
  {
    std::vector<FrequencyRange>& ranges;
    std::vector < ::Extremum>& extremums;
  public:
      explicit RangesTableModel(std::vector<FrequencyRange>& ranges, std::vector < ::Extremum>& extremums, QObject* parent = 0)
          : QAbstractTableModel(parent), ranges(ranges), extremums(extremums)
    { }

    virtual int columnCount(const QModelIndex& = QModelIndex()) const override
    {
//*******
  //      return 2;
      return 3;
//*******
    }

    virtual int rowCount(const QModelIndex& = QModelIndex()) const override
    {
      return ranges.size();
    }

    virtual Qt::ItemFlags flags(const QModelIndex&) const override
    {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
      switch (role) {
      case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
          switch (section) {
          case 0: return "От";
          case 1: return "До";
          case 2: return "Экстремум";
          }

        case Qt::Vertical:
          return QString::number(section + 1);
        }
      }

      return QVariant();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
      switch (role) {
      case Qt::DisplayRole:
      case Qt::EditRole:
        switch (index.column()) {
        case 0: return QString::number(ranges[index.row()].from);
        case 1: return QString::number(ranges[index.row()].to);
        case 2: 
            switch (extremums[index.row()]) {
            case ::Extremum::Aver:
                return QString("Aver");
            case ::Extremum::Min:
                return QString("Min");
            case ::Extremum::Diff:
                return QString("Diff");
            default:
            case ::Extremum::Max:
                return QString("Max");
            }
        }
      }

      return QVariant();
    }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
    {
      switch (role) {
      case Qt::EditRole:
        switch (index.column()) {
        case 0:
          ranges[index.row()].from = value.toDouble();
          return true;
        case 1:
          ranges[index.row()].to = value.toDouble();
          return true;
        case 2:
            auto indexRow = index.row();
            if (indexRow >= extremums.size()){
                extremums.resize(indexRow+1);
            }
            switch (value.toInt()) {
            case 0:
                extremums[index.row()] = Extremum::Max;
                break;
            case 1:
                extremums[index.row()] = Extremum::Min;
                break;
            case 2:
                extremums[index.row()] = Extremum::Aver;
                break;
            case 3:
                extremums[index.row()] = Extremum::Diff;
                break;
            }
          return true;
        }
      }

      return false;
    }

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginInsertRows(parent, row, row + count - 1);
      ranges.insert(ranges.begin() + row, count, FrequencyRange{ 0, 0 });
      extremums.insert(extremums.begin() + row, count, Extremum::Max);
      endInsertRows();
      return true;
    }

    virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationRow) override
    {
      if (beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow)) {
        std::vector<FrequencyRange> moved(ranges.begin() + sourceRow, ranges.begin() + sourceRow + count);
        ranges.erase(ranges.begin() + sourceRow, ranges.begin() + sourceRow + count);
        std::vector<Extremum> movedExtremums(extremums.begin() + sourceRow, extremums.begin() + sourceRow + count);
        extremums.erase(extremums.begin() + sourceRow, extremums.begin() + sourceRow + count);

        if (destinationRow > sourceRow) {
          ranges.insert(ranges.begin() + (destinationRow - count), moved.begin(), moved.end());
          extremums.insert(extremums.begin() + (destinationRow - count), movedExtremums.begin(), movedExtremums.end());
        } else {
          ranges.insert(ranges.begin() + destinationRow, moved.begin(), moved.end());
          extremums.insert(extremums.begin() + destinationRow, movedExtremums.begin(), movedExtremums.end());
        }
        endMoveRows();
        return true;
      } else {
        return false;
      }
    }

    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginRemoveRows(parent, row, row + count - 1);
      ranges.erase(ranges.begin() + row, ranges.begin() + row + count);
      extremums.erase(extremums.begin() + row, extremums.begin() + row + count);
      endRemoveRows();
      return true;
    }
  };

  class DefectsTableModel : public QAbstractTableModel
  {
    std::vector<DefectKindView>& defects;
    std::vector<FrequencyRange>& ranges;
    std::vector < ::Extremum>& extremums;


  public:
    explicit DefectsTableModel(std::vector<DefectKindView>& defects, std::vector<FrequencyRange>& ranges, std::vector < ::Extremum>& extremums, QObject* parent = 0)
      : QAbstractTableModel(parent), defects(defects), ranges(ranges), extremums(extremums)
    { }

    virtual int columnCount(const QModelIndex& = QModelIndex()) const override
    {
      return 13;
    }

    virtual int rowCount(const QModelIndex& = QModelIndex()) const override
    {
      return defects.size();
    }

    virtual Qt::ItemFlags flags(const QModelIndex&) const override
    {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
      switch (role) {
      case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
          switch (section) {
          case 0: return "Название";
          case 1: return "Красный\nдиапазон";
          case 2: return "Красный\nпредел";
          case 3: return "Красный\nусиление";
		  case 4: return "Красный\nменьшие значения";
          case 5: return "Синий\nдиапазон";
          case 6: return "Синий\nпредел";
          case 7: return "Синий\nусиление";
		  case 8: return "Синий\nменьшие значения";
          case 9: return "Зелёный\nдиапазон";
          case 10: return "Зелёный\nпредел";
          case 11: return "Зелёный\nусиление";
		  case 12: return "Зелёный\nменьшие значения";
          }

        case Qt::Vertical:
          return QString::number(section + 1);
        }
      }

      return QVariant();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
      QString str = "Min";

      switch (role) {
      case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return QString::fromStdString(defects[index.row()].title);
        case 1: 
            str = "Min";
            if (extremums[defects[index.row()].red.range] == ::Extremum::Max) {
                str = "Max";
            }
            else if (extremums[defects[index.row()].red.range] == ::Extremum::Aver) str = "Aver";
            else if (extremums[defects[index.row()].red.range] == ::Extremum::Diff) str = "Diff";
            return QString("%1 - %2 : %3").arg(ranges[defects[index.row()].red.range].from).arg(ranges[defects[index.row()].red.range].to).arg(str);
        case 2: return defects[index.row()].red.limit;
        case 3: return defects[index.row()].red.amplification;
		case 4: return defects[index.row()].red.useLeastValues;
        case 5:
            str = "Min";
            if (extremums[defects[index.row()].blue.range] == ::Extremum::Max) {
                str = "Max";
            }
            else if (extremums[defects[index.row()].blue.range] == ::Extremum::Aver) str = "Aver";
            else if (extremums[defects[index.row()].blue.range] == ::Extremum::Diff) str = "Diff";
            return QString("%1 - %2 : %3").arg(ranges[defects[index.row()].blue.range].from).arg(ranges[defects[index.row()].blue.range].to).arg(str);
        case 6: return defects[index.row()].blue.limit;
        case 7: return defects[index.row()].blue.amplification;
		case 8: return defects[index.row()].blue.useLeastValues;
        case 9: 
            str = "Min";
            if (extremums[defects[index.row()].green.range] == ::Extremum::Max) {
                str = "Max";
            }
            else if (extremums[defects[index.row()].green.range] == ::Extremum::Aver) str = "Aver";
            else if (extremums[defects[index.row()].green.range] == ::Extremum::Diff) str = "Diff";
            return QString("%1 - %2 : %3").arg(ranges[defects[index.row()].green.range].from).arg(ranges[defects[index.row()].green.range].to).arg(str);
        case 10: return defects[index.row()].green.limit;
        case 11: return defects[index.row()].green.amplification;
		case 12: return defects[index.row()].green.useLeastValues;
        }
      case Qt::EditRole:
        switch (index.column()) {
        case 0: return QString::fromStdString(defects[index.row()].title);
        case 1: return defects[index.row()].red.range;
        case 2: return defects[index.row()].red.limit;
        case 3: return defects[index.row()].red.amplification;
		case 4: return defects[index.row()].red.useLeastValues;
        case 5: return defects[index.row()].blue.range;
		case 6: return defects[index.row()].blue.limit;
        case 7: return defects[index.row()].blue.amplification;
		case 8: return defects[index.row()].blue.useLeastValues;
        case 9: return defects[index.row()].green.range;
		case 10: return defects[index.row()].green.limit;
        case 11: return defects[index.row()].green.amplification;
		case 12: return defects[index.row()].green.useLeastValues;
        }
      }

      return QVariant();
    }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
    {
      switch (role) {
      case Qt::EditRole:
        switch (index.column()) {
        case 0:
          defects[index.row()].title = value.toString().toStdString();
          return true;
        case 1:
          defects[index.row()].red.range = value.toUInt();
          return true;
        case 2:
          defects[index.row()].red.limit = value.toDouble();
          return true;
        case 3:
          defects[index.row()].red.amplification = value.toDouble();
          return true;
		case 4:
		  defects[index.row()].red.useLeastValues = value.toBool();
          return true;
        case 5:
          defects[index.row()].blue.range = value.toUInt();
          return true;
        case 6:
          defects[index.row()].blue.limit = value.toDouble();
          return true;
        case 7:
          defects[index.row()].blue.amplification = value.toDouble();
          return true;
		case 8:
		  defects[index.row()].blue.useLeastValues = value.toBool();
          return true;
        case 9:
          defects[index.row()].green.range = value.toUInt();
          return true;
        case 10:
          defects[index.row()].green.limit = value.toDouble();
          return true;
        case 11: 
          defects[index.row()].green.amplification = value.toDouble();
          return true;
		case 12:
		  defects[index.row()].green.useLeastValues = value.toBool();
          return true;
        }
      }

      return false;
    }

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginInsertRows(parent, row, row + count - 1);
      defects.insert(defects.begin() + row, count, DefectKindView{ "", DefectChannel{ 0, 0, 0, false }, DefectChannel{ 0, 0, 0, false }, DefectChannel{ 0, 0, 0, false } });
      endInsertRows();

      return true;
    }

    virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationRow) override
    {
      if (beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow)) {
        std::vector<DefectKindView> moved(defects.begin() + sourceRow, defects.begin() + sourceRow + count);
        defects.erase(defects.begin() + sourceRow, defects.begin() + sourceRow + count);

        if (destinationRow > sourceRow) {
          defects.insert(defects.begin() + (destinationRow - count), moved.begin(), moved.end());
        } else {
          defects.insert(defects.begin() + destinationRow, moved.begin(), moved.end());
        }
        endMoveRows();
        return true;
      } else {
        return false;
      }
    }

    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginRemoveRows(parent, row, row + count - 1);
      defects.erase(defects.begin() + row, defects.begin() + row + count);
      endRemoveRows();

      return true;
    }
  };

  class DefectRangesTableModel: public QAbstractTableModel
  {
	std::vector<DefectSearchingRange>& defectRanges;
	std::vector<FrequencyRange>& ranges;
    std::vector <::Extremum>& extremums;

  public:
	explicit DefectRangesTableModel(std::vector<DefectSearchingRange>& defectRanges,std::vector<FrequencyRange>& ranges, std::vector <::Extremum>& extremums, QObject* parent = 0)
      : QAbstractTableModel(parent), defectRanges(defectRanges), ranges(ranges), extremums(extremums)
    { }

    virtual int columnCount(const QModelIndex& = QModelIndex()) const override
    {
      return 3; 
    }

    virtual int rowCount(const QModelIndex& = QModelIndex()) const override
    {
      return defectRanges.size();
    }

    virtual Qt::ItemFlags flags(const QModelIndex&) const override
    {
      return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
      switch (role) {
      case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
          switch (section) {
          case 0: return "Диапазон";
          case 1: return "Минимальное значение";
		  case 2: return "Максимальное значение";
          }

        case Qt::Vertical:
          return QString::number(section + 1);
        }
      }

      return QVariant();
    }

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
      switch (role) {
      case Qt::DisplayRole:
	    switch (index.column()) {
		case 0:{//проверка добавлена для совместимости со старыми сканами
				FrequencyRange range;
				range = ranges[defectRanges[index.row()].range < ranges.size() ? defectRanges[index.row()].range : 0];
                auto extremum = extremums[defectRanges[index.row()].range < ranges.size() ? defectRanges[index.row()].range : 0];
                QString str = "Max";
                if (extremum == ::Extremum::Min ) str = "Min";
                if (extremum == ::Extremum::Aver ) str = "Aver";
                if (extremum == ::Extremum::Diff ) str = "Diff";
				return QString("%1-%2 : %3").arg(range.from).arg(range.to).arg(str);
			}
		case 1: return QString::number(defectRanges[index.row()].minimumValue);
		case 2: return QString::number(defectRanges[index.row()].maximumValue);
        }
      case Qt::EditRole:
        switch (index.column()) {
		case 0: return defectRanges[index.row()].range;
		case 1: return defectRanges[index.row()].minimumValue;
		case 2: return defectRanges[index.row()].maximumValue;
        }
      }

      return QVariant();
    }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
    {
      switch (role) {
      case Qt::EditRole:
        switch (index.column()) {
        case 0:
		  defectRanges[index.row()].range = value.toUInt();
          return true;
        case 1:
		  defectRanges[index.row()].minimumValue = value.toDouble();
          return true;
		case 2:
		  defectRanges[index.row()].maximumValue = value.toDouble();
          return true;
        }
      }

      return false;
    }

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginInsertRows(parent, row, row + count - 1);
	  defectRanges.insert(defectRanges.begin() + row, count,DefectSearchingRange());
      endInsertRows();

      return true;
    }

    virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationRow) override
    {
      if (beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationRow)) {
		std::vector<DefectSearchingRange> moved(defectRanges.begin() + sourceRow, defectRanges.begin() + sourceRow + count);
        defectRanges.erase(defectRanges.begin() + sourceRow, defectRanges.begin() + sourceRow + count);

        if (destinationRow > sourceRow) {
          defectRanges.insert(defectRanges.begin() + (destinationRow - count), moved.begin(), moved.end());
        } else {
          defectRanges.insert(defectRanges.begin() + destinationRow, moved.begin(), moved.end());
        }
        endMoveRows();
        return true;
      } else {
        return false;
      }
    }

    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginRemoveRows(parent, row, row + count - 1);
      defectRanges.erase(defectRanges.begin() + row, defectRanges.begin() + row + count);
      endRemoveRows();

      return true;
    }
  };

}

ProcessingParametersDialog::ProcessingParametersDialog(const ProcessingParameters& params, bool technological, QWidget* parent)
  : QDialog(parent), params(params)
{
  ui.setupUi(this);
  if (technological) {
    setWindowTitle("Технологические параметры обработки");
  } else {
    setWindowTitle("Параметры обработки");
  }

  // Скрываем параметр ui.peakPauseCountBox "Длительность паузы после пика", 
  // т.к. его использование закомменчено.
  ui.label_16->setVisible(true); // метка "Длительность паузы после пика"
  ui.peakPauseCountBox->setVisible(true);
  colorDialog = new AssignColorForColorBarDialog(params, false,  this);
  // Прячем кнопки перемещения итемов, т.к. реализация не завершена.
  colorDialog->ui.moveTopButton->setVisible(false);
  colorDialog->ui.moveBottomButton->setVisible(false);
  colorDialog->ui.moveDownButton->setVisible(false);
  colorDialog->ui.moveUpButton->setVisible(false);

  colorDialog->setAttribute(Qt::WA_DeleteOnClose, true);

  ui.horizontalLayout_14->addWidget(colorDialog);

  connect(ui.rangesAddButton, SIGNAL(clicked()), this, SLOT(rangesAdd()));
  connect(ui.rangesRemoveButton, SIGNAL(clicked()), this, SLOT(rangesRemove()));
  connect(ui.rangesClearButton, SIGNAL(clicked()), this, SLOT(rangesClear()));
  connect(ui.rangesMoveTopButton, SIGNAL(clicked()), this, SLOT(rangesMoveTop()));
  connect(ui.rangesMoveUpButton, SIGNAL(clicked()), this, SLOT(rangesMoveUp()));
  connect(ui.rangesMoveDownButton, SIGNAL(clicked()), this, SLOT(rangesMoveDown()));
  connect(ui.rangesMoveBottomButton, SIGNAL(clicked()), this, SLOT(rangesMoveBottom()));
  connect(ui.splitRangeButton, SIGNAL(clicked()), this, SLOT(rangesSplit()));
  connect(ui.changeExtremumPushButton, SIGNAL(clicked()), this, SLOT(rangesChangeExtremum()));
  connect(ui.defectsAddButton, SIGNAL(clicked()), this, SLOT(defectsAdd()));
  connect(ui.defectsRemoveButton, SIGNAL(clicked()), this, SLOT(defectsRemove()));
  connect(ui.defectsClearButton, SIGNAL(clicked()), this, SLOT(defectsClear()));
  connect(ui.defectsMoveTopButton, SIGNAL(clicked()), this, SLOT(defectsMoveTop()));
  connect(ui.defectsMoveUpButton, SIGNAL(clicked()), this, SLOT(defectsMoveUp()));
  connect(ui.defectsMoveDownButton, SIGNAL(clicked()), this, SLOT(defectsMoveDown()));
  connect(ui.defectsMoveBottomButton, SIGNAL(clicked()), this, SLOT(defectsMoveBottom()));
  connect(ui.defectRangesAddButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesAdd()));
  connect(ui.defectRangesRemoveButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesRemove()));
  connect(ui.defectRangesClearButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesClear()));
  connect(ui.defectRangesMoveTopButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesMoveTop()));
  connect(ui.defectRangesMoveUpButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesMoveUp()));
  connect(ui.defectRangesMoveDownButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesMoveDown()));
  connect(ui.defectRangesMoveBottomButton, SIGNAL(clicked()), this, SLOT(defectSearchingRangesMoveBottom()));
  connect(ui.smoothingPointsCountBox,SIGNAL(editingFinished()),this,SLOT(controlSmoothingSpinBox()));

  connect(ui.unionKernelButton,SIGNAL(clicked()),SLOT(showUnionKernelDialog()));
  connect(ui.separationKernelButton,SIGNAL(clicked()),SLOT(showSeparationKernelDialog()));

  connect(this, SIGNAL(accepted()), this, SLOT(updateParameters()));

  fillWidgets();
}

  void ProcessingParametersDialog::showUnionKernelDialog()
  {
	  auto dlg = new KernelDialog(params.defectSearching.unionKernel);
	  dlg->setModal(true);
	  if(dlg->exec() == QDialog::Accepted){
		  params.defectSearching.unionKernel = dlg->getKernelParams();
	  }
	  else
		return;
  }

  void ProcessingParametersDialog::showSeparationKernelDialog()
  {
	  auto dlg = new KernelDialog(params.defectSearching.separationKernel);
	  dlg->setModal(true);
	  if(dlg->exec() == QDialog::Accepted){
		  params.defectSearching.separationKernel = dlg->getKernelParams();
	  }
	  else
		return;
  }

void ProcessingParametersDialog::controlSmoothingSpinBox()
{
  if (!(ui.smoothingPointsCountBox->value()%2)){
    ui.smoothingPointsCountBox->setValue(ui.smoothingPointsCountBox->value()+1);
  }
}

ProcessingParameters ProcessingParametersDialog::getProcessingParameters() const
{
  return params;
}

void ProcessingParametersDialog::fillWidgets()
{
  ui.initialSkipBox->setValue(params.initialSkip);
  ui.stepForSplitFrequencyRangesSpinBox->setValue(params.stepForSplitFrequencyRanges);
  ui.peakMagnitudeLimitBox->setValue(params.peakMagnitudeLimit);
  ui.peakBackstepBox->setValue(params.peakBackstep);
  ui.peakForestepBox->setValue(params.peakForestep);
  ui.peakForestepSoundBox->setValue(params.headAndScanCollectorParameters.peakForestepSound);
  ui.firstStepShiftBox->setValue(params.headAndScanCollectorParameters.firstStepShift);

  ui.peakPauseCountBox->setValue(params.peakPauseCount);
  ui.countOfPeakToCatchForAreaBox->setValue(params.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox);
  ui.columnModelOrderBox->setValue(params.columnModelOrder);
  ui.smoothingPointsCountBox->setValue(params.smoothingPointsCount);
  ui.normalizeCheckBox->setChecked(params.shouldNormalize);
  ui.relateCheckBox->setChecked(params.shouldRelate);
  ui.useSubRangesCheckBox->setChecked(params.useSubRanges);

  ui.redStartBox->setValue(params.defectRendering.red.startValue);
  ui.redEndBox->setValue(params.defectRendering.red.endValue);
  ui.redLightBox->setValue(params.defectRendering.red.lightnessStart);

  ui.greenStartBox->setValue(params.defectRendering.green.startValue);
  ui.greenEndBox->setValue(params.defectRendering.green.endValue);
  ui.greenLightBox->setValue(params.defectRendering.green.lightnessStart);

  ui.blueStartBox->setValue(params.defectRendering.blue.startValue);
  ui.blueEndBox->setValue(params.defectRendering.blue.endValue);
  ui.blueLightBox->setValue(params.defectRendering.blue.lightnessStart);

  ui.fixedColorScaleCheckBox->setChecked(params.defectRendering.fixedColorScale);

  ui.redFixedBox->setValue(params.defectRendering.redFixed);
  ui.greenFixedBox->setValue(params.defectRendering.greenFixed);
  ui.blueFixedBox->setValue(params.defectRendering.blueFixed);
  ui.rangesTableView->setModel(new RangesTableModel(this->params.ranges, this->params.extremumOfRanges, this));
  ui.rangesTableView->setItemDelegateForColumn(2, new ExtremumDelegate(this));

  ui.defectsTableView->setModel(new DefectsTableModel(this->params.defectPoints, this->params.ranges, this->params.extremumOfRanges, this));
  ui.defectsTableView->setItemDelegateForColumn(1, new DefectRangeDelegate(this->params.ranges, this->params.extremumOfRanges, this));
  ui.defectsTableView->setItemDelegateForColumn(5, new DefectRangeDelegate(this->params.ranges, this->params.extremumOfRanges, this));
  ui.defectsTableView->setItemDelegateForColumn(9, new DefectRangeDelegate(this->params.ranges, this->params.extremumOfRanges, this));

  ui.blurCheckBox->setChecked(this->params.defectSearching.useBlur);
  ui.blurHEdit->setText(QString::number(this->params.defectSearching.blurHeight));
  ui.blurWEdit->setText(QString::number(this->params.defectSearching.blurWidth));
  ui.unionCheckBox->setChecked(this->params.defectSearching.useUnion);
  ui.separationCheckBox->setChecked(this->params.defectSearching.useSeparation);
  ui.defectRangesTableView->setModel(new DefectRangesTableModel(this->params.defectSearching.defectRanges,this->params.ranges, this->params.extremumOfRanges,this));
  ui.defectRangesTableView->setItemDelegateForColumn(0, new DefectRangeDelegate(this->params.ranges, this->params.extremumOfRanges, this));
  ui.approximationComboBox->setCurrentIndex(this->params.defectSearching.edgesApproximationType-1);
  ui.defectInterpretationComboBox->setCurrentIndex((int)this->params.defectSearching.isDefectInside);
  ui.minAreaOfDefectDblSpinBox->setValue(this->params.defectSearching.minDefectArea);
  
  ui.heightBox->setValue(this->params.headAndScanCollectorParameters.height);
  ui.widthBox->setValue(this->params.headAndScanCollectorParameters.width);
  ui.spinBoxHeadsSampleRate->setValue(this->params.headAndScanCollectorParameters.headsSampleRate);
  ui.spinBoxSoundsSampleRate->setValue(this->params.headAndScanCollectorParameters.soundsSampleRate);
  ui.spinBoxMaxRecordingTime->setValue(this->params.headAndScanCollectorParameters.maximumTimeMinutes);

  if (this->params.headAndScanCollectorParameters.needPackInSquare) {
      ui.rbPackInSquare->setChecked(true);
  }
  if (this->params.headAndScanCollectorParameters.needPackInLine) {
      ui.rbPackInLine->setChecked(true);
  }
}

void ProcessingParametersDialog::updateParameters()
{
  params.colorStopsList = (colorDialog->getProcessingParameters()).colorStopsList;
  params.initialSkip = ui.initialSkipBox->value();
  params.stepForSplitFrequencyRanges = ui.stepForSplitFrequencyRangesSpinBox->value();
  params.smoothingPointsCount = ui.smoothingPointsCountBox->value();
  params.shouldNormalize = ui.normalizeCheckBox->isChecked();
  params.shouldRelate = ui.relateCheckBox->isChecked();
  params.useSubRanges = ui.useSubRangesCheckBox->isChecked();
  params.peakMagnitudeLimit = ui.peakMagnitudeLimitBox->value();
  params.peakBackstep = ui.peakBackstepBox->value();
  params.peakForestep = ui.peakForestepBox->value();
  params.headAndScanCollectorParameters.peakForestepSound = ui.peakForestepSoundBox->value();
  params.headAndScanCollectorParameters.firstStepShift = ui.firstStepShiftBox->value();
  params.peakPauseCount = unsigned int(ui.peakPauseCountBox->value());
  params.headAndScanCollectorParameters.countOfPeakToCatchForAreaBox = unsigned int(ui.countOfPeakToCatchForAreaBox->value());

  params.headAndScanCollectorParameters.height             = ui.heightBox->value();
  params.headAndScanCollectorParameters.width              = ui.widthBox->value();
  params.headAndScanCollectorParameters.headsSampleRate    = ui.spinBoxHeadsSampleRate->value();
  params.headAndScanCollectorParameters.soundsSampleRate   = ui.spinBoxSoundsSampleRate->value();
  params.headAndScanCollectorParameters.maximumTimeMinutes = ui.spinBoxMaxRecordingTime->value();
  bool Sbool = ui.rbPackInSquare->isChecked();
  bool Lbool = ui.rbPackInLine->isChecked();;
      params.headAndScanCollectorParameters.needPackInSquare = Sbool;
      params.headAndScanCollectorParameters.needPackInLine = Lbool;

  params.columnModelOrder = ui.columnModelOrderBox->value();

  params.defectRendering.red.startValue = ui.redStartBox->value();
  params.defectRendering.red.endValue = ui.redEndBox->value();
  params.defectRendering.red.lightnessStart = ui.redLightBox->value();

  params.defectRendering.green.startValue = ui.greenStartBox->value();
  params.defectRendering.green.endValue = ui.greenEndBox->value();
  params.defectRendering.green.lightnessStart = ui.greenLightBox->value();

  params.defectRendering.blue.startValue = ui.blueStartBox->value();
  params.defectRendering.blue.endValue = ui.blueEndBox->value();
  params.defectRendering.blue.lightnessStart = ui.blueLightBox->value();

  params.defectRendering.redFixed = ui.redFixedBox->value();
  params.defectRendering.greenFixed = ui.greenFixedBox->value();
  params.defectRendering.blueFixed = ui.blueFixedBox->value();

  params.defectRendering.fixedColorScale = ui.fixedColorScaleCheckBox->isChecked();

  params.defectSearching.useBlur = ui.blurCheckBox->isChecked();
  params.defectSearching.blurHeight = ui.blurHEdit->text().toInt();
  params.defectSearching.blurWidth = ui.blurWEdit->text().toInt();
  params.defectSearching.useSeparation = ui.separationCheckBox->isChecked();
  params.defectSearching.useUnion = ui.unionCheckBox->isChecked();

  params.defectSearching.isDefectInside = (bool)ui.defectInterpretationComboBox->currentIndex();
  params.defectSearching.minDefectArea = ui.minAreaOfDefectDblSpinBox->value();

  //CV_CHAIN_APPROX_NONE=1,
  //CV_CHAIN_APPROX_SIMPLE=2,
  //CV_CHAIN_APPROX_TC89_L1=3,
  //CV_CHAIN_APPROX_TC89_KCOS=4,
  //CV_LINK_RUNS=5

  params.defectSearching.edgesApproximationType = ui.approximationComboBox->currentIndex()+1;

}

void ProcessingParametersDialog::rangesAdd()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.rangesTableView->model()->insertRow(selection->selectedRows().front().row());
  } else {
    ui.rangesTableView->model()->insertRow(ui.rangesTableView->model()->rowCount());
  }
}

void ProcessingParametersDialog::rangesChangeExtremum()
{
    auto selection = ui.rangesTableView->selectionModel();
    if (selection->selectedRows().size()<1)
        return;
    auto extremum = ui.changeExtremumComboBox->currentIndex();
    QModelIndexList selectedRows = ui.rangesTableView->selectionModel()->selectedRows();
    for (int i = 0; i < selectedRows.count(); i++)
    {
        QModelIndex index = selectedRows.at(i);
        ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(index.row(), 2), extremum);
    }
    ui.rangesTableView->model()->dataChanged(selectedRows.at(0), selectedRows.at(selectedRows.count()-1));
    //ui.rangesTableView->model()->submit();
}

void ProcessingParametersDialog::rangesSplit()
{
    auto selection = ui.rangesTableView->selectionModel();
    if (selection->selectedRows().size()<1)
        return;
    auto row = selection->selectedRows().front().row();
    auto from = ui.rangesTableView->model()->index(row, 0).data().toInt();
    auto to = ui.rangesTableView->model()->index(row, 1).data().toInt();
    auto partsNum = ui.splitRangeSpinBox->value();
    if ((from > to)||(partsNum == 0))
        return;
    auto lengthOfParts = std::floor((to - from) / partsNum);
    for (auto i = 0; i < partsNum-1; i++) {
          ui.rangesTableView->model()->insertRow(row+i);
          ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + i, 0), from);
          ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + i, 1), from+lengthOfParts);
          ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + i, 2), 0);
          from += lengthOfParts;
          //model->submitAll();
    }
    // Последний диапазон расширяем до "to"-границы, на случай, если ширина диапазонов 
    // получилась дробной.
    ui.rangesTableView->model()->insertRow(row + partsNum - 1);
    ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + partsNum - 1, 0), from);
    ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + partsNum - 1, 1), to);
    ui.rangesTableView->model()->setData(ui.rangesTableView->model()->index(row + partsNum - 1, 2), 0);
}

void ProcessingParametersDialog::rangesRemove()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.rangesTableView->model()->removeRows(selection->selectedRows().front().row(), 
                                            selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1);
  }
}

void ProcessingParametersDialog::rangesClear()
{
  ui.rangesTableView->model()->removeRows(0, ui.rangesTableView->model()->rowCount());
}

void ProcessingParametersDialog::rangesMoveTop()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.rangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          0);
  }
}

void ProcessingParametersDialog::rangesMoveUp()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().front().row() > 0) {
    ui.rangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().front().row() - 1);
  }
}

void ProcessingParametersDialog::rangesMoveDown()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().back().row() < ui.rangesTableView->model()->rowCount() - 1) {
    ui.rangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().back().row() + 2);
  }
}

void ProcessingParametersDialog::rangesMoveBottom()
{
  auto selection = ui.rangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.rangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          ui.rangesTableView->model()->rowCount());
  }
}

void ProcessingParametersDialog::defectsAdd()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectsTableView->model()->insertRow(selection->selectedRows().front().row());
  } else {
    ui.defectsTableView->model()->insertRow(ui.defectsTableView->model()->rowCount());
  }
}

void ProcessingParametersDialog::defectsRemove()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectsTableView->model()->removeRows(selection->selectedRows().front().row(), 
                                            selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1);
  }
}

void ProcessingParametersDialog::defectsClear()
{
  ui.defectsTableView->model()->removeRows(0, ui.defectsTableView->model()->rowCount());
}

void ProcessingParametersDialog::defectsMoveTop()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectsTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          0);
  }
}

void ProcessingParametersDialog::defectsMoveUp()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().front().row() > 0) {
    ui.defectsTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().front().row() - 1);
  }
}

void ProcessingParametersDialog::defectsMoveDown()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().back().row() < ui.defectsTableView->model()->rowCount() - 1) {
    ui.defectsTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().back().row() + 2);
  }
}

void ProcessingParametersDialog::defectsMoveBottom()
{
  auto selection = ui.defectsTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectsTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          ui.defectsTableView->model()->rowCount());
  }
}


void ProcessingParametersDialog::defectSearchingRangesAdd()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectRangesTableView->model()->insertRow(selection->selectedRows().front().row());
  } else {
    ui.defectRangesTableView->model()->insertRow(ui.defectRangesTableView->model()->rowCount());
  }
}

void ProcessingParametersDialog::defectSearchingRangesRemove()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectRangesTableView->model()->removeRows(selection->selectedRows().front().row(), 
                                            selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1);
  }
}

void ProcessingParametersDialog::defectSearchingRangesClear()
{
  ui.defectRangesTableView->model()->removeRows(0, ui.defectRangesTableView->model()->rowCount());
}

void ProcessingParametersDialog::defectSearchingRangesMoveTop()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectRangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          0);
  }
}

void ProcessingParametersDialog::defectSearchingRangesMoveUp()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().front().row() > 0) {
    ui.defectRangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().front().row() - 1);
  }
}

void ProcessingParametersDialog::defectSearchingRangesMoveDown()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection() && selection->selectedRows().back().row() < ui.defectRangesTableView->model()->rowCount() - 1) {
    ui.defectRangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          selection->selectedRows().back().row() + 2);
  }
}

void ProcessingParametersDialog::defectSearchingRangesMoveBottom()
{
  auto selection = ui.defectRangesTableView->selectionModel();
  if (selection->hasSelection()) {
    ui.defectRangesTableView->model()->moveRows(QModelIndex(),
                                          selection->selectedRows().front().row(),
                                          selection->selectedRows().back().row() - selection->selectedRows().front().row() + 1,
                                          QModelIndex(),
                                          ui.defectRangesTableView->model()->rowCount());
  }
}
