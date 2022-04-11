/*
 * Gui/AssembleScanDialog.cc
 */

#include <iterator>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm/min_element.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <QtWidgets/QFileDialog>

#include <regex>
#include "Gui/AssembleScanDialog.hh"

namespace adp = boost::adaptors;

AssembleScanDialog::AssembleScanDialog(QWidget* parent)
  : QDialog(parent)
{
  ui.setupUi(this);
  connectSignals();
}

std::vector<QString> AssembleScanDialog::getFiles()
{
  std::vector<QString> r;
  for (int i = 0; i < ui.sourcesList->count(); ++i)
    r.push_back(ui.sourcesList->item(i)->text());
  return r;
}

void AssembleScanDialog::connectSignals()
{
  connect(ui.addButton, SIGNAL(clicked()), this, SLOT(addSource()));
  connect(ui.removeButton, SIGNAL(clicked()), this, SLOT(removeSource()));
  connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(clearSources()));

  connect(ui.moveTopButton, SIGNAL(clicked()), this, SLOT(moveTop()));
  connect(ui.moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));
  connect(ui.moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));
  connect(ui.moveBottomButton, SIGNAL(clicked()), this, SLOT(moveBottom()));
}

void AssembleScanDialog::addSource()
{
  QString filter;
  if(ui.sourcesList->count()!=0){
    filter = (std::regex_match(ui.sourcesList->item(0)->text().toStdString(),std::regex("(.*)(xml)"))) ? "Файлы сборки (*.xml)" : "Звуковые файлы (*.wav)";
  }else
    filter = "Файлы сборки (*.xml);;Звуковые файлы (*.wav)";
  
  auto files = QFileDialog::getOpenFileNames(this, "Выберите файлы для импорта", "", filter);
  ui.sourcesList->addItems(files);
}

void AssembleScanDialog::removeSource()
{
  std::vector<int> selectedRows;
  boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selectedRows),
    [this] (QListWidgetItem* i) { return this->ui.sourcesList->row(i); });
  boost::sort(selectedRows);
  boost::for_each(selectedRows | adp::reversed, [this] (int i) { delete this->ui.sourcesList->takeItem(i); });
}

void AssembleScanDialog::clearSources()
{
  ui.sourcesList->clear();
}

void AssembleScanDialog::moveTop()
{
  std::vector<std::pair<int, QString>> selection;
  boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selection),
    [this] (QListWidgetItem* i) { return std::make_pair(this->ui.sourcesList->row(i), i->text()); });
  
  boost::sort(selection, [] (const std::pair<int, QString>& x, const std::pair<int, QString>& y) {
    return x.first < y.first;
  });
  
  boost::for_each(selection | adp::reversed, 
    [this] (const std::pair<int, QString>& x) { delete this->ui.sourcesList->takeItem(x.first); });

  boost::for_each(selection | adp::reversed, 
    [this] (const std::pair<int, QString>& x) { this->ui.sourcesList->insertItem(0, x.second); });
}

void AssembleScanDialog::moveUp()
{
  std::vector<int> selectedRows;
  boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selectedRows),
    [this] (QListWidgetItem* i) { return this->ui.sourcesList->row(i); });

  if (selectedRows.empty()) return;

  int firstRow = *boost::min_element(selectedRows);
  int lastRow = *boost::max_element(selectedRows);

  auto precItem = ui.sourcesList->takeItem(firstRow - 1);
  if (precItem) ui.sourcesList->insertItem(lastRow, precItem);
}

void AssembleScanDialog::moveDown()
{
  std::vector<int> selectedRows;
  boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selectedRows),
    [this] (QListWidgetItem* i) { return this->ui.sourcesList->row(i); });

  if (selectedRows.empty()) return;

  int firstRow = *boost::min_element(selectedRows);
  int lastRow = *boost::max_element(selectedRows);

  auto succItem = ui.sourcesList->takeItem(lastRow + 1);
  if (succItem) ui.sourcesList->insertItem(firstRow, succItem);
}

void AssembleScanDialog::moveBottom()
{
  std::vector<std::pair<int, QString>> selection;
  boost::transform(ui.sourcesList->selectedItems(), std::back_inserter(selection),
    [this] (QListWidgetItem* i) { return std::make_pair(this->ui.sourcesList->row(i), i->text()); });
  
  boost::sort(selection, [] (const std::pair<int, QString>& x, const std::pair<int, QString>& y) {
    return x.first < y.first;
  });
  
  boost::for_each(selection | adp::reversed, 
    [this] (const std::pair<int, QString>& x) { delete this->ui.sourcesList->takeItem(x.first); });

  boost::for_each(selection, 
    [this] (const std::pair<int, QString>& x) { this->ui.sourcesList->addItem(x.second); });
}
