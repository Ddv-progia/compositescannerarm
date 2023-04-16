#pragma once

#include <QTableView>
#include <Core/ScanData.hh>
#include <QPoint>
#include <QItemDelegate>
#include <QTableView>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/mean.hpp>


#include "Gui/RangeDelegate.hh"
#include "ui_DefectClassificationWidget.h"
#include <exception>


///////Типы операций над значениями


class AbstractToken
{
protected:
	std::vector<std::vector<RangeScanLine>> spec;
	QPoint beginIdx;
	QPoint endIdx;
public:
	AbstractToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx) : spec(spec),beginIdx(beginIdx),endIdx(endIdx){}
	virtual double getValue(FrequencyRange firstRange,FrequencyRange secondRange) = 0;

	static AbstractToken* tokenByNumber(unsigned int N,const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx);
};

class MaximumToken : public AbstractToken
{
	double calculateValue(FrequencyRange range)
	{
		double max = std::numeric_limits<double>::min();

		for(auto row = beginIdx.y(); row <= endIdx.y(); row++){
			for(auto nRange = 0; nRange < spec[row].size(); nRange++){
				if(range.from == spec[row][nRange].range.from){
					for(auto column = beginIdx.x();column<=std::min(static_cast<size_t>(endIdx.x()),spec[row][nRange].samples.size());column++){
						auto value = spec[row][nRange].samples[column];
						if(value > max)
							max = value;
					}
					break;
				}
			}
		}
		return max;
	}
public :
	MaximumToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange) override
	{
		if(firstRange.from > secondRange.from)	std::swap(firstRange,secondRange);
		namespace ba=boost::accumulators;
		ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
		for(auto range = firstRange.from; range < secondRange.to;range+=1000){
      acc(calculateValue(FrequencyRange{ range, range + 1000 }));
		}
		return ba::mean(acc);
	}
};

class MinimumToken : public AbstractToken
{
	double calculateValue(FrequencyRange range)
	{
		double min = std::numeric_limits<double>::max();

		for(auto row = beginIdx.y(); row <= endIdx.y(); row++){
			for(auto nRange = 0; nRange < spec[row].size(); nRange++){
				if(range.from == spec[row][nRange].range.from){
					for(auto column = beginIdx.x();column<=std::min(static_cast<size_t>(endIdx.x()),spec[row][nRange].samples.size());column++){
						auto value = spec[row][nRange].samples[column];
						if(value < min)
							min = value;
					}
					break;
				}
			}
		}

		return min;
	}
public :
	MinimumToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange) override
	{
		if(firstRange.from > secondRange.from)	std::swap(firstRange,secondRange);
		namespace ba=boost::accumulators;
		ba::accumulator_set<double, ba::stats<ba::tag::mean>> acc;
		for(auto range = firstRange.from; range < secondRange.to;range+=1000){
      acc(calculateValue(FrequencyRange{ range, range + 1000 }));
		}
		return ba::mean(acc);
	}
};

class NegativeHalfSumToken :  public AbstractToken
{
public:
	NegativeHalfSumToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange) override
	{
		auto minimumToken = MinimumToken(spec,beginIdx,endIdx);
		auto firstRangeValue = minimumToken.getValue(firstRange,firstRange);
		auto secondRangeValue = minimumToken.getValue(secondRange,secondRange);

		return (firstRangeValue/2 + secondRangeValue/2);
	}
};

class PositiveHalfSumToken :  public AbstractToken
{
public:
	PositiveHalfSumToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange) override
	{
		auto maximumToken = MaximumToken(spec,beginIdx,endIdx);
		auto firstRangeValue = maximumToken.getValue(firstRange,firstRange);
		auto secondRangeValue = maximumToken.getValue(secondRange,secondRange);

		return (firstRangeValue/2 + secondRangeValue/2);
	}
};

//может выбросить исключение деления на 0
class PositiveRelativeResidualToken :  public AbstractToken
{
public:
	PositiveRelativeResidualToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange)
	{
		auto maximumToken = MaximumToken(spec,beginIdx,endIdx);
		auto firstRangeValue = maximumToken.getValue(firstRange,firstRange);
		auto secondRangeValue = maximumToken.getValue(secondRange,secondRange);

		if(secondRangeValue + firstRangeValue == 0)
			throw std::exception("Zero division exception");
		
		return 0.5*((secondRangeValue - firstRangeValue)/(secondRangeValue + firstRangeValue));
	}
};

//может выбросить исключение деления на 0
class NegativeRelativeResidualToken :  public AbstractToken
{
public:
	NegativeRelativeResidualToken(const std::vector<std::vector<RangeScanLine>>& spec, QPoint beginIdx, QPoint endIdx):AbstractToken(spec,beginIdx,endIdx){}
	double getValue(FrequencyRange firstRange,FrequencyRange secondRange)
	{
		auto minimumToken = MinimumToken(spec,beginIdx,endIdx);
		auto firstRangeValue = minimumToken.getValue(firstRange,firstRange);
		auto secondRangeValue = minimumToken.getValue(secondRange,secondRange);

		if(secondRangeValue + firstRangeValue == 0)
			throw std::exception("Zero division exception");
		
		return 0.5*((secondRangeValue - firstRangeValue)/(secondRangeValue + firstRangeValue));
	}
};

//////////////////
class DefectClassificationTokenDelegate : public QStyledItemDelegate
{
	std::vector<QString> tokens;
  public:
    explicit DefectClassificationTokenDelegate(const std::vector<QString>& tokens,QObject* parent = 0)
      : QStyledItemDelegate(parent),tokens(tokens)
    { }

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
      auto cb = new QComboBox(parent);
	  for(auto& token : tokens)
		  cb->addItem(token);
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

////////////////
class DefectClassificationTableModel: public QAbstractTableModel
{
	Q_OBJECT
	std::vector<DefectType>& defects;
	std::vector<QString> tokens;
	std::vector<FrequencyRange> ranges;
public:
	DefectClassificationTableModel(std::vector<DefectType>& defects,const std::vector<QString>& tokens, const std::vector<FrequencyRange>& ranges) : defects(defects),ranges(ranges),tokens(tokens)
	{

	}

    int columnCount(const QModelIndex&) const 
    {
		//3 столбца под выбор алгоритма и диапазонов, столбец дефекта состоит из значения и коэффициента
		return (3 + defects.size()*2);	
    }

    int rowCount(const QModelIndex&) const 
    {
	  size_t nParams = 0;
	  //число уникальных параметров для дефектов. В простом случае предполагаем, 
	  //что все дефекты характеризует один набор параметров, поэтому выбираем наименьшее число параметров
	  nParams = defects.front().params.size();
	  for(auto& defect : defects){
		  if(defect.params.size()!=nParams)
			  nParams = std::min(nParams,defect.params.size()); 
	  }
      return (2 + nParams);
    }

    Qt::ItemFlags flags(const QModelIndex& index) const
    {
		if(index.row() == 1 || (index.row() == 0 && index.column() == 0))
			return Qt::ItemIsEnabled;
		else
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

	QVariant data(const QModelIndex& index, int role) const
  {
      switch (role) {
      case Qt::DisplayRole:
	    case Qt::EditRole:
		  if(index.row()==0){
			  switch (index.column()) {
			  case 0: return QString("Наименование дефекта:");
			  default:
          if (defects.empty()) return QVariant();
          return index.column() % 2 ? QString::fromStdString(defects[(index.column() - 1) / 2 - 1].name) : QVariant();
			  }
		  }
		  if(index.row()==1)
			  switch (index.column()) {
			  case 0: return QString("Параметр");
		      case 1: return QString("Диапазон 1");
			  case 2: return QString("Диапазон 2");
			  default:
				  return (index.column()%2 ?  QString("Коэф."):QString("Знач."));
			  }
		  else{
        if (defects.empty() || ranges.empty()) return QVariant();
			  switch (index.column()) {
			  case 0: 
				  if(role == Qt::DisplayRole)
					  return tokens[defects[index.column()/2].params[index.row()-2].token];
				  else
					  return defects[index.column()/2].params[index.row()-2].token;
			  case 1:
          if (role == Qt::DisplayRole){
            auto params = defects[0].params[index.row() - 2];
            if (params.ranges.empty()) return QVariant();
            auto range = ranges[params.ranges[0]];
            return QString("%1 - %2").arg(range.from).arg(range.to);
          } else {
            return defects[index.column() / 2].params[index.row() - 2].ranges[0];
          }
			  case 2:
				  if(role == Qt::DisplayRole)
            if (role == Qt::DisplayRole){
              auto params = defects[0].params[index.row() - 2];
              if (params.ranges.empty()) return QVariant();
              auto range = ranges[params.ranges[1]];
              return QString("%1 - %2").arg(range.from).arg(range.to);
            }
            else {
				if (defects[index.column() / 2].params[index.row() - 2].ranges.size() > 1)
                  return defects[index.column() / 2].params[index.row() - 2].ranges[1];
				return QVariant();
            }
        default:
				  return (index.column()%2 ? defects[(index.column()-1)/2-1].params[index.row()-2].koeff : defects[(index.column()-1)/2-1].params[index.row()-2].val);
			  }
		  }
	  }
      return QVariant();
  }

	virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
    {
      switch (role) {
      case Qt::EditRole:
		  if(index.row()==0){
			  if(index.column()<3)
				  return false;
			  defects[(index.column()-1)/2-1].name = value.toString().toStdString();
		  }
		  else{
			switch (index.column()) {
			case 0:
				for(auto& defect : defects)
					defect.params[index.row()-2].token = value.toInt();
				break;
			case 1:
				for(auto& defect : defects)
					if (defect.params[index.row() - 2].ranges.size())
					  defect.params[index.row()-2].ranges[0] = value.toInt();
					else {
						std::vector<unsigned int> ranges(2, 0);
						defect.params[index.row() - 2].ranges = ranges;
						defect.params[index.row() - 2].ranges[0] = value.toInt();
					}
				break;
			case 2:
				for(auto& defect : defects)
					if (defect.params[index.row() - 2].ranges.size() > 1)
					  defect.params[index.row()-2].ranges[1] = value.toInt();
					else {
						std::vector<unsigned int> ranges(2, 0);
						defect.params[index.row() - 2].ranges = ranges;
						defect.params[index.row() - 2].ranges[1] = value.toInt();
					}
				break;
			default:
				index.column()%2 ? defects[(index.column()-1)/2 - 1].params[index.row()-2].koeff = value.toDouble() : 
								   defects[(index.column()-1)/2 - 1].params[index.row()-2].val = value.toDouble();
			
			}
		  }
		emit changed();
		return true;
	  }

      return false;
    }

	Q_SLOT virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginInsertRows(parent, row, row + count - 1);
	  for(auto& defect : defects){
		  std::vector<unsigned int> ranges(2,0);
		  auto defaultParameter = defect.params.front();
		  defaultParameter.token = 0;
		  defaultParameter.koeff = 1;
		  defaultParameter.ranges = ranges;
		  defaultParameter.val = 0.0;

		  defect.params.insert(defect.params.begin() + (row - 2), count, defaultParameter);
	  }
      endInsertRows();
	  emit changed();
      return true;
    }

	Q_SLOT virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override
    {
      beginRemoveRows(parent, row, row + count - 1);
	  for(auto& defect : defects){
		  defect.params.erase(defect.params.begin() + (row - 2), defect.params.begin() + (row - 2 + count));
	  }
      endRemoveRows();
	  emit changed();
      return true;
    }

	Q_SLOT virtual bool insertColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override
    {
		beginInsertColumns(parent, column, column + count);

		DefectType defect = defects.front();
		defect.name = "Новый дефект";
		for(auto& param : defect.params){
		  std::vector<unsigned int> ranges(2,0);
		  param.koeff = 1;
		  param.val = 0.0;
		}

		defects.insert(defects.begin() + (column - 3)/2, count, defect);

		endInsertColumns();
		emit changed();
		return true;
    }

	Q_SLOT virtual bool removeColumns(int column, int count, const QModelIndex& parent = QModelIndex()) override
    {
		beginRemoveColumns(parent, column, column + count-1);

		defects.erase(defects.begin() + (column-3)/2, defects.begin() + (column-3)/2 + count/2);

		endRemoveColumns();

		emit changed();
		return true;
    }

	Q_SLOT void findSelfInfluence()
	{
		std::vector<std::vector<double>> result(defects.size());
		for(auto& influence : result)
			influence.resize(defects.size());

		std::vector<std::vector<double>> normalizeParams;
		for(auto nParam = 0; nParam < defects.back().params.size(); nParam++){
			std::vector<double> params;
			auto min = std::numeric_limits<double>::max();
			auto max = -std::numeric_limits<double>::max();
			for(auto nDefect = 0; nDefect<defects.size(); nDefect++){
				auto& value = defects[nDefect].params[nParam].val;
				min = (value<min ? value : min);
				max = (value>max ? value : max);
			}

			for(auto nDefect = 0; nDefect<defects.size(); nDefect++){
				params.push_back((defects[nDefect].params[nParam].val - min)/(max-min));
			}

			normalizeParams.push_back(params);
		}

		for(auto column = 0; column<defects.size(); column++){
			for(auto row = 0; row<defects.size(); row++){
				auto sum = 0.0;
				for(auto nParam = 0; nParam < normalizeParams.size();nParam++){
					sum+=std::abs(normalizeParams[nParam][column] - normalizeParams[nParam][row]) * defects[column].params[nParam].koeff;
				}
				result[row][column] = sum;
			}
		}
		emit selfInfluence(result);
	}

	Q_SIGNAL void selfInfluence(const std::vector<std::vector<double>>&);
	Q_SIGNAL void changed();
};


class DefectClassificationWidget : public QWidget, public Ui::DefectClassificationWidget
{
	Q_OBJECT
	std::vector<DefectType> defects;
	std::vector<FrequencyRange> ranges;
	//std::vector < ::Extremum> extremums;
	std::vector<QString> tokens;
	DefectClassificationTableModel* model;
public:
	DefectClassificationWidget(const std::shared_ptr<Scan>& scan, QWidget* parent = 0) : defects(scan->parameters.defectClassification), QWidget(parent)
	{
		this->setupUi(this);
		for(auto& range : scan->normalizedSpec.back())
			ranges.push_back(range.range);
		tokens.push_back(QString("Минимум"));
        tokens.push_back(QString("Максимум"));
	    tokens.push_back(QString("Полусумма(+)"));
	    tokens.push_back(QString("Полусумма(-)"));
	    tokens.push_back(QString("Относительная разность(+)"));
	    tokens.push_back(QString("Относительная разность(-)"));

		model = new DefectClassificationTableModel(defects,tokens,ranges);
		this->tableView->setModel(model);
		this->tableView->setItemDelegateForColumn(0,new DefectClassificationTokenDelegate(tokens));
		this->tableView->setItemDelegateForColumn(1,new DefectRangeDelegate(ranges));
		this->tableView->setItemDelegateForColumn(2,new DefectRangeDelegate(ranges));

		this->selfInfluenceTable->hide();
		//объединение ячеек
		this->tableView->setSpan(0,0,1,3);
		for(auto i = 0;i<scan->parameters.defectClassification.size();i++)
			this->tableView->setSpan(0,i*2+3,1,2);

		connect(model,SIGNAL(changed()),model,SLOT(findSelfInfluence()));
		connect(model,SIGNAL(selfInfluence(const std::vector<std::vector<double>>&)),SLOT(refreshSelfInfluenceTable(const std::vector<std::vector<double>>&)));
		connect(buttonBox,SIGNAL(accepted()),SLOT(sendRefresh()));

		model->findSelfInfluence();
	}

	Q_SLOT void addDefect()
	{

		auto selection = tableView->selectionModel();
		tableView->model()->insertColumn(tableView->model()->columnCount());
		tableView->setSpan(0,tableView->model()->columnCount()-2,1,2);
	}

	Q_SLOT void removeDefect() 
	{
		auto selection = tableView->selectionModel();
		if (selection->hasSelection()) {
			auto frontColumn = selection->selectedIndexes().front().column();
			auto backColumn = selection->selectedIndexes().back().column();
			if(frontColumn<3)
				return;
			frontColumn = frontColumn%2 ? frontColumn : frontColumn - 1;
			backColumn = backColumn%2 ? backColumn + 1 : backColumn;

			tableView->setSpan(0,frontColumn,1,1);
			tableView->model()->removeColumns(frontColumn, backColumn - frontColumn + 1);
		}
	}

	Q_SLOT void addToken()
	{
		auto selection = tableView->selectionModel();
		tableView->model()->insertRow(tableView->model()->rowCount());
	}

	Q_SLOT void removeToken()
	{
		auto selection = tableView->selectionModel();
		if (selection->hasSelection()) {
			tableView->model()->removeRows(selection->selectedIndexes().front().row(), 
                                           selection->selectedIndexes().back().row() - selection->selectedIndexes().front().row() + 1);
		}
	}

	Q_SLOT void sendRefresh()
	{
		emit defectClassificationRefreshed(defects);
	}

	Q_SIGNAL void defectClassificationRefreshed(std::vector<DefectType>&);

	Q_SLOT void refreshSelfInfluenceTable(const std::vector<std::vector<double>>& selfInfluence)
	{
		selfInfluenceTable->setColumnCount(selfInfluence.size());
		selfInfluenceTable->setRowCount(selfInfluence.size());
		for(auto row = 0; row < selfInfluence.size(); row++){
			for(auto column = 0; column < selfInfluence[row].size(); column++){
				selfInfluenceTable->setItem(row,column,new QTableWidgetItem(QString::number(selfInfluence[row][column])));
			}
		}
	}
};



class DefectClassificator
{
	const std::vector<DefectType>& patternDefects;
	const std::vector<std::vector<RangeScanLine>>& spec;
public:
	DefectClassificator(const std::vector<DefectType>& patternDefects,const std::vector<std::vector<RangeScanLine>>& spec):patternDefects(patternDefects),spec(spec)
	{}

	std::vector<double> getControlParams(QPoint beginPoint,QPoint endPoint)
	{
		std::vector<FrequencyRange> ranges;
		for(auto& range : spec.back())
			ranges.push_back(range.range);

		std::vector<double> result;
		for(auto param : patternDefects.front().params)
			result.push_back(AbstractToken::tokenByNumber(param.token,spec,beginPoint,endPoint)->getValue(ranges[param.ranges[0]],ranges[param.ranges[1]]));
		return result;
	}

	std::vector<double> findFieldCloseness(QPoint beginPoint,QPoint endPoint)
	{
		std::vector<double> result,values;
		
		values = normalizeControlParams(getControlParams(beginPoint,endPoint));
		auto normalizedParams = normalizeParams(patternDefects);
		auto residuals = getResiduals(normalizedParams,values);

		for(auto nDefect = 0; nDefect<patternDefects.size();nDefect++){
			auto sum = 0.0;
			for(auto nParam = 0; nParam < patternDefects[nDefect].params.size();nParam++)
				sum += residuals[nParam][nDefect];
			result.push_back(sum);
		}
		
		return result;
	}

	std::vector<std::vector<double>> getResiduals(const std::vector<std::vector<double>>& normalizedParams,const std::vector<double>& values)
	{
		std::vector<std::vector<double>> result;

		for(auto nParam = 0; nParam<normalizedParams.size();nParam++){
			std::vector<double> params;
			for(auto nDefect = 0; nDefect < normalizedParams.back().size();nDefect++)
				params.push_back(std::abs(patternDefects[nDefect].params[nParam].koeff * (values[nParam] - normalizedParams[nParam][nDefect])));
			result.push_back(params);
		}
		return result;
	}
	std::vector<double> normalizeControlParams(const std::vector<double>& params)
	{
		std::vector<double> result;
		for(auto nParam = 0;nParam<params.size();nParam++){
			auto max = -std::numeric_limits<double>::max();
			auto min = std::numeric_limits<double>::max();
			for(auto& defect : patternDefects){
				auto value = defect.params[nParam].val;
				max = value>max ? value : max;
				min = value<min ? value : min;
			}
			result.push_back((params[nParam] - min)/(max - min));
		}
		return result;
	}

	std::vector<std::vector<double>> normalizeParams(const std::vector<DefectType>& defects)
	{
		std::vector<std::vector<double>> result;

		for(auto nParam = 0;nParam<defects.back().params.size();nParam++){
			std::vector<double> normal;
			auto max = -std::numeric_limits<double>::max();
			auto min = std::numeric_limits<double>::max();
			for(auto& defect : defects){
				min = defect.params[nParam].val<min ? defect.params[nParam].val : min;
				max = defect.params[nParam].val>max ? defect.params[nParam].val : max;
			}

			for(auto& defect : defects){
				normal.push_back((defect.params[nParam].val - min)/(max-min));
			}
			result.push_back(normal);
		}

		return result;
	}

};