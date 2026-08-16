/**
 * @file log_view.cpp
 * @brief LogView 环形缓冲式追加、分级着色与 ThemeTokens 样式
 */

#include "logView/log_view.h"

#include "logView/log_view_style.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QScrollBar>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>

namespace ui {

LogView::LogView(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setObjectName(QStringLiteral("operationLogView"));
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setMaximumBlockCount(0);

    bindTheme();
    refreshStyle();
}

void LogView::setMaxEntries(int maxEntries)
{
    m_maxEntries = qMax(1, maxEntries);
    while (m_lineCount > m_maxEntries)
        removeLastEntry();
}

/** 外部列表变短时清空；否则从 m_syncedCount 起增量 pushEntry */
void LogView::syncEntries(const QVector<LogEntry> &entries)
{
    if (entries.size() < m_syncedCount) {
        clearLog();
    }

    for (int i = m_syncedCount; i < entries.size(); ++i)
        pushEntry(entries[i].time, entries[i].level, entries[i].message);

    m_syncedCount = entries.size();
}

void LogView::appendEntry(const LogEntry &entry)
{
    pushEntry(entry.time, entry.level, entry.message);
}

void LogView::appendEntry(const QDateTime &time, const QString &level, const QString &message)
{
    pushEntry(time, level, message);
}

/** 始终在顶部插入最新行；满额时淘汰最末行 */
void LogView::pushEntry(const QDateTime &time, const QString &level, const QString &message)
{
    const QString line  = formatLine(time, level, message);
    const QColor  color = levelColor(level);

    if (m_lineCount >= m_maxEntries)
        removeLastEntry();

    insertEntryAtTop(line, color);
}

void LogView::insertEntryAtTop(const QString &line, const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color);

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    cursor.insertText(line, fmt);
    cursor.insertBlock();

    ++m_lineCount;

    if (verticalScrollBar())
        verticalScrollBar()->setValue(verticalScrollBar()->minimum());
}

void LogView::removeLastEntry()
{
    if (m_lineCount <= 0 || document()->isEmpty())
        return;

    QTextCursor cursor(document()->lastBlock());
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
    if (document()->firstBlock().isValid())
        cursor.deleteChar();

    m_lineCount = qMax(0, m_lineCount - 1);
}

QString LogView::formatLine(const QDateTime &time, const QString &level, const QString &message) const
{
    return QStringLiteral("[%1] %2  %3")
        .arg(time.toString(QStringLiteral("HH:mm:ss")), level.toUpper(), message);
}

QColor LogView::levelColor(const QString &level) const
{
    return logLevelColor(ThemeTokens::resolve(), level);
}

void LogView::clearLog()
{
    clear();
    m_lineCount   = 0;
    m_syncedCount = 0;
}

void LogView::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void LogView::refreshStyle()
{
    setStyleSheet(buildLogViewStylesheet(ThemeTokens::resolve()));
}

} // namespace ui
