/**
 * @file themed_push_button.cpp
 * @brief ThemedPushButton 实现：变体 QSS、可选投影、主题刷新绑定
 */

#include "controls/button/themed_push_button.h"

#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QEvent>
#include <QGraphicsDropShadowEffect>

namespace ui {

ThemedPushButton::ThemedPushButton(QWidget *parent)
    : QPushButton(parent)
{
    bindTheme();
    refreshStyle();
}

ThemedPushButton::ThemedPushButton(const QString &text, PushButtonVariant variant,
                                   QWidget *parent, bool dropShadow)
    : QPushButton(text, parent)
    , m_variant(variant)
    , m_dropShadow(dropShadow)
{
    bindTheme();
    refreshStyle();
}

void ThemedPushButton::setVariant(PushButtonVariant variant)
{
    if (m_variant == variant)
        return;
    m_variant = variant;
    refreshStyle();
}

void ThemedPushButton::setDropShadowEnabled(bool enabled)
{
    if (m_dropShadow == enabled)
        return;
    m_dropShadow = enabled;
    updateDropShadow();
}

void ThemedPushButton::bindTheme()
{
    bindThemeRefresh(this, [this]() {
        refreshStyle();
        updateDropShadow();
    });
}

void ThemedPushButton::refreshStyle()
{
    setStyleSheet(buildPushButtonStylesheet(m_variant));
}

void ThemedPushButton::updateDropShadow()
{
    if (!m_dropShadow) {
        setGraphicsEffect(nullptr);
        m_shadowFx = nullptr;
        return;
    }

    if (!m_shadowFx) {
        m_shadowFx = new QGraphicsDropShadowEffect(this);
        setGraphicsEffect(m_shadowFx);
    }

    const ThemeTokens t = ThemeTokens::resolve();
    m_shadowFx->setBlurRadius(8);
    m_shadowFx->setOffset(0, 2);
    m_shadowFx->setColor(t.dropShadow);
}

void ThemedPushButton::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshStyle();
}

ThemedPushButton *makePushButton(const QString &text, PushButtonVariant variant,
                                 QWidget *parent, bool dropShadow)
{
    return new ThemedPushButton(text, variant, parent, dropShadow);
}

} // namespace ui
