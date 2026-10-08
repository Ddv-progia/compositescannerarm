#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>

#include "Core/ScanData.hh"

class DefectRangeDelegate : public QStyledItemDelegate
  {
    std::vector<FrequencyRange>& ranges;
    std::vector<Extremum> extremums;

  public:
    //explicit DefectRangeDelegate(std::vector<FrequencyRange>& ranges, std::vector < ::Extremum>& extremums, QObject* parent = 0)
    explicit DefectRangeDelegate(std::vector<FrequencyRange>& ranges, QObject* parent = 0)
        : QStyledItemDelegate(parent), ranges(ranges)
    { }

    DefectRangeDelegate(std::vector<FrequencyRange>& ranges, std::vector < ::Extremum>& extremums, QObject* parent = 0)
      : QStyledItemDelegate(parent), ranges(ranges), extremums(extremums)
    { }

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto cb = new QComboBox(parent);
      int i = 0;
      int targedRowIndex = index.row();
      int rowIndex = 0;
      for (auto& r : ranges) {
            QString strExtremum = "";
            if (extremums[rowIndex] == ::Extremum::Max) strExtremum = "Max";
            else if (extremums[rowIndex] == ::Extremum::Min) strExtremum = "Min";
            else if (extremums[rowIndex] == ::Extremum::Aver) strExtremum = "Aver";
            else if (extremums[rowIndex] == ::Extremum::Diff) strExtremum = "Diff";
            else if (extremums[rowIndex] == ::Extremum::DiffOnTable) strExtremum = "DiffOnTable";
            cb->addItem(QString("%1 - %2 : %3").arg(r.from).arg(r.to).arg(strExtremum));
            rowIndex++;
      }

      return cb;
    }

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
      static_cast<QComboBox*>(editor)->setCurrentIndex(index.data().toInt());
    }

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
      model->setData(index, static_cast<QComboBox*>(editor)->currentIndex());
    }
  };