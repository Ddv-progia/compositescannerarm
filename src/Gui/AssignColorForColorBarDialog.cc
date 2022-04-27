/*
 * Gui/AssignColorForColorBarDialog.cc
 */

#include <iterator>
#include <string>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QColorDialog>
#include <QListWidgetItem>
#include <QtXml>
#include <QTextStream>

#include <regex>
#include "Gui/AssignColorForColorBarDialog.hh"

namespace adp = boost::adaptors;

AssignColorForColorBarDialog::AssignColorForColorBarDialog(const ProcessingParameters& params, bool buttonBoxVisible , QWidget* parent)
  : QDialog(parent), params(params)
{
  ui.setupUi(this);
  connectSignals();
  fillWidgets();
  ui.buttonBox->setVisible(buttonBoxVisible);
}

ProcessingParameters AssignColorForColorBarDialog::getProcessingParameters() const
{
    return params;
}

void AssignColorForColorBarDialog::fillWidgets()
{
    clearSources();
    for (auto item : params.colorStopsList) {
        addColorItem(item.val, item.color.c_str());
    }
    ui.sbxCountOfColors->setValue(params.colorStopsList.size());
}

void AssignColorForColorBarDialog::updateParameters()
{
    params.colorStopsList.clear();
    for (auto i = 0; i < ui.sourcesList->count(); i++) {
        QListWidgetItem* item = ui.sourcesList->item(i);
        WidgetForItem* widget = dynamic_cast<WidgetForItem*>(ui.sourcesList->itemWidget(item));
        widget->pushButton->setFlat(true);
        widget->pushButton->setAutoFillBackground(true);
        auto curveColor = widget->pushButton->palette().color(QPalette::Button);
        ColorStop colorStop;
        colorStop.val = widget->doubleSpinBox->value();
        if (curveColor.isValid()) {
            colorStop.color = (curveColor.name().toStdString());
            params.colorStopsList.push_back(colorStop);
        }
    }
}

void AssignColorForColorBarDialog::GetItemValueAndColor(int i, QColor& curveColor, double& val)
{
    QListWidgetItem* item = ui.sourcesList->item(i);
    WidgetForItem* widget = dynamic_cast<WidgetForItem*>(ui.sourcesList->itemWidget(item));
    widget->pushButton->setFlat(true);
    widget->pushButton->setAutoFillBackground(true);
    curveColor = widget->pushButton->palette().color(QPalette::Button);
    val = widget->doubleSpinBox->value();

    if (curveColor.isValid()) {
    }
    else {
        curveColor = QColor("red");
    }
}

void AssignColorForColorBarDialog::distributeHSV() {
    int colorCount = 0;
    QList<QListWidgetItem*> itemsList;
    qreal f_minimum;
    qreal f_maximum;
    QColor qColorMinimum;
    QColor qColorMaximum;

    GetItemValueAndColor(0, qColorMinimum, f_minimum);
    GetItemValueAndColor(0, qColorMaximum, f_maximum);

    colorCount = ui.sourcesList->selectedItems().size();
    if (colorCount >1) {
        int index = ui.sourcesList->row(ui.sourcesList->selectedItems().at(0));
        GetItemValueAndColor(index, qColorMinimum, f_minimum);
        GetItemValueAndColor(index, qColorMaximum, f_maximum);
        itemsList = QList<QListWidgetItem*>(ui.sourcesList->selectedItems());
        foreach(QListWidgetItem * item, itemsList)
        {
            int index = ui.sourcesList->row(item);
            double value;
            QColor color;
            GetItemValueAndColor(index, color, value);
            if (value < f_minimum) {
                f_minimum = value;
                qColorMinimum = QColor(color);
            }
            if (value > f_maximum) {
                f_maximum = value;
                qColorMaximum = QColor(color);
            }
            delete ui.sourcesList->takeItem(index);
        }
    }
    else {
        colorCount = ui.sourcesList->count();
        itemsList = QList<QListWidgetItem*>(ui.sourcesList->selectedItems());
        if (colorCount < 2)
            return;
        for (auto i = 0; i < colorCount; i++) {
            double val;
            QColor curveColor;
            GetItemValueAndColor(i, curveColor, val);
            if (val < f_minimum) {
                f_minimum = val;
                qColorMinimum = QColor(curveColor);
            }
            if (val > f_maximum) {
                f_maximum = val;
                qColorMaximum = QColor(curveColor);
            }
        }
        clearSources();
    }

    int qColorMaximum_h = 0;
    int qColorMaximum_s = 0;
    int qColorMaximum_v = 0;
    int qColorMinimum_h = 0;
    int qColorMinimum_s = 0;
    int qColorMinimum_v = 0;

    colorCount = ui.sbxCountOfColors->value();

    qColorMaximum.getHsv(&qColorMaximum_h, &qColorMaximum_s, &qColorMaximum_v);
    qColorMinimum.getHsv(&qColorMinimum_h, &qColorMinimum_s, &qColorMinimum_v);
    int f_hueStep = int((qColorMaximum_h - qColorMinimum_h) / (colorCount - 1));
    int f_satStep = int((qColorMaximum_s - qColorMinimum_s) / (colorCount - 1));
    int f_volStep = int((qColorMaximum_v - qColorMinimum_v) / (colorCount - 1));
    qreal f_step = (f_maximum - f_minimum) / qreal(colorCount - 1);
    
    for (int i = 0; i < colorCount; ++i) {
        QColor f_color;
        f_color.setHsv(qColorMinimum_h + f_hueStep * i,
            qColorMinimum_s + f_satStep * i,
            qColorMinimum_v + f_volStep * i);
        addColorItem((f_minimum + (f_step * qreal(i))), f_color.name());
    }
    ui.sbxCountOfColors->setValue(ui.sourcesList->count());
}

void AssignColorForColorBarDialog::saveParamsToXMLFile() {
    QFile xmlFile("paramsColorStopsList.xml");
    if (!xmlFile.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }
    QTextStream xmlContent(&xmlFile);

    QDomDocument document;

    QDomElement element = document.createElement("ColorStopsList");
    document.appendChild(element);

    for (auto colorPair: params.colorStopsList) {
        QDomElement student = document.createElement("Color");
        student.setAttribute("value", colorPair.val);
        student.setAttribute("color", colorPair.color.c_str());
        element.appendChild(student);
    }

}

void AssignColorForColorBarDialog::saveToXMLFile() {
    QFile xmlFile("colorValues.xml");
    if (!xmlFile.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
    }
    QTextStream xmlContent(&xmlFile);

    QDomDocument document;

    QDomElement element = document.createElement("ColorStopsList");
    document.appendChild(element);
    for (auto i = 0; i < ui.sourcesList->count(); i++) {
        QListWidgetItem* item = ui.sourcesList->item(i);
        WidgetForItem* widget = dynamic_cast<WidgetForItem*>(ui.sourcesList->itemWidget(item));
        widget->pushButton->setFlat(true);
        widget->pushButton->setAutoFillBackground(true);
        auto value = widget->doubleSpinBox->value();
        auto color = widget->pushButton->palette().color(QPalette::Button);
        QDomElement subElement = document.createElement("Color");
        subElement.setAttribute("value", value);
        subElement.setAttribute("color", color.name());
        element.appendChild(subElement);
    }
    xmlContent << document.toString();
}

void AssignColorForColorBarDialog::loadFromXMLFile() {
    QDomDocument documentXML;
    QFile xmlFile("colorValues.xml");
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
        return;
    }
    documentXML.setContent(&xmlFile);
    xmlFile.close();

    QDomElement element = documentXML.documentElement();
    QDomElement node = element.firstChild().toElement();

    QString datas = "";
    if (!node.isNull()) {
        clearSources();
    }
    while (node.isNull() == false)
    {
        qDebug() << node.tagName();
        if (node.tagName() == "Color") {
            while (!node.isNull()) {
                QString valueStr = node.attribute("value", "0");
                QString color = node.attribute("color", "white");
                addColorItem(std::stod(valueStr.toStdString()), QColor(color) );
                node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }
}

void AssignColorForColorBarDialog::loadParamsFromXMLFile() {
    QDomDocument documentXML;
    QFile xmlFile("paramsColorStopsList.xml");
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Already opened or there is another issue";
        xmlFile.close();
        return;
    }
    documentXML.setContent(&xmlFile);
    xmlFile.close();

    QDomElement element = documentXML.documentElement();
    QDomElement node = element.firstChild().toElement();

    QString datas = "";
    if (!node.isNull()) {
        params.colorStopsList.clear();
    }
    while (node.isNull() == false)
    {
        qDebug() << node.tagName();
        if (node.tagName() == "Color") {
            while (!node.isNull()) {
                QString valueStr = node.attribute("value", "0");
                QString color = node.attribute("color", "white");
                ColorStop colorStop;
                colorStop.val = std::stod(valueStr.toStdString());
                colorStop.color = color.toStdString();
                 params.colorStopsList.push_back(colorStop);
                node = node.nextSibling().toElement();
            }
        }
        node = node.nextSibling().toElement();
    }
    fillWidgets();
}

void AssignColorForColorBarDialog::connectSignals()
{
  connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addColorItem()));
  connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSource()));
  connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(clearSources()));
  connect(ui.readFromParametersButton, SIGNAL(clicked()), this, SLOT(fillWidgets()));
  connect(ui.distributeColorsButton, SIGNAL(clicked()), this, SLOT(distributeHSV()));
  connect(ui.saveToXMLButton, SIGNAL(clicked()), this, SLOT(saveToXMLFile()));
  connect(ui.loadFromXMLButton, SIGNAL(clicked()), this, SLOT(loadFromXMLFile()));

  connect(ui.moveTopButton, SIGNAL(clicked()), this, SLOT(moveTop()));
  connect(ui.moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(ui.moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));
  connect(ui.moveBottomButton, SIGNAL(clicked()), this, SLOT(moveBottom()));
  connect(this, SIGNAL(accepted()), this, SLOT(updateParameters()));
  if (auto parentQDialog = qobject_cast<QDialog*>(parent())) {
      connect(parentQDialog, SIGNAL(accepted()), this, SLOT(updateParameters()));
  }

}

void AssignColorForColorBarDialog::addColorItem(double value, QColor color)
{
    WidgetForItem* twoButtonWidget = new WidgetForItem(value, color);

    QListWidgetItem* ListItem = new QListWidgetItem();
    ListItem->setSizeHint(twoButtonWidget->minimumSizeHint());

    ui.sourcesList->addItem(ListItem);
    ui.sourcesList->setItemWidget(ListItem, twoButtonWidget);
}

void AssignColorForColorBarDialog::removeSource()
{
  qDeleteAll(ui.sourcesList->selectedItems());
}

void AssignColorForColorBarDialog::clearSources()
{
  ui.sourcesList->clear();
}

void AssignColorForColorBarDialog::moveTop()
{
  //std::vector<std::pair<int, QString>> selection;
  //boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selection),
  //  [this] (QListWidgetItem* i) { return std::make_pair(this->ui.sourcesList->row(i), i->text()); });
  //
  //boost::sort(selection, [] (const std::pair<int, QString>& x, const std::pair<int, QString>& y) {
  //  return x.first < y.first;
  //});
  //
  //boost::for_each(selection | adp::reversed, 
  //  [this] (const std::pair<int, QString>& x) { delete this->ui.sourcesList->takeItem(x.first); });

  //boost::for_each(selection | adp::reversed, 
  //  [this] (const std::pair<int, QString>& x) { this->ui.sourcesList->insertItem(0, x.second); });
}

void AssignColorForColorBarDialog::moveUp()
{
  //std::vector<int> selectedRows;
  //boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selectedRows),
  //  [this] (QListWidgetItem* item) { return this->ui.sourcesList->row(item); });

  //if (selectedRows.empty()) return;
  //    //int firstRow = *boost::min_element(selectedRows);
  //    //int lastRow = *boost::max_element(selectedRows);
  //    //auto precItem = ui.sourcesList->takeItem(firstRow - 1);
  //    //if (precItem) ui.sourcesList->insertItem(lastRow, precItem);

  //std::sort(begin(selectedRows), end(selectedRows)); // sorts by pointer value
  ////boost::sort(selectedRows);
  //for (auto i = 0; i < selectedRows.size();i++) {
  //    //int firstRow = *boost::min_element(selectedRows);
  //    int firstRow = selectedRows[i];

  //    double value;
  //    QColor color;
  //    GetItemValueAndColor(firstRow, color, value);

  //    WidgetForItem* twoButtonWidget = new WidgetForItem(value, color);
  //    QListWidgetItem* ListItem = new QListWidgetItem();
  //    ListItem->setSizeHint(twoButtonWidget->minimumSizeHint());

  //    QListWidgetItem* precItem = ui.sourcesList->takeItem(firstRow);
  //    delete precItem;
  //    if (firstRow>0) ui.sourcesList->insertItem(firstRow, ListItem);
  //    ui.sourcesList->setItemWidget(ListItem, twoButtonWidget);
  //}
}

void AssignColorForColorBarDialog::moveDown()
{
  //std::vector<int> selectedRows;
  //boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selectedRows),
  //  [this] (QListWidgetItem* i) { return this->ui.sourcesList->row(i); });

  //if (selectedRows.empty()) return;

  //int firstRow = *boost::min_element(selectedRows);
  //int lastRow = *boost::max_element(selectedRows);

  //auto succItem = ui.sourcesList->takeItem(lastRow + 1);
  //if (succItem) ui.sourcesList->insertItem(firstRow, succItem);
}

void AssignColorForColorBarDialog::moveBottom()
{
  //std::vector<std::pair<int, QString>> selection;
  //boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selection),
  //  [this] (QListWidgetItem* i) { return std::make_pair(this->ui.sourcesList->row(i), i->text()); });
  //
  //boost::sort(selection, [] (const std::pair<int, QString>& x, const std::pair<int, QString>& y) {
  //  return x.first < y.first;
  //});
  //
  //boost::for_each(selection | adp::reversed, 
  //  [this] (const std::pair<int, QString>& x) { delete this->ui.sourcesList->takeItem(x.first); });

  //boost::for_each(selection, 
  //  [this] (const std::pair<int, QString>& x) { this->ui.sourcesList->addItem(x.second); });
}

WidgetForItem::WidgetForItem(double value, QColor color)
{
    QHBoxLayout* HLay = new QHBoxLayout();

    QDoubleSpinBox* b1 = new QDoubleSpinBox();
    b1->setMaximum(1000000);
    b1->setMinimum(-1000000);
    b1->setDecimals(3);
    b1->setSingleStep(0.1);
    b1->setValue(value);
    b1->setMaximumWidth(100);
    b1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QPushButton* b2 = new QPushButton(" ");
    if (!color.isValid()) {
        color = QColor("red");
    };
    QString qss = QString("background-color: %1").arg(color.name());
    b2->setStyleSheet(qss);
    this->colorLocal = new QColor(color);
    this->valueLocal = new double(value);

    connect(b2, &QPushButton::released, this, [b2, this, color]() {
        QColor colorChoosen = QColorDialog::getColor(color, this);
        if (colorChoosen.isValid()) {
            QString qss = QString("background-color: %1").arg(colorChoosen.name());
            this->colorLocal = new QColor(colorChoosen.name());
            b2->setStyleSheet(qss);
        };
        });

    HLay->addWidget(b1);
    HLay->addWidget(b2);

    this->setLayout(HLay);
    pushButton = b2;
    doubleSpinBox = b1;
}
