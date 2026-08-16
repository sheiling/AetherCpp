#pragma once

/**
 * @file themed_icon_button.h
 * @brief 主题图标按钮：Unicode glyph / 自定义字体图标，三种布局，可设语义色
 *
 * 布局：纯图标 | 上图下文 | 左图右文。图标色默认跟随 ThemeTokens::textPrimary，
 * 可通过 setIconSemanticColor / setIconColor 覆盖。
 * Font Awesome 等图标字体由调用方通过 setIconDrawFont 注入（见 component/font/fa_icon_button）。
 */

#include "controls/button/icon_button_style.h"
#include "component_global.h"

#include <QColor>
#include <QFont>
#include <QPushButton>
#include <QSize>

class QEvent;
class QPaintEvent;
class QPainter;
class QWidget;

namespace ui {

struct ThemeTokens;

/** 图标与文字排列方式 */
enum class IconButtonLayout {
    IconOnly,           ///< 仅图标（默认 28×28）
    IconTopTextBottom,  ///< 上图标、下文字
    IconLeftTextRight,  ///< 左图标、右文字
};

/** 预设图标语义色（映射 ThemeTokens / HTML 原型） */
enum class IconSemanticColor {
    Foreground,  ///< textPrimary（默认）
    Red,         ///< danger
    Yellow,      ///< warning
    Green,       ///< success
    LightGreen,  ///< 浅绿 #7BC67E
    Orange,      ///< 橘红 #FF8C42
};

class UI_COMPONENT_EXPORT ThemedIconButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ThemedIconButton(QWidget *parent = nullptr);
    explicit ThemedIconButton(const QString &glyph, QWidget *parent = nullptr);

    /** Unicode / emoji 字符（使用主题 UI 字体绘制） */
    void setGlyph(const QString &glyph);

    /** 图标字符（可配合 setIconDrawFont 绘制 FA 等图标字体） */
    void setIconGlyph(const QString &glyph);
    /** checkable 时 Off/On 各一字形 */
    void setToggleIconGlyphs(const QString &offGlyph, const QString &onGlyph);

    /** 非空时使用该字体绘制图标；空字体回退 themeUiFont */
    void setIconDrawFont(const QFont &font);

    void setLayout(IconButtonLayout layout);
    IconButtonLayout layout() const { return m_layout; }

    void setCaption(const QString &text);
    QString caption() const { return m_caption; }

    void setIconPixelSize(int px);
    int iconPixelSize() const { return m_iconPixelSize; }

    /** 无效 QColor 表示跟随前景色 */
    void setIconColor(const QColor &color);
    QColor iconColor() const { return m_iconColor; }

    void setIconSemanticColor(IconSemanticColor role);
    IconSemanticColor iconSemanticColor() const { return m_iconSemanticColor; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
    void syncButtonChrome();
    QColor resolvedIconColor() const;
    QString activeIconGlyph() const;
    QFont iconDrawFont() const;
    void paintContent(QPainter &p, const QRect &contentRect, const ThemeTokens &tokens,
                      bool enabled) const;

    IconButtonLayout   m_layout            = IconButtonLayout::IconOnly;
    QString            m_iconGlyph;
    QString            m_iconGlyphChecked;
    bool               m_hasToggleGlyphs   = false;
    QFont              m_iconDrawFont;
    bool               m_useCustomIconFont = false;
    QString            m_caption;
    int                m_iconPixelSize       = 14;
    QColor             m_iconColor;
    IconSemanticColor  m_iconSemanticColor   = IconSemanticColor::Foreground;
    bool               m_useCustomIconColor  = false;
};

/** 工厂：Unicode glyph 按钮 */
UI_COMPONENT_EXPORT ThemedIconButton *makeIconButton(const QString &glyph, QWidget *parent = nullptr,
                                                     const QString &tooltipKey = QString());

QColor UI_COMPONENT_EXPORT resolveIconSemanticColor(IconSemanticColor role);

} // namespace ui
