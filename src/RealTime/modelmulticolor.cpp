#include "modelmulticolor.h"
#include <QColor>

ModelMulticolor::ModelMulticolor(QList<MulticolorItem> *listMulticolor)
    :m_listMulticolor(nullptr),m_listMulticolorOriginal(listMulticolor)
{
    if(!m_listMulticolorOriginal)
        return;
    m_listMulticolor = new QList<MulticolorItem>;
    foreach(auto multicolor,*m_listMulticolorOriginal){
        m_listMulticolor->push_front(multicolor);
    }
    m_headerList << tr("Value") << tr("Color");

}

ModelMulticolor::~ModelMulticolor(){
    if(m_listMulticolor){delete m_listMulticolor;m_listMulticolor = nullptr;}
}


QVariant ModelMulticolor::data(const QModelIndex &index, int role )const{

    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_listMulticolor->size())
        return QVariant();
    int column = index.column();
    if (role == Qt::DisplayRole){
    switch (column){
        case 0:
           return m_listMulticolor->at(index.row()).bound;
        case 1:
            return m_listMulticolor->at(index.row()).value;

        default:
            return QVariant();
      }
    }
    else if(role == Qt::ForegroundRole){
           return QVariant();//this->m_items->at(index.row())->color();
    }
    else if(role == Qt::FontRole){
           return QVariant();//QFont("Times", 12, QFont::Bold);
    }
    else
        return QVariant();
}

int ModelMulticolor::rowCount(const QModelIndex &parent)const{
    Q_UNUSED(parent)
    return m_listMulticolor->size();
}

int ModelMulticolor::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 2;
}

QVariant ModelMulticolor::headerData(int section, Qt::Orientation orientation, int role) const {

    if (role != Qt::DisplayRole)
       return QVariant();
    if (orientation == Qt::Horizontal && section < m_headerList.size()){
        return m_headerList[section];
    }
    else if(orientation == Qt::Vertical)
        return section;
    return QVariant();
}

Qt::ItemFlags ModelMulticolor::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    if (index.row() >= m_listMulticolor->size())
        return Qt::NoItemFlags;
    int column = index.column();
    switch (column){
        case 0:{
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
        }
        case 1:{
           return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
        }
        default:
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }
}

bool ModelMulticolor::setData(const QModelIndex &index, const QVariant &value, int role){
    if (!index.isValid())
        return Qt::NoItemFlags;
    if (index.row() >= m_listMulticolor->size())
        return Qt::NoItemFlags;
    int column = index.column();
    if (role == Qt::EditRole){
        switch (column){
            case 0:{
               m_listMulticolor->operator[](index.row()).bound = value.toDouble();
               return true;
            }
            case 1:{
               m_listMulticolor->operator[](index.row()).value = value.toString();
               return true;
            }
            default:
                return false;
        }
    }
    else
        return false;
}

void ModelMulticolor::apply(){
    if(!m_listMulticolorOriginal || !m_listMulticolor)
        return;
    m_listMulticolorOriginal->clear();
    foreach(auto multicolor,*m_listMulticolor){
        m_listMulticolorOriginal->push_front(multicolor);
    }
}

void ModelMulticolor::insertColor(){
    if(m_listMulticolor){
       beginInsertRows(QModelIndex(),m_listMulticolor->size(), m_listMulticolor->size());
       m_listMulticolor->push_back(MulticolorItem{m_listMulticolor->last().bound,m_listMulticolor->last().value});
       endInsertRows();
    }
}

void ModelMulticolor::removeColor(){
    if(m_listMulticolor->size() <= 3)
        return;
    if(m_listMulticolor){
       beginRemoveRows(QModelIndex(),m_listMulticolor->size() - 1, m_listMulticolor->size() - 1);
       m_listMulticolor->removeAt(m_listMulticolor->size() - 2);
       endRemoveRows();
    }
}

void ModelMulticolor::calculateHSV(){
    if(!m_listMulticolor)
        return;
    if(m_listMulticolor->size() < 2)
        return;
    if(m_listMulticolor->first().bound < m_listMulticolor->last().bound){
        qreal f_bound = m_listMulticolor->first().bound;
        m_listMulticolor->first().bound = m_listMulticolor->last().bound;
        m_listMulticolor->last().bound = f_bound;
    }
    m_listMulticolor->last().value = QColor(Qt::white).name();
    m_listMulticolor->first().value = QColor(Qt::red).name();

    qreal f_minimum = m_listMulticolor->first().bound;
    qreal f_maximum = m_listMulticolor->last().bound;

    qreal f_hueStep = 360 / qreal(m_listMulticolor->size());
    qreal f_step = (f_maximum - f_minimum) / qreal(m_listMulticolor->size() - 1);

    for(int i = 1; i < m_listMulticolor->size() - 1; ++i){
        m_listMulticolor->operator[](i).bound = (f_minimum + (f_step * qreal(i)));
        QColor f_color;
        f_color.setHsv((f_hueStep * qreal(i)),255,255);
        m_listMulticolor->operator[](i).value = f_color.name();
    }
    emit dataChanged(index(0,0),index(m_listMulticolor->size(),2));
}

void ModelMulticolor::calculateRainbow(){
    if(!m_listMulticolor)
        return;
    if(m_listMulticolor->size() < 2)
        return;
    qreal f_minimumValue = m_listMulticolor->last().bound;
    qreal f_maximumValue = m_listMulticolor->first().bound;
    if(f_minimumValue > f_maximumValue){
        qreal min = f_maximumValue;
        f_maximumValue = f_minimumValue;
        f_minimumValue = min;
    }
    beginResetModel();
    m_listMulticolor->clear();
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(Qt::white).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(142,0,255,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(0,0,255,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(24,127,255,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(0,127,0,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(255,255,0,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_minimumValue,QColor(255,151,0,0).name()});
    m_listMulticolor->push_front(MulticolorItem{f_maximumValue,QColor(Qt::red).name()});

    if(m_listMulticolor->first().bound < m_listMulticolor->last().bound){
        qreal f_bound = m_listMulticolor->first().bound;
        m_listMulticolor->first().bound = m_listMulticolor->last().bound;
        m_listMulticolor->last().bound = f_bound;
    }
    qreal f_step = (f_maximumValue - f_minimumValue) / qreal(m_listMulticolor->size() - 1);

    for(int i = 1; i < m_listMulticolor->size() - 1; ++i){
        m_listMulticolor->operator[](i).bound = (f_maximumValue - (f_step * qreal(i)));
    }
    endResetModel();
    emit dataChanged(index(0,0),index(m_listMulticolor->size(),2));

}
