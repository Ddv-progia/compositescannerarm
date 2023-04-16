/*
 * Gui/EditorWindow.hh
 */

#pragma once

#include <QtCore/QString>
#include <QtGui/QSyntaxHighlighter>
#include <QtWidgets/QTextEdit>

#include "Gui/Saveable.hh"

class ScriptSyntaxHighlighter : public QSyntaxHighlighter
{
  struct HighlightingRule
  {
    //QRegExp pattern;
    QTextCharFormat format;

    HighlightingRule() { }

    HighlightingRule(/*const QRegExp& pattern,*/ const QTextCharFormat& format)
       : /*pattern(pattern),*/ format(format)
    { }
  };

  QVector<HighlightingRule> highlightingRules;
  
public:
  explicit ScriptSyntaxHighlighter(QTextDocument* parent = 0);

protected:
  void highlightBlock(const QString& text);
};

class EditorWindow : public QTextEdit, public Saveable
{
  Q_OBJECT

public:
  explicit EditorWindow(QWidget* parent = 0);
  explicit EditorWindow(const QString& pathname, QWidget* parent = 0);

  QString associatedPathname() const;
  QString scriptCode() const;

  void save();
  void saveAs();
  Q_SLOT virtual void save(BackgroundTaskExecutor&) override;
  Q_SLOT virtual void saveAs(BackgroundTaskExecutor&) override;
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
