/*
 * Gui/EditorWindow.hh
 */

#pragma once

#include <QtCore/QString>
#include <QtGui/QSyntaxHighlighter>
#include <QMdiArea>
#include <QtWidgets/QTextEdit>
#include <qregularexpression.h>

#include "Core/PersistentVariable.hh"
#include "Core/ScanFactory.hh"
#include "Gui/Saveable.hh"
#include "Gui/Loadable.hh"

class ScriptSyntaxHighlighter : public QSyntaxHighlighter
{
  struct HighlightingRule
  {
    QRegularExpression pattern;
    QTextCharFormat format;

    HighlightingRule() { }

    HighlightingRule(const QRegularExpression& pattern, const QTextCharFormat& format)
       : pattern(pattern), format(format)
    { }
  };

  QVector<HighlightingRule> highlightingRules;
  
public:
  explicit ScriptSyntaxHighlighter(QTextDocument* parent = 0);

protected:
  void highlightBlock(const QString& text);
};

class EditorWindow : public QTextEdit, public Saveable, public Loadable
{
  Q_OBJECT

public:
  explicit EditorWindow(QWidget* parent = 0);
  explicit EditorWindow(const QString& pathname, QWidget* parent = 0);

  QString associatedPathname() const;
  QString scriptCode() const;

  void save();
  void saveAs();
  Q_SLOT void save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
  Q_SLOT void saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0) override;
  Q_SLOT void load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea = 0)  override ;
  Q_SLOT void highlightLine(int lineNumber);
  Q_SLOT void unhighlightLine();

protected:
  virtual void closeEvent(QCloseEvent* evt) override;

private:
  QString pathname;
  bool saved;

  void init();
  void loadFromFile(const QString& pathname);
  void saveToFile(const QString& newPathname);


  Q_SLOT void programModified();
};
