#pragma once

/**
 * @file themed_message_box.h
 * @brief 主题确认/提示框：无边框 ThemedPanelDialog 壳，静态调用
 *
 * 替代 QMessageBox 的轻量封装，统一 AOI 主题样式与按钮变体。
 * 图标枚举 ThemedMessageBoxIcon 预留扩展，当前实现未渲染图标（调用方传参被 Q_UNUSED）。
 */

#include "component_global.h"
#include "controls/button/push_button_style.h"

#include <QString>

class QWidget;

namespace ui {

/** 消息框语义图标（预留，尚未在 UI 中绘制） */
enum class ThemedMessageBoxIcon {
    None,        /**< 无图标 */
    Question,    /**< 确认/询问 */
    Warning,     /**< 警告 */
    Information, /**< 信息提示 */
    Critical,    /**< 严重错误 */
};

/**
 * 主题消息框工具类（不可实例化）。
 *
 * 典型用法：
 * - ask()         — 双按钮确认，返回用户是否点击确认
 * - information() — 单按钮提示，阻塞至关闭
 *
 * 弹窗挂载在 parent->window() 上，最小面板宽度 360px。
 */
class UI_COMPONENT_EXPORT ThemedMessageBox
{
public:
    /**
     * 双按钮确认框。
     * @param parent            逻辑父控件，用于定位顶层窗口；可为 nullptr
     * @param title             标题栏文案
     * @param text              正文（自动换行、可选中）
     * @param confirmButtonText 确认按钮文案
     * @param cancelButtonText  取消按钮文案；空字符串则隐藏取消按钮
     * @param icon              语义图标（当前未显示）
     * @param confirmIsDefault  true 时确认钮获焦点，否则取消钮获焦点（有取消钮时）
     * @param confirmVariant    确认钮 PushButtonVariant（默认 Primary）
     * @return true 表示用户点击确认（QDialog::Accepted）
     */
    static bool ask(QWidget *parent,
                    const QString &title,
                    const QString &text,
                    const QString &confirmButtonText,
                    const QString &cancelButtonText = QString(),
                    ThemedMessageBoxIcon icon = ThemedMessageBoxIcon::Question,
                    bool confirmIsDefault = false,
                    PushButtonVariant confirmVariant = PushButtonVariant::Primary);

    /**
     * 单按钮提示框（模态 exec，无返回值）。
     * @param parent        逻辑父控件
     * @param title         标题栏文案
     * @param text          正文
     * @param buttonText    确认钮文案；空则使用 TR("aoi.recipe.add_dialog.confirm")
     * @param icon          语义图标（当前未显示）
     * @param buttonVariant 确认钮变体（默认 Primary）
     */
    static void information(QWidget *parent,
                            const QString &title,
                            const QString &text,
                            const QString &buttonText = QString(),
                            ThemedMessageBoxIcon icon = ThemedMessageBoxIcon::Information,
                            PushButtonVariant buttonVariant = PushButtonVariant::Primary);
};

} // namespace ui
