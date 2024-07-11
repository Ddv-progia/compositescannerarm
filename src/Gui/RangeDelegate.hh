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
      if (extremums.size() == ranges.size()) {
          for (auto& r : ranges) {
              QString str = "Max";
              if (extremums[i] == ::Extremum::Aver) str = "Aver";
              else if (extremums[i] == ::Extremum::Min) str = "Min";
              else if (extremums[i] == ::Extremum::Diff) str = "Diff";
              i++;
              cb->addItem(QString("%1 - %2 : %3").arg(r.from).arg(r.to).arg(str));
          }
      }
      else {
          for (auto& r : ranges) cb->addItem(QString("%1 - %2").arg(r.from).arg(r.to));
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