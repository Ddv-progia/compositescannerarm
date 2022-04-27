/*
 * Gui/AssignColorForColorBarDialog.hh
 */

#pragma once

#include <vector>
#include <QtCore/QString>
#include <Core\ScanData.hh>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QSplitter>
#include <QColor>
#include <QPushButton>
#include <QComboBox>

#include "ui_AssignColorForColorBarDialog.h"

struct MulticolorItem {
    qreal bound;
    QString value;
};

class WidgetForItem : public QWidget
{
    Q_OBJECT
    double value=0;
public:
    explicit WidgetForItem(double value, QColor color);
    ~WidgetForItem() {}
    double* valueLocal;
    QColor* colorLocal;
    QDoubleSpinBox* doubleSpinBox;
    QPushButton* pushButton;
};

class AssignColorForColorBarDialog: public QDialog
{
  Q_OBJECT
public:
  explicit AssignColorForColorBarDialog(const ProcessingParameters& params, bool buttonBoxVisible = true, QWidget* parent = 0);
  ProcessingParameters getProcessingParameters() const;
  Ui::AssignColorForColorBarDialog ui;
  Q_SLOT void updateParameters();

private:
  ProcessingParameters params;

  void connectSignals();
  Q_SLOT void fillWidgets();
  Q_SLOT void addColorItem(double value = 0, QColor color = "red");
  Q_SLOT void removeSource();
  Q_SLOT void clearSources();
  void GetItemValueAndColor(int i, QColor& curveColor, double& val);
  Q_SLOT void distributeHSV();
  Q_SLOT void saveToXMLFile();
  Q_SLOT void loadFromXMLFile();
  Q_SLOT void saveParamsToXMLFile();
  Q_SLOT void loadParamsFromXMLFile();

  Q_SLOT void moveTop();
  Q_SLOT void moveUp();
  Q_SLOT void moveDown();
  Q_SLOT void moveBottom();
};