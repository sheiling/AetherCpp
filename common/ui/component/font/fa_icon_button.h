#pragma once

/**
 * @file fa_icon_button.h
 * @brief Font Awesome 图标与 ThemedIconButton 桥接工厂
 */

#include "controls/button/themed_icon_button.h"
#include "controls/spinbox/spinbox_paint_utils.h"
#include "font/font_awesome.h"

class QWidget;

namespace ui {

UI_COMPONENT_EXPORT FaIcon faIconForTriangleDirection(TriangleDirection dir);

UI_COMPONENT_EXPORT void applyFaIcon(ThemedIconButton *btn, FaIcon icon, int pixelSize = 14);
UI_COMPONENT_EXPORT void applyFaToggleIcons(ThemedIconButton *btn, FaIcon offIcon, FaIcon onIcon,
                                            int pixelSize = 14);

UI_COMPONENT_EXPORT ThemedIconButton *makeFaIconButton(FaIcon icon, QWidget *parent,
                                                        const QString &tooltipKey = QString());

UI_COMPONENT_EXPORT ThemedIconButton *makeFaIconButton(TriangleDirection dir, QWidget *parent,
                                                         const QString &tooltipKey = QString());

} // namespace ui
