#pragma once

/**
 * @file kpi_metric_card_style.h
 * @brief KpiMetricCard 动态 QSS（颜色来自 ThemeTokens）
 *
 * KpiValueRole（通过 QFrame role 属性）与 ThemeTokens 映射：
 * - 卡片外框（所有 role）：surfaceElevated, surfaceBorder
 * - 标题：textSecondary
 * - Normal 数值：textPrimary
 * - Ok（role=\"ok\"）：successText
 * - Ng（role=\"ng\"）：dangerText
 * - Warning（role=\"warning\"）：warningText
 */

#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/** 生成 #kpiMetricCard 及其子 QLabel 选择器 QSS */
inline QString buildKpiMetricCardStylesheet(const ThemeTokens &t)
{
    return QStringLiteral(
        "QFrame#kpiMetricCard {"
        "  background-color: %1; border: 1px solid %2; border-radius: 6px;"
        "}"
        "QFrame#kpiMetricCard QLabel#kpiMetricTitle {"
        "  color: %3; font-size: 12px; background: transparent; border: none;"
        "}"
        "QFrame#kpiMetricCard QLabel#kpiMetricValue {"
        "  color: %4; font-size: 18px; font-weight: 600; background: transparent; border: none;"
        "}"
        "QFrame#kpiMetricCard[role=\"ok\"] QLabel#kpiMetricValue { color: %5; }"
        "QFrame#kpiMetricCard[role=\"ng\"] QLabel#kpiMetricValue { color: %6; }"
        "QFrame#kpiMetricCard[role=\"warning\"] QLabel#kpiMetricValue { color: %7; }"
    )
        .arg(t.surfaceElevated.name())
        .arg(t.surfaceBorder.name())
        .arg(t.textSecondary.name())
        .arg(t.textPrimary.name())
        .arg(t.successText.name())
        .arg(t.dangerText.name())
        .arg(t.warningText.name());
}

} // namespace ui
