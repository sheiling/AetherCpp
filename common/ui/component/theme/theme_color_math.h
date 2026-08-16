#pragma once

/**
 * @file theme_color_math.h
 * @brief ACSS 七原色之上的派生色计算（blend / lighten / darken / Material Shade）
 *
 * 七原色见 `AcssBasePalette`；自绘控件应通过 `ThemeTokens::resolve()` 取语义 token，
 * 仅在 token 定义处使用本文件的混合/色阶函数。
 *
 * 主要消费者：theme_tokens.h（ThemeTokens::resolve 派生链）。
 */

#include <QColor>

namespace ui {

/** ACSS 主题 xml 中的 7 个标准色（与 ThemeManager::themeColor 键名一一对应） */
struct AcssBasePalette
{
    QColor pageBg;         ///< secondaryDarkColor — 页面最深层背景
    QColor surface;        ///< secondaryColor — 卡片/输入区/面板底
    QColor border;         ///< secondaryLightColor — 描边、分隔、浅层强调
    QColor textPrimary;    ///< primaryTextColor — 主文字
    QColor textSecondary;  ///< secondaryTextColor — 次要文字
    QColor accentLight;    ///< primaryLightColor — 强调色（亮）
    QColor accent;         ///< primaryColor — 强调色（深）
};

/** 线性混合：weight=0 → a，weight=1 → b */
inline QColor themeBlend(const QColor &a, const QColor &b, qreal weight)
{
    if (!a.isValid())
        return b;
    if (!b.isValid())
        return a;
    const qreal w = qBound<qreal>(0.0, weight, 1.0);
    return QColor(
        qRound(a.red() * (1.0 - w) + b.red() * w),
        qRound(a.green() * (1.0 - w) + b.green() * w),
        qRound(a.blue() * (1.0 - w) + b.blue() * w),
        qRound(a.alpha() * (1.0 - w) + b.alpha() * w));
}

/** 向白色提亮，amount ∈ [0,1] 表示混入白色比例 */
inline QColor themeLighten(const QColor &c, qreal amount)
{
    return themeBlend(c, Qt::white, qBound<qreal>(0.0, amount, 1.0));
}

/** 向黑色压暗，amount ∈ [0,1] 表示混入黑色比例 */
inline QColor themeDarken(const QColor &c, qreal amount)
{
    return themeBlend(c, Qt::black, qBound<qreal>(0.0, amount, 1.0));
}

/**
 * Material 风格色阶：以 base 为 500，50 最浅、900 最深。
 * level 建议取 50 的倍数（50…900）。
 */
inline QColor themeMaterialShade(const QColor &base, int level)
{
    if (!base.isValid())
        return base;
    level = qBound(50, level, 900);
    if (level == 500)
        return base;
    if (level < 500) {
        const qreal t = (500 - level) / 450.0;
        return themeLighten(base, t * 0.92);
    }
    const qreal t = (level - 500) / 400.0;
    return themeDarken(base, t * 0.85);
}

/** 设置 alpha，保留 RGB */
inline QColor themeWithAlpha(const QColor &c, int alpha)
{
    if (!c.isValid())
        return c;
    QColor out = c;
    out.setAlpha(qBound(0, alpha, 255));
    return out;
}

/** 语义色（success / warning / danger）的一组自绘 token，由 themeSemanticTokens 填充 */
struct SemanticColorTokens
{
    QColor text;       ///< 前景/指示文字色
    QColor border;     ///< 边框色（通常与 text 同色）
    QColor bg;         ///< 默认半透明底
    QColor bgHover;    ///< hover 态底
    QColor bgPressed;  ///< 按下态底（Material 700）
};
/**
 * 由语义基色派生按钮/标签/日志用 token。
 * @param surfaceBase 面板底色，用于与 semantic 混合出 bg 系列
 * @param semantic success/warning/danger 基色
 * @param bgMix bg 混合权重，默认 0.20（等效 HTML rgba(semantic, 0.2)）
 */
inline SemanticColorTokens themeSemanticTokens(const QColor &surfaceBase, const QColor &semantic,
                                               qreal bgMix = 0.20){
    SemanticColorTokens t;
    if (!semantic.isValid())
        return t;
    t.text       = semantic;
    t.border     = semantic;
    t.bg         = themeBlend(surfaceBase, semantic, bgMix);
    t.bgHover    = themeBlend(surfaceBase, semantic, 0.50);
    t.bgPressed  = themeMaterialShade(semantic, 700);
    return t;
}

} // namespace ui
