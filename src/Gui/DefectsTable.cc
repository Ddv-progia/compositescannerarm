/*
 * Gui/DefectsTable.cc
 */

#include "Gui/DefectsTable.hh"

    DefectsViewTableModel::DefectsViewTableModel(const std::vector<Defect>& defects, QObject* parent)
      : QAbstractTableModel(parent), defects(defects)
    {
	}

    int DefectsViewTableModel::columnCount(const QModelIndex&) const 
    {
      return 6;
    }

    int DefectsViewTableModel::rowCount(const QModelIndex&) const 
    {
      return defects.size();
    }

    Qt::ItemFlags DefectsViewTableModel::flags(const QModelIndex&) const
    {
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant DefectsViewTableModel::headerData(int section, Qt::Orientation orientation, int role) const 
    {
      switch (role) {
      case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
          switch (section) {
		  case 0: return QString("№");
		  case 1: return QString("Ширина,мм");
		  case 2: return QString("Высота,мм");
		  case 3: return QString("Центр");
		  case 4: return QString("Площадь,мм");
		  case 5: return QString("Тип дефекта");
          }
        }
      }

      return QVariant();
    }

    QVariant DefectsViewTableModel::data(const QModelIndex& index, int role) const
    {
      switch (role) {
      case Qt::DisplayRole:
        switch (index.column()) {
		case 0: return (QString::number(index.row()+1));
		case 1: return QString::number(defects[index.row()].width);
		case 2: return QString::number(defects[index.row()].height);
		case 3: return QString("x:%1,y:%2").arg(defects[index.row()].center.x()).arg(defects[index.row()].center.y());
		case 4: return QString::number(defects[index.row()].area);
		case 5: return (QString("Дефект"));
        }
      }
      return QVariant();
    }
