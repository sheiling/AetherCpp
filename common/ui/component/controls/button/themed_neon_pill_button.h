#pragma once

/**
 * @file themed_neon_pill_button.h
 * @brief 胶囊霓虹按钮：可设主体/强调色、光晕、阴影，中心文字
 *
 * 横向圆角胶囊自绘控件，风格参考多色霓虹玻璃按钮条：暗色半透明盘面、
 * accent 描边与底部内光晕、左右端点亮边；文字居中并带轻微发光。
 * 绘制逻辑内置于 themed_neon_pill_button.cpp。
 */

#include <QAbstractButton>
#include <QColor>
#include <QString>
#include "component_global.h"

class QEvent;

namespace ui {

/** 胶囊霓虹按钮；尺寸由 pillHeight + 文字宽度决定 */
class UI_COMPONENT_EXPORT ThemedNeonPillButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int pillHeight READ pillHeight WRITE setPillHeight)
    Q_PROPERTY(int minPillWidth READ minPillWidth WRITE setMinPillWidth)
    Q_PROPERTY(int horizontalPadding READ horizontalPadding WRITE setHorizontalPadding)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor)
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(bool haloEnabled READ haloEnabled WRITE setHaloEnabled)
    Q_PROPERTY(bool shadowEnabled READ shadowEnabled WRITE setShadowEnabled)

public:
    explicit ThemedNeonPillButton(QWidget *parent = nullptr);
    explicit ThemedNeonPillButton(const QString &text, QWidget *parent = nullptr);

    int pillHeight() const { return m_pillHeight; }
    void setPillHeight(int height);

    int minPillWidth() const { return m_minPillWidth; }
    void setMinPillWidth(int width);

    int horizontalPadding() const { return m_horizontalPadding; }
    void setHorizontalPadding(int padding);

    QColor accentColor() const { return m_accentColor; }
    void setAccentColor(const QColor &color);

    QColor bodyColor() const { return m_bodyColor; }
    void setBodyColor(const QColor &color);

    bool haloEnabled() const { return m_haloEnabled; }
    void setHaloEnabled(bool enabled);

    bool shadowEnabled() const { return m_shadowEnabled; }
    void setShadowEnabled(bool enabled);

    void setText(const QString &text);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;

private:
    void bindTheme();
    void updateSizeFromContent();
    QRectF pillRectInWidget() const;
    int    pillContentWidth() const;

    int    m_pillHeight         = 40;
    int    m_minPillWidth       = 96;
    int    m_horizontalPadding  = 28;
    QColor m_accentColor;
    QColor m_bodyColor;
    bool   m_haloEnabled        = true;
    bool   m_shadowEnabled      = true;
};

/** 工厂：创建带文案的胶囊霓虹按钮，可选 accent */
UI_COMPONENT_EXPORT ThemedNeonPillButton *makeNeonPillButton(const QString &text, QWidget *parent = nullptr,
                                         const QColor &accentColor = {});

} // namespace ui
