#pragma once

/**
 * @file log_entry.h
 * @brief 单条操作日志的数据载体，供 LogView 与 ViewModel 传递
 */

#include <QDateTime>
#include <QString>

namespace ui {

/** 时间戳 + 级别 + 正文，level 通常为 INFO/WARN/ERROR */
struct LogEntry
{
    QDateTime time;
    QString   level;
    QString   message;
};

} // namespace ui
