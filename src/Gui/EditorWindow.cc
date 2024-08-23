/*
 * Gui/EditorWindow.cc
 */

#include <db_cxx.h>
#include <QtCore/QFile>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include "Core/LoadScanTask.hh"
#include "Gui/EditorWindow.hh"

#include <UCL/Exception.hh>


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

  highlightingRules << HighlightingRule(QRegularExpression ("[-+]?[0-9_]+(\\.[0-9_]+)?"), numberFormat)
                    << HighlightingRule(QRegularExpression ("\\b([a-zA-Z][_a-zA-Z0-9]*)\\b"), identifierFormat)
                    << HighlightingRule(QRegularExpression ("\\bbreak\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\belse\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bnew\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bvar\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bcase\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bfinally\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\breturn\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bvoid\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bcatch\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bfor\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bswitch\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bwhile\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bcontinue\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bfunction\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bthis\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bwith\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bdefault\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bif\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bthrow\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bdelete\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bin\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\btry\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\bdo\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\binstanceof\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("\\btypeof\\b"), keywordFormat)
                    << HighlightingRule(QRegularExpression ("//[^\n]*"), singleLineCommentFormat)
                    ;
}

void ScriptSyntaxHighlighter::highlightBlock(const QString& text)
{
  foreach (const HighlightingRule& rule, highlightingRules) {
    //QRegularExpression expression(rule.pattern);
    //QRegExp expression(rule.pattern);
    //for (int index = expression.indexIn(text); index >= 0;
    //  index = expression.indexIn(text, index + expression.matchedLength()))
    //  
    //    setFormat(index, expression.matchedLength(), rule.format);
  //TODO проверить работу
    QRegularExpression re(rule.pattern);
    auto it = re.globalMatch(text);
    while (it.hasNext()) {
        auto match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
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
void EditorWindow::save(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
{
  save();
}

void EditorWindow::saveAs(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
{
  saveAs();
}

void EditorWindow::load(BackgroundTaskExecutor& taskExecutor, QMdiArea* mdiArea)
{
    auto pathnames = QFileDialog::getOpenFileNames(this, "Открыть", lastOpenDir, "Все файлы сканера (*.js *.csp)");
    if (pathnames.isEmpty()) return;

    bool newPartCreated = false;
    for (auto& pathname : pathnames) {
        QString normalizedSuffix = QFileInfo(pathname).suffix().toLower();
        if (normalizedSuffix == "js") {
            auto ew = new EditorWindow(pathname, this);
            ew->setAttribute(Qt::WA_DeleteOnClose);
            mdiArea->addSubWindow(ew);
            ew->showMaximized();
        }
        else if (normalizedSuffix == "csp") {
            try {
                taskExecutor.enqueue(new LoadScanTask(pathname, *scanFactory, **processingParameters, false));

            }
            catch (DbException& exc) {
                QMessageBox::critical(this, "Ошибка", exc.what());
            }
            catch (uts::Exception& exc) {
                auto msg = boost::get_error_info<uts::ErrInfo_Description>(exc);
                if (msg) {
                    QMessageBox::critical(this, "Ошибка", QString::fromUtf8(msg->c_str()));
                }
                else {
                    QMessageBox::critical(this, "Ошибка", QString::fromLocal8Bit(boost::current_exception_diagnostic_information().c_str()));
                }
            }
        }
    }

    lastOpenDir = QFileInfo(pathnames.back()).dir().path();
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
