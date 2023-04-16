/*
 * Gui/EditorWindow.cc
 */

#include <QtCore/QFile>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include "Gui/EditorWindow.hh"

ScriptSyntaxHighlighter::ScriptSyntaxHighlighter(QTextDocument* parent)
  : QSyntaxHighlighter(parent)
{
  QTextCharFormat keywordFormat;
  keywordFormat.setForeground(Qt::darkBlue);
  keywordFormat.setFontWeight(QFont::Bold);

  QTextCharFormat identifierFormat;
  identifierFormat.setForeground(Qt::black);

  QTextCharFormat singleLineCommentFormat;
  singleLineCommentFormat.setForeground(Qt::darkGray);
  singleLineCommentFormat.setFontItalic(true);

  QTextCharFormat numberFormat;
  numberFormat.setForeground(Qt::darkMagenta);

  /*highlightingRules << HighlightingRule(QRegExp("[-+]?[0-9_]+(\\.[0-9_]+)?"), numberFormat)
                    << HighlightingRule(QRegExp("\\b([a-zA-Z][_a-zA-Z0-9]*)\\b"), identifierFormat)
                    << HighlightingRule(QRegExp("\\bbreak\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\belse\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bnew\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bvar\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bcase\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bfinally\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\breturn\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bvoid\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bcatch\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bfor\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bswitch\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bwhile\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bcontinue\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bfunction\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bthis\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bwith\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bdefault\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bif\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bthrow\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bdelete\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bin\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\btry\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\bdo\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\binstanceof\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("\\btypeof\\b"), keywordFormat)
                    << HighlightingRule(QRegExp("//[^\n]*"), singleLineCommentFormat)
                    ;*/
}

void ScriptSyntaxHighlighter::highlightBlock(const QString& text)
{
  foreach (const HighlightingRule& rule, highlightingRules) {
    /*QRegExp expression(rule.pattern);
    for (int index = expression.indexIn(text); index >= 0;
      index = expression.indexIn(text, index + expression.matchedLength()))
      setFormat(index, expression.matchedLength(), rule.format);*/
  }
}

EditorWindow::EditorWindow(QWidget* parent)
  : QTextEdit(parent)
{ 
  init();
  setWindowTitle("Новая программа*");
}

EditorWindow::EditorWindow(const QString& pathname, QWidget* parent)
  : QTextEdit(parent), pathname(pathname)
{ 
  init();
  loadFromFile(pathname);
  setWindowTitle(pathname);
}

void EditorWindow::init()
{
  setFont(QFont("Consolas", 11));
  new ScriptSyntaxHighlighter(document());
  connect(this, SIGNAL(textChanged()), this, SLOT(programModified()));
}

void EditorWindow::loadFromFile(const QString& pathname)
{
  if (pathname.isEmpty() || !QFile::exists(pathname)) return;

  QFile sourceFile(pathname);
  if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QMessageBox::critical(this, "Ошибка", QString("Не удалось загрузить файл %1").arg(pathname));
      return;
    }

  setPlainText(sourceFile.readAll());
  this->pathname = pathname;
}

QString EditorWindow::associatedPathname() const
{
  return pathname;
}

QString EditorWindow::scriptCode() const
{
  return toPlainText();
}

void EditorWindow::save()
{
  if (pathname.isEmpty()) {
    saveAs();
  } else {
    saveToFile(pathname);
  }
}

void EditorWindow::saveAs()
{
  auto newPathname = QFileDialog::getSaveFileName(this, "Сохранить программу в файл", QString(), "Скрипты (*.js)");
  if (!newPathname.isEmpty()) {
    return saveToFile(newPathname);
  }
}
void EditorWindow::save(BackgroundTaskExecutor& te)
{
  save();
}

void EditorWindow::saveAs(BackgroundTaskExecutor&)
{
  saveAs();
}

void EditorWindow::saveToFile(const QString& newPathname)
{
  QFile targetFile(newPathname);
  if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::critical(this, "Ошибка", QString("Не удалось сохранить программу в файл %1").arg(pathname));
    return;
  }

  targetFile.write(toPlainText().toUtf8());
  pathname = newPathname;
  saved = true;
  setWindowTitle(pathname);
}

void EditorWindow::highlightLine(int lineNumber)
{
  QTextEdit::ExtraSelection lineSelection;
  lineSelection.cursor = textCursor();
  lineSelection.cursor.movePosition(QTextCursor::Start);
  lineSelection.cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineNumber - 1);
  lineSelection.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
  lineSelection.format.setBackground(Qt::yellow);

  QList<QTextEdit::ExtraSelection> selections;
  selections << lineSelection;
  setExtraSelections(selections);
}

void EditorWindow::unhighlightLine()
{
  setExtraSelections(QList<QTextEdit::ExtraSelection>());
}

void EditorWindow::closeEvent(QCloseEvent* evt)
{
  if (!saved) {
    auto answer = QMessageBox::question(this, "Сохранить программу?", "Программа не была сохранена. Сохранить изменения?",
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    switch (answer) {
    case QMessageBox::Yes:
      save();
    case QMessageBox::No:
      evt->accept();
      break;
    case QMessageBox::Cancel:
      evt->ignore();
      break;
    }
  }
}

void EditorWindow::programModified()
{
  setWindowTitle(pathname + "*");
  saved = false;
}
