#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>

#include "Core/ScanData.hh"

class DefectRangeDelegate : public QStyledItemDelegate
  {
    std::vector<FrequencyRange>& ranges;
  public:
    explicit DefectRangeDelegate(std::vector<FrequencyRange>& ranges, QObject* parent = 0)
      : QStyledItemDelegate(parent), ranges(ranges)
    { }

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto cb = new QComboBox(parent);
      for (auto& r : ranges) cb->addItem(QString("%1 - %2").arg(r.from).arg(r.to));
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