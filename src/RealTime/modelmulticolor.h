#ifndef MODELMULTICOLOR_H
#define MODELMULTICOLOR_H
#include <QAbstractListModel>
#include "aitem.h"

class ModelMulticolor : public QAbstractListModel
{
    Q_OBJECT
    QStringList m_headerList;
    QList<MulticolorItem> *m_listMulticolor;
    QList<MulticolorItem> *m_listMulticolorOriginal;
public:
    ModelMulticolor(QList<MulticolorItem> *listMulticolor);
    ~ModelMulticolor()override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role)override;

    void apply();

public slots:
    void insertColor();
    void removeColor();
    void calculateHSV();
    void calculateRainbow();
};

#endif // MODELMULTICOLOR_H
