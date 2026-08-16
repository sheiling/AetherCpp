#pragma once

/**
 * @file themed_glass_indicator_dot.h
 * @brief 主题玻璃质感状态指示灯：可设直径与主色，带光晕高光
 */

#include <QColor>
#include <QWidget>
#include "component_global.h"

namespace ui {

/** 自绘圆点指示灯（#themedGlassIndicatorDot），颜色由 setAccentColor 驱动 */
class UI_COMPONENT_EXPORT ThemedGlassIndicatorDot : public QWidget
{
    Q_OBJECT

public:
    explicit ThemedGlassIndicatorDot(QWidget *parent = nullptr);

    /** 灯珠本体直径（像素），外圈光晕会额外占位 */
    void setDiameter(int pixels);
    int  diameter() const { return m_diameter; }

    /** 主色（连接态语义色），主题切换时仍用此色重绘 */
    void setAccentColor(const QColor &color);

protected:
    QSize sizeHint() const override;
    void  paintEvent(QPaintEvent *event) override;

private:
    void bindTheme();
    void refreshAccentFromTheme();

    int    m_diameter   = 10;
    QColor m_accentColor;
    bool   m_useThemeFallback = true;
};

} // namespace ui
