#pragma once

/**
 * @file log_view.h
 * @brief 操作日志只读视图：分级着色、条数上限、与外部 LogEntry 增量同步
 */

#include "logView/log_entry.h"
#include "component_global.h"

#include <QPlainTextEdit>
#include <QVector>

namespace ui {

/**
 * @brief 首页操作日志 QPlainTextEdit
 *
 * 新日志始终在顶部插入；满额后淘汰最末行。滚动条保持在最新可见处（顶部）。
 * 行颜色由 log_view_style.h 的 logLevelColor 映射 ERROR/WARN/其它。
 */
class UI_COMPONENT_EXPORT LogView : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit LogView(QWidget *parent = nullptr);

    void setMaxEntries(int maxEntries);
    int  maxEntries() const { return m_maxEntries; }

    /** 与外部日志列表增量同步：仅追加新增条目 */
    void syncEntries(const QVector<LogEntry> &entries);

    void appendEntry(const LogEntry &entry);
    void appendEntry(const QDateTime &time, const QString &level, const QString &message);
    void clearLog();

private:
    void bindTheme();
    void refreshStyle();
    void pushEntry(const QDateTime &time, const QString &level, const QString &message);
    void insertEntryAtTop(const QString &line, const QColor &color);
    void removeLastEntry();
    QString formatLine(const QDateTime &time, const QString &level, const QString &message) const;
    QColor  levelColor(const QString &level) const;

    int m_maxEntries  = 500;
    int m_lineCount   = 0;
    int m_syncedCount = 0;
};

} // namespace ui
