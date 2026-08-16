#pragma once

/**
 * @file log_view_style.h
 * @brief LogView 容器 QSS 与日志级别颜色 token（ThemeTokens 映射）
 */

#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/**
 * @brief operationLogView 容器样式
 * @param t 当前主题 token；使用 logBackground / textPrimary / inputBorder
 */
inline QString buildLogViewStylesheet(const ThemeTokens &t)
{
    return QStringLiteral(
        "QPlainTextEdit#operationLogView {"
        "  background-color: %1; color: %2; border: 1px solid %3;"
        "  border-radius: 4px; font-family: Consolas, monospace; font-size: 12px;"
        "  padding: 6px;"
        "}"
    )
        .arg(t.logBackground.name())
        .arg(t.textPrimary.name())
        .arg(t.inputBorder.name());
}

/**
 * @brief 按级别返回行前景色
 * @param level 不区分大小写；ERROR→dangerText，WARN/WARNING→warningText，其它→successText
 */
inline QColor logLevelColor(const ThemeTokens &t, const QString &level)
{
    const QString upper = level.trimmed().toUpper();
    if (upper == QStringLiteral("ERROR"))
        return t.dangerText;
    if (upper == QStringLiteral("WARN") || upper == QStringLiteral("WARNING"))
        return t.warningText;
    return t.successText;
}

} // namespace ui
