#pragma once

/**
 * @file themed_push_button.h
 * @brief 主题 PushButton：Secondary / Primary / Danger 变体，可选投影
 *
 * 样式由 push_button_style.h 生成；主题切换时通过 bindThemeRefresh 自动 refreshStyle。
 * 可选 QGraphicsDropShadowEffect，阴影色取自 ThemeTokens::dropShadow。
 */

#include "controls/button/push_button_style.h"
#include "component_global.h"

#include <QPushButton>

class QEvent;
class QGraphicsDropShadowEffect;
class QWidget;

namespace ui {

/** 带 ThemeTokens 动态 QSS 的 QPushButton 封装 */
class UI_COMPONENT_EXPORT ThemedPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ThemedPushButton(QWidget *parent = nullptr);
    explicit ThemedPushButton(const QString &text, PushButtonVariant variant,
                              QWidget *parent = nullptr,
                              bool dropShadow = false);

    PushButtonVariant variant() const { return m_variant; }
    /** 切换语义变体并立即刷新 QSS */
    void setVariant(PushButtonVariant variant);

    bool dropShadowEnabled() const { return m_dropShadow; }
    /** 启用/关闭底部投影（ThemeTokens::dropShadow） */
    void setDropShadowEnabled(bool enabled);

protected:
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
    void updateDropShadow();

    PushButtonVariant           m_variant     = PushButtonVariant::Secondary;
    bool                        m_dropShadow  = false;
    QGraphicsDropShadowEffect  *m_shadowFx    = nullptr;
};

/** 工厂：创建带文案与变体的 ThemedPushButton */
UI_COMPONENT_EXPORT ThemedPushButton *makePushButton(const QString &text, PushButtonVariant variant,
                                 QWidget *parent = nullptr, bool dropShadow = false);

} // namespace ui
