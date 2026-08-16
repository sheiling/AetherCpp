#pragma once

/**
 * @file themed_cyber_pill_button.h
 * @brief 赛博胶囊按钮：暗色主体 + 强调色霓虹描边，左右 chevron 装饰线，中心文字
 *
 * 风格参考素材标签按钮：半透明深色盘面、内外双层描边（外缘淡白 + 内缘 accent 光晕）、
 * 文字两侧水平装饰线与向内 chevron。可配置主体色（bodyColor）与强调色（accentColor）。
 */

#include <QAbstractButton>
#include <QColor>
#include <QString>
#include "component_global.h"

class QEvent;

namespace ui {

/** 赛博风胶囊按钮；宽度随文案与 minPillWidth 自适应 */
class UI_COMPONENT_EXPORT ThemedCyberPillButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(int pillHeight READ pillHeight WRITE setPillHeight)
    Q_PROPERTY(int minPillWidth READ minPillWidth WRITE setMinPillWidth)
    Q_PROPERTY(int horizontalPadding READ horizontalPadding WRITE setHorizontalPadding)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor)
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(bool decorationsEnabled READ decorationsEnabled WRITE setDecorationsEnabled)

public:
    explicit ThemedCyberPillButton(QWidget *parent = nullptr);
    explicit ThemedCyberPillButton(const QString &text, QWidget *parent = nullptr);

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

    QColor textColor() const { return m_textColor; }
    void setTextColor(const QColor &color);

    bool decorationsEnabled() const { return m_decorationsEnabled; }
    void setDecorationsEnabled(bool enabled);

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

    int    m_pillHeight        = 36;
    int    m_minPillWidth      = 88;
    int    m_horizontalPadding = 36;
    QColor m_accentColor;
    QColor m_bodyColor;
    QColor m_textColor;
    bool   m_decorationsEnabled = true;
};

UI_COMPONENT_EXPORT ThemedCyberPillButton *makeCyberPillButton(const QString &text, QWidget *parent = nullptr,
                                           const QColor &accentColor = {});

} // namespace ui
