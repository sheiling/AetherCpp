#pragma once

/**
 * @file theme_tokens.h
 * @brief 从 ThemeManager（ACSS 七原色 + 语义扩展变量）解析自绘控件语义色板
 *
 * 解析链路：ThemeManager::themeColor / themeVariable → AcssBasePalette /
 * AcssSemanticPalette → ThemeTokens::resolve()。七原色键名与 ACSS xml 一致；
 * success/warning/danger 来自 material.json variables；其余 token 按 HTML 原型
 *（ui_aoi/styles/main.css）与 `theme_color_math.h` 中的 blend/shade 规则派生。
 *
 * 典型用法：在 paintEvent、applyStyle、build*Stylesheet 中调用
 * `const ThemeTokens t = ThemeTokens::resolve();`，再读取对应字段。
 */

#include "theme/theme_color_math.h"

#include "uibase/thememanager.h"

#include <QColor>
#include <QStringList>

namespace ui {

/** @return HTML 原型 success 色（#33C06B），ACSS 变量缺失时使用 */
inline QColor htmlFallbackSuccess()
{
    return QColor(QStringLiteral("#33C06B"));
}
/** @return HTML 原型 warning 色（#F0C244） */
inline QColor htmlFallbackWarning()
{
    return QColor(QStringLiteral("#F0C244"));
}
/** @return HTML 原型 danger 色（#F25F5C） */
inline QColor htmlFallbackDanger()
{
    return QColor(QStringLiteral("#F25F5C"));
}

/**
 * 按顺序尝试多个 ACSS themeColor 键名，返回第一个有效色。
 * @param names 候选键名列表（如 secondaryDarkColor、secondaryColor）
 * @return 首个有效 QColor，全部无效时返回无效 QColor
 */
inline QColor themeColorChain(const QStringList &names)
{
    for (const QString &n : names) {
        const QColor c = TM ? TM->themeColor(n) : QColor{};
        if (c.isValid()) {
            return c;
        }
    }
    return QColor{};
}

/**
 * 读取单个 ACSS 主题色。
 * @param name ThemeManager::themeColor 键名
 * @param fallback 键缺失或颜色无效时的回退值
 */
inline QColor themeColorOrDefault(const QString &name, const QColor &fallback = QColor{})
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

/**
 * 读取 material.json 中的 CSS 变量并解析为 QColor。
 * @param name 变量名（success / warning / danger 等）
 * @param htmlFallback 变量为空或解析失败时的 HTML 原型色
 */
inline QColor resolveAcssThemeVariable(const QString &name, const QColor &htmlFallback = QColor{})
{
    if (!TM)
        return htmlFallback;
    const QString v = TM->themeVariable(name);
    if (v.isEmpty())
        return htmlFallback;
    const QColor c(v);
    return c.isValid() ? c : htmlFallback;
}

/** @return 从 ThemeManager 读取的 ACSS 七原色（见 AcssBasePalette） */
inline AcssBasePalette resolveAcssBasePalette()
{
    AcssBasePalette p;
    p.pageBg        = themeColorChain({QStringLiteral("secondaryDarkColor")});
    p.surface       = themeColorChain({QStringLiteral("secondaryColor")});
    p.border        = themeColorChain({QStringLiteral("secondaryLightColor")});
    p.textPrimary   = themeColorChain({QStringLiteral("primaryTextColor")});
    p.textSecondary = themeColorChain({QStringLiteral("secondaryTextColor")});
    p.accentLight   = themeColorChain({QStringLiteral("primaryLightColor")});
    p.accent        = themeColorChain({QStringLiteral("primaryColor")});
    return p;
}

/** ACSS 语义三色（material.json variables），供 ThemeTokens 派生 success/warning/danger 系列 */
struct AcssSemanticPalette
{
    QColor success;  ///< 成功/正常态基色
    QColor warning;  ///< 警告/待机态基色
    QColor danger;   ///< 错误/断开态基色
};

/** @return material.json 中的 success / warning / danger，缺失时用 HTML fallback */
inline AcssSemanticPalette resolveAcssSemanticPalette()
{
    AcssSemanticPalette s;
    s.success = resolveAcssThemeVariable(QStringLiteral("success"), htmlFallbackSuccess());
    s.warning = resolveAcssThemeVariable(QStringLiteral("warning"), htmlFallbackWarning());
    s.danger  = resolveAcssThemeVariable(QStringLiteral("danger"), htmlFallbackDanger());
    return s;
}

/**
 * 一次解析的 UI 语义色板，供 `*Style` / paintEvent 共用。
 * 字段注释中的「典型消费者」对应当前 `plugins/aoi/ui/widgets` 用法。
 */
struct ThemeTokens
{
    /** 页面根背景 — `HomeView` 根节点；HTML `--bg` */
    QColor background;

    /** 卡片/侧栏面板底 — HTML `--panel` */
    QColor surface;

    /** KPI 等抬升面板底 — HTML `--panel-2` */
    QColor surfaceElevated;

    /** 卡片/面板外框 — HTML `--border` */
    QColor surfaceBorder;

    /** 主正文 — HTML `--text` */
    QColor textPrimary;

    /** 次要标签 — HTML `--muted` */
    QColor textSecondary;

    /** 强调色（默认态）— HTML `--primary`；Primary 按钮边框、Tab 激活 */
    QColor accent;

    /** 强调色 hover — Material 400 或 HTML 半透明 hover */
    QColor accentHover;

    /** 强调色按下 — HTML `primaryColor` 深色阶 */
    QColor accentPressed;

    /** 强调色描边 — 同 accent */
    QColor accentBorder;

    /** Primary 按钮半透明底 — HTML `rgba(primary, 0.25)` */
    QColor accentBg;

    /** Primary 按钮 hover 半透明底 — HTML `rgba(primary, 0.45)` */
    QColor accentBgHover;

    /** 焦点环 — 输入框 focus、`ImagePreview` 聚焦框 */
    QColor focus;

    /** 输入类控件底色 — HTML `input` `#111d31` 派生 */
    QColor inputBackground;

    /** 输入类默认边框 */
    QColor inputBorder;

    /** 输入类 hover 边框 — accentLight */
    QColor inputBorderHover;

    /** 网格线/弱分隔 — HTML 表格线 */
    QColor divider;

    /** SpinBox 箭头（启用） */
    QColor arrowActive;

    /** SpinBox 箭头（禁用） */
    QColor arrowDisabled;

    /** `PillSwitch` 关闭轨道 — HTML `#3a4a5e` 派生 */
    QColor switchTrackOff;

    /** `PillSwitch` 开启轨道 — HTML `rgba(primary, 0.45)` */
    QColor switchTrackOn;

    /** `PillSwitch` 关闭轨道边框 */
    QColor switchBorderOff;

    /** `PillSwitch` 开启轨道边框 — HTML `#298de0` 派生 */
    QColor switchBorderOn;

    /** `PillSwitch` 滑块 */
    QColor switchThumb;

    /** SpinBox 步进区背景 */
    QColor spinButtonBg;

    /** 表头/分组标题默认底 — HTML 表头 `#152a45` 派生 */
    QColor headerBg;

    /** 表头/分组标题 hover 底 — surface 与 accentLight 更高混合比 */
    QColor headerBgHover;

    /** 表头/分组标题按下底 — 较 headerBg 更暗的 accent 着色 */
    QColor headerBgPressed;

    /** 图标/箭头按钮默认底 — 通常等于 surface */
    QColor iconButtonBg;

    /** 图标/箭头按钮 hover 底 — surface 与 border 混合 */
    QColor iconButtonBgHover;

    /** 弱化文字（textSecondary 别名） */
    QColor mutedText;

    /** 高对比前景 — Primary 按钮文字、表头文字 */
    QColor highlightText;

    /** 运行日志终端底 — HTML `.log-box` `#0a111f` 派生 */
    QColor logBackground;

    /** Success 文字/指示 — HTML `--ok`；连接正常、OK 结果、INFO 日志 */
    QColor successText;

    /** Success 边框 — 与 successText 同色 */
    QColor successBorder;

    /** Success 半透明底 — surface 与 success 混合（≈ rgba 0.2） */
    QColor successBg;

    /** Success 按钮/标签 hover 底 */
    QColor successBgHover;

    /** Success 按钮/标签按下底 — Material 700 色阶 */
    QColor successBgPressed;

    /** Warning 文字/指示 — HTML `--warn`；RW、待机、WARN 日志 */
    QColor warningText;

    /** Warning 边框 */
    QColor warningBorder;

    /** Warning 半透明底 */
    QColor warningBg;

    /** Warning hover 底 */
    QColor warningBgHover;

    /** Warning 按下底 */
    QColor warningBgPressed;

    /** Danger 文字/指示 — HTML `--error`；NG、断开、ERROR 日志 */
    QColor dangerText;

    /** Danger 边框 */
    QColor dangerBorder;

    /** Danger 半透明底 */
    QColor dangerBg;

    /** Danger hover 底 */
    QColor dangerBgHover;

    /** Danger 按下底 */
    QColor dangerBgPressed;

    /** 投影色 — textSecondary 带 alpha，用于卡片/浮层阴影 */
    QColor dropShadow;

    /**
     * 一次性解析完整语义色板；每次调用均从 ThemeManager 重新读取并派生。
     * @return 当前主题下的 ThemeTokens 快照
     */
    static ThemeTokens resolve()
    {
        const AcssBasePalette   base = resolveAcssBasePalette();
        const AcssSemanticPalette sem = resolveAcssSemanticPalette();
        ThemeTokens             t;

        // ── 布局：七原色 + HTML --panel-2 ───────────────────────────────
        t.background       = base.pageBg;
        t.surface          = base.surface;
        t.surfaceBorder    = base.border;
        t.surfaceElevated  = themeBlend(base.surface, base.border, 0.42);

        t.textPrimary   = base.textPrimary;
        t.textSecondary = base.textSecondary;
        t.mutedText     = base.textSecondary;
        t.highlightText = base.textPrimary;

        // ── 强调色：accentLight=500，accent=700，半透明按钮底 ───────────
        t.accent        = base.accentLight;
        t.accentHover   = themeMaterialShade(base.accentLight, 400);
        t.accentPressed = base.accent;
        t.accentBorder  = base.accentLight;
        t.accentBg      = themeBlend(base.surface, base.accentLight, 0.25);
        t.accentBgHover = themeBlend(base.surface, base.accentLight, 0.45);
        t.focus         = base.accentLight;

        // ── 输入类：HTML input #111d31 ≈ pageBg↔surface 混合 ─────────────
        t.inputBackground  = themeBlend(base.pageBg, base.surface, 0.48);
        t.inputBorder      = themeBlend(base.border, base.accentLight, 0.10);
        t.inputBorderHover = base.accentLight;
        t.divider          = base.border;

        t.arrowActive   = base.textPrimary;
        t.arrowDisabled = base.textSecondary;

        // ── Switch：对照 HTML .switch ────────────────────────────────────
        t.switchTrackOff  = themeBlend(base.surface, base.border, 0.62);
        t.switchTrackOn   = t.accentBgHover;
        t.switchBorderOff = themeBlend(base.border, base.textSecondary, 0.32);
        t.switchBorderOn  = themeMaterialShade(base.accentLight, 400);
        t.switchThumb     = base.textPrimary;

        t.spinButtonBg = t.inputBackground;

        // ── 表头：HTML th #152a45 ≈ surface + accentLight 着色 ─────────
        t.headerBg        = themeBlend(base.surface, base.accentLight, 0.18);
        t.headerBgHover   = themeBlend(base.surface, base.accentLight, 0.28);
        t.headerBgPressed = themeBlend(base.surface, base.accentLight, 0.12);

        t.iconButtonBg      = base.surface;
        t.iconButtonBgHover = themeBlend(base.surface, base.border, 0.55);

        // ── 日志终端：HTML .log-box #0a111f ≈ pageBg 压暗 ───────────────
        t.logBackground = themeDarken(base.pageBg, 0.28);

        // ── 语义色：material.json + HTML rgba(*, 0.2) 派生 ──────────────
        const SemanticColorTokens okTokens =
            themeSemanticTokens(base.surface, sem.success);
        t.successText       = okTokens.text;
        t.successBorder     = okTokens.border;
        t.successBg         = okTokens.bg;
        t.successBgHover    = okTokens.bgHover;
        t.successBgPressed  = okTokens.bgPressed;

        const SemanticColorTokens warnTokens =
            themeSemanticTokens(base.surface, sem.warning);
        t.warningText       = warnTokens.text;
        t.warningBorder     = warnTokens.border;
        t.warningBg         = warnTokens.bg;
        t.warningBgHover    = warnTokens.bgHover;
        t.warningBgPressed  = warnTokens.bgPressed;

        const SemanticColorTokens errTokens =
            themeSemanticTokens(base.surface, sem.danger);
        t.dangerText       = errTokens.text;
        t.dangerBorder     = errTokens.border;
        t.dangerBg         = errTokens.bg;
        t.dangerBgHover    = errTokens.bgHover;
        t.dangerBgPressed  = errTokens.bgPressed;

        t.dropShadow = themeWithAlpha(base.textSecondary, 72);

        return t;
    }
};

} // namespace ui
