#pragma once

/**
 * @file image_preview_style.h
 * @brief ImagePreview 视图的 QSS 样式生成
 *
 * 目标控件：QGraphicsView#imagePreviewView（见 ImagePreview::setObjectName）
 *
 * ThemeTokens 用量：
 *   - inputBackground — 视图背景色（HTML input 区派生色）
 *   - inputBorder     — 1px 圆角边框
 *
 * 无 variant 分支；传入的 ThemeTokens 通常来自 ThemeTokens::resolve()。
 */

#include "theme/theme_tokens.h"

#include <QString>

namespace ui {

/**
 * 生成 ImagePreview 专用 stylesheet 片段。
 * @param t 已解析的主题 token；需含有效的 inputBackground / inputBorder
 * @return 适用于 setStyleSheet 的 QSS 字符串
 */
inline QString buildImagePreviewStylesheet(const ThemeTokens &t)
{
    return QStringLiteral(
        "QGraphicsView#imagePreviewView {"
        "  background-color: %1; border: 1px solid %2; border-radius: 4px;"
        "}"
    )
        .arg(t.inputBackground.name())
        .arg(t.inputBorder.name());
}

} // namespace ui
