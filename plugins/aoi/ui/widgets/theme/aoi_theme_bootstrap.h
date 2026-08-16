#pragma once

/**
 * @file aoi_theme_bootstrap.h
 * @brief AOI 插件加载时经 TM 硬编码覆盖七原色与语义色，并刷新全局 QSS
 *
 * 暗色主题：写入设计稿（ui_aoi/styles/main.css）七原色 + success/warning/danger。
 * 亮色主题：仅写入语义色，七原色保留 ACSS light_*.xml 默认值，以便亮/暗切换可见。
 *
 * 亮/暗切换时须在 ACSS setCurrentTheme 之后、updateStylesheet 之前
 * 通过 ThemeManager::registerBeforeStylesheetHook 再次注入（setCurrentTheme 会清空变量表）。
 */

#include "uibase/thememanager.h"

#include <QDebug>
#include <QHash>
#include <QString>

namespace aoi {

/** @param darkTheme 当前 ACSS xml 是否 dark="1"（setCurrentTheme 之后读取） */
inline QHash<QString, QString> aoiThemeVariableOverrides(bool darkTheme)
{
    QHash<QString, QString> vars = {
        {QStringLiteral("success"), QStringLiteral("#33C06B")},
        {QStringLiteral("warning"), QStringLiteral("#F0C244")},
        {QStringLiteral("danger"), QStringLiteral("#F25F5C")},
    };

    if (!darkTheme)
        return vars;

    vars.insert(QStringLiteral("secondaryDarkColor"), QStringLiteral("#0F1A2C"));
    vars.insert(QStringLiteral("secondaryColor"), QStringLiteral("#1E2A3A"));
    vars.insert(QStringLiteral("secondaryLightColor"), QStringLiteral("#32445D"));
    vars.insert(QStringLiteral("primaryTextColor"), QStringLiteral("#E6EDF7"));
    vars.insert(QStringLiteral("secondaryTextColor"), QStringLiteral("#9FB0C7"));
    vars.insert(QStringLiteral("primaryLightColor"), QStringLiteral("#0078D7"));
    vars.insert(QStringLiteral("primaryColor"), QStringLiteral("#005A9E"));
    return vars;
}

/** 仅写入变量，不刷新 QSS（供 registerBeforeStylesheetHook 使用） */
inline void applyAoiGlobalThemeVariables()
{
    if (!TM || !TM->isInitialized()) {
        qWarning() << "[AoiTheme] ThemeManager not initialized, skip AOI theme variables.";
        return;
    }
    const bool dark = TM->isCurrentAcssThemeDark();
    TM->setThemeVariableValues(aoiThemeVariableOverrides(dark));
}

/**
 * 写入 AOI 色板覆盖并触发全局样式刷新。
 * 若 TM 未 initialized 则输出 qWarning 并直接返回。
 */
inline void applyAoiGlobalTheme()
{
    if (!TM || !TM->isInitialized()) {
        qWarning() << "[AoiTheme] ThemeManager not initialized, skip AOI theme bootstrap.";
        return;
    }
    applyAoiGlobalThemeVariables();
    TM->refreshStylesheet();
    qDebug() << "[AoiTheme] Applied AOI color overrides on theme:" << TM->currentAcssTheme()
             << "dark=" << TM->isDark();
}

} // namespace aoi
