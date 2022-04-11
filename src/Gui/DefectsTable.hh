/*
 * Gui/DefectsTable.hh
 */

#pragma once

#include <QTableWidget>
#include <QHeaderView>

#include "Core/ImageProcessing.hh"



 class DefectsViewTableModel : public QAbstractTableModel
  {
	Q_OBJECT
    std::vector<Defect> defects;

  public:
    explicit DefectsViewTableModel(const std::vector<Defect>& defects, QObject* parent = 0);
    virtual int columnCount(const QModelIndex& = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex& = QModelIndex()) const override;
    virtual Qt::ItemFlags flags(const QModelIndex&) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	Q_SIGNAL void defectSelected(size_t);
 };
