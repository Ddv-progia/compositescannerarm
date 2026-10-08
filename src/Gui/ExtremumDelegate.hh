#pragma once

#include <QStyledItemDelegate>
#include <QComboBox>

#include "Core/ScanData.hh"

class ExtremumDelegate : public QStyledItemDelegate
  {
  public:
    explicit ExtremumDelegate( QObject* parent = 0)
      : QStyledItemDelegate(parent)
    { }

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto cb = new QComboBox(parent);
      cb->addItem("Max");
      cb->addItem("Min");
      cb->addItem("Aver");
      cb->addItem("Diff");
      cb->addItem("DiffOnTable");
      //hand = static_cast<Suit>(account_num);
      return cb;
    }

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        auto dat = index.data().toInt();
      static_cast<QComboBox*>(editor)->setCurrentIndex(dat);
    }

    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
      model->setData(index, static_cast<QComboBox*>(editor)->currentIndex());

    }

  };