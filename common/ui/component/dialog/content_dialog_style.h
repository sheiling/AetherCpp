#pragma once

/**
 * @file content_dialog_style.h
 * @brief 通用内容弹窗壳 QSS（ThemeTokens，随 ACSS 刷新）
 *
 * 供 ThemedPanelDialog 及同类无边框弹窗使用。占位符仅用 %1–%10，链式 arg() 替换，
 * 避免 %15 被 QString::arg 解析为 %1 后接字面量「5」。
 *
 * objectName 选择器（均以 rootSelector 为前缀）：
 * - themedPanelFrame          — 圆角面板外框（surface / surfaceBorder）
 * - themedPanelTitleBar       — 固定 36px 标题栏行
 * - themedPanelTitle          — 标题 QLabel（14px 粗体，textPrimary）
 * - themedPanelTitleCloseBtn  — 右上角关闭按钮（textSecondary，hover → textPrimary）
 * - themedFormLabel           — 表单字段标签（11px，textSecondary）
 * - themedPanelMessageLabel   — 消息正文（13px，textPrimary）
 * - themedPanelHintLabel      — 辅助说明（11px，textSecondary）
 * - themedPanelDangerBtn      — 危险操作按钮（dangerBg / dangerText / dangerBorder）
 *
 * ThemeTokens 占位映射（buildContentDialogStylesheet）：
 * - %1  rootSelector（#<rootObjectName>）
 * - %2  themeFontFamilyCss()
 * - %3  textPrimary
 * - %4  textSecondary
 * - %5  dangerBg
 * - %6  dangerText
 * - %7  dangerBorder
 * - %8  dangerBgHover
 * - %9  surface
 * - %10 surfaceBorder
 *
 * 末尾拼接 buildComboBoxStylesheet(rootSelector)，使弹窗内下拉框与主题一致。
 */

#include "controls/combo/combo_box_style.h"
#include "theme/theme_font.h"
#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/**
 * 构建内容弹窗根控件及其子树的完整 QSS。
 * @param rootObjectName 对话框根 objectName（如 themedPanelDialog），用作 # 选择器前缀
 * @return 可传给 QWidget::setStyleSheet 的样式表字符串
 */
inline QString buildContentDialogStylesheet(const QString &rootObjectName)
{
    const ThemeTokens t            = ThemeTokens::resolve();
    const QString     rootSelector = QStringLiteral("#") + rootObjectName;
    const QString     fontFamily   = themeFontFamilyCss();

    return QStringLiteral(
        "%1, %1 * { font-family: %2; }"
        "%1 { background: transparent; color: %3; }"
        "%1 QLabel { color: %3; background: transparent; font-family: %2; font-size: 12px; }"
        "%1 QLabel#themedPanelTitle { font-weight: bold; font-family: %2; font-size: 14px; }"
        "%1 QLabel#themedFormLabel { color: %4; font-family: %2; font-size: 11px; }"
        "%1 QLabel#themedPanelMessageLabel { color: %3; font-family: %2; font-size: 13px; }"
        "%1 QLabel#themedPanelHintLabel { color: %4; font-family: %2; font-size: 11px; }"
        "%1 QWidget#themedPanelTitleBar { background: transparent; min-height: 36px; max-height: 36px; }"
        "%1 QPushButton#themedPanelTitleCloseBtn {"
        "  min-width: 24px; max-width: 24px; min-height: 24px; max-height: 24px; padding: 0;"
        "  background: transparent; color: %4; border: none; font-family: %2; font-size: 16px;"
        "}"
        "%1 QPushButton#themedPanelTitleCloseBtn:hover { color: %3; }"
        "%1 QPushButton#themedPanelDangerBtn {"
        "  min-height: 28px; min-width: 64px; padding: 2px 12px; border-radius: 4px;"
        "  background-color: %5; color: %6; border: 1px solid %7;"
        "}"
        "%1 QPushButton#themedPanelDangerBtn:hover:enabled { background-color: %8; }"
        "%1 QFrame#themedPanelFrame {"
        "  background-color: %9; border: 1px solid %10; border-radius: 8px;"
        "}"
    ).arg(rootSelector)
         .arg(fontFamily)
         .arg(t.textPrimary.name())
         .arg(t.textSecondary.name())
         .arg(t.dangerBg.name())
         .arg(t.dangerText.name())
         .arg(t.dangerBorder.name())
         .arg(t.dangerBgHover.name())
         .arg(t.surface.name())
         .arg(t.surfaceBorder.name())
         + buildComboBoxStylesheet(rootSelector);
}

} // namespace ui
