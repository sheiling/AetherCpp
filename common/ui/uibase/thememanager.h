#pragma once

#include "uibase_global.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QHash>
#include <functional>

namespace acss { class QtAdvancedStylesheet; }

/**
 * @brief 皮肤主题管理器（单例）
 *
 * 封装 Qt-Advanced-Stylesheets，提供：
 *  - Light / Dark 枚举切换（映射到可配置的 ACSS 主题名）
 *  - 细粒度的 ACSS 样式 / 主题名称选取（applyAcssTheme / setCurrentAcssTheme）
 *  - 主题色变量访问
 *
 * 亮/暗 ACSS 主题名默认见 kFallbackLightTheme / kFallbackDarkTheme；
 * 启动时由 CorePlugin 从 app.toml [ui].light_theme / dark_theme 注入，
 * 也可运行时调用 setLightDarkAcssThemes() 覆盖。
 *
 * 变量扩展：registerBeforeStylesheetHook() 注册的回调在每次 updateStylesheet
 * **之前**同步执行（setCurrentTheme 仅解析 xml，不生成 QSS，不会提前刷新界面）。
 */
class UIBASE_EXPORT ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum Theme { Light, Dark };
    Q_ENUM(Theme)

    static ThemeManager *instance();

    /** ACSS 主题名 fallback（config 未配置时使用） */
    static constexpr const char *kFallbackStyle      = "qt_material";
    static constexpr const char *kFallbackLightTheme = "light_blue";
    static constexpr const char *kFallbackDarkTheme  = "dark_aether";

    /**
     * 初始化 ACSS 引擎。
     * @param stylesDirPath  styles 目录（通常为 <appDir>/styles）
     * @param outputDirPath  ACSS 生成文件的缓存目录（可为空，默认使用临时目录）
     */
    void initialize(const QString &stylesDirPath,
                    const QString &outputDirPath = QString());

    bool isInitialized() const;

    // ── 简单 Light / Dark 切换 ──────────────────────────────────────────
    Theme currentTheme() const { return m_theme; }
    bool  isDark()        const { return m_theme == Dark; }

    /** 当前 ACSS xml 是否 dark="1"（setCurrentTheme 之后、updateStylesheet 之前可用来区分亮/暗色板） */
    bool isCurrentAcssThemeDark() const;

    void setTheme(Theme theme);
    void toggleTheme();

    /** 配置 toggleTheme / setTheme(Light|Dark) 使用的 ACSS 主题名（不含 .xml） */
    void setLightDarkAcssThemes(const QString &lightTheme, const QString &darkTheme);
    QString lightAcssTheme() const { return m_lightAcssTheme; }
    QString darkAcssTheme() const { return m_darkAcssTheme; }

    // ── ACSS 细粒度接口 ────────────────────────────────────────────────
    QStringList availableStyles()  const;
    QStringList availableThemes()  const;

    QString currentStyle()     const;
    QString currentAcssTheme() const;

    void setCurrentStyle(const QString &style);
    void setCurrentAcssTheme(const QString &theme);

    /** 切换 ACSS 主题并可选覆盖变量（如 success/warning/danger），随后刷新全局 QSS。*/
    void applyAcssTheme(const QString &theme, const QHash<QString, QString> &variables = {});

    /** 覆盖单个 ACSS 变量（material.json 或主题 xml 中的键） */
    void setThemeVariable(const QString &variableId, const QString &value);

    /** 批量覆盖 ACSS 变量并刷新全局样式表 */
    void setThemeVariables(const QHash<QString, QString> &variables);

    /** 仅写入 ACSS 变量，不触发 updateStylesheet（供 beforeStylesheet 钩子使用） */
    void setThemeVariableValues(const QHash<QString, QString> &variables);

    /**
     * 注册扩展钩子：在每次 ACSS updateStylesheet 之前同步调用（Direct，非 Queued）。
     * 典型用途：setCurrentTheme 清空插件变量后、生成 QSS 前重注入色板。
     */
    void registerBeforeStylesheetHook(std::function<void()> hook);

    /** 重新生成并应用当前 ACSS 样式表到 qApp */
    void refreshStylesheet();

    /**
     * 读取主题颜色变量，例如 themeColor("primaryColor")。
     * 若 ACSS 未初始化或变量不存在，返回无效 QColor。
     */
    QColor themeColor(const QString &variableId) const;

    /** 读取 ACSS 样式变量（如 material.json 中的 font_family、danger） */
    QString themeVariable(const QString &variableId) const;

    /**
     * 直接访问底层 ACSS 对象（可为 nullptr）。
     */
    acss::QtAdvancedStylesheet *acss() const { return m_acss; }

signals:
    void themeChanged(ThemeManager::Theme newTheme);
    /** ACSS 样式表刷新完成（主题变化、样式变化均会触发）*/
    void stylesheetUpdated();

private:
    explicit ThemeManager(QObject *parent = nullptr);

    /**
     * 将 ACSS 当前生成的 QSS 写入 qApp->setStyleSheet()。
     * 由 QtAdvancedStylesheet::stylesheetChanged 信号触发，或在 initialize 末尾间接调用。
     * 若 qApp 尚未创建则静默跳过。
     */
    void applyCurrentStylesheet();

    /**
     * 按注册顺序同步执行 m_beforeStylesheetHooks 中的回调（Direct，非 Queued）。
     * 在 updateStylesheet() 之前调用，供插件在 QSS 生成前重注入 theme 变量。
     */
    void runBeforeStylesheetHooks();

    /**
     * 样式刷新核心链路：runBeforeStylesheetHooks() → m_acss->updateStylesheet()。
     * updateStylesheet 成功后会 emit stylesheetChanged，进而 applyCurrentStylesheet + stylesheetUpdated。
     * 所有会改变全局 QSS 的公开 API 最终都应汇聚到此函数。
     */
    void refreshAcssStylesheet();

    /** Light/Dark 枚举 → ACSS 主题名（不含 .xml），分别读 m_lightAcssTheme / m_darkAcssTheme */
    QString acssThemeNameFor(Theme theme) const;

    /**
     * 根据 ACSS 当前主题的 dark 属性回写 m_theme（Light/Dark）。
     * 用于 applyAcssTheme 等按 ACSS 主题名切换、而非 setTheme 枚举切换的路径。
     */
    void syncThemeEnumFromAcss();

    /** 逻辑亮/暗状态；与 ACSS currentTheme 在 setTheme/applyAcssTheme 成功后保持一致 */
    Theme m_theme = Light;

    /** setTheme(Light) / toggleTheme 亮侧使用的 ACSS 主题名，可由 setLightDarkAcssThemes 或 CorePlugin 覆盖 */
    QString m_lightAcssTheme = QString::fromLatin1(kFallbackLightTheme);

    /** setTheme(Dark) / toggleTheme 暗侧使用的 ACSS 主题名 */
    QString m_darkAcssTheme = QString::fromLatin1(kFallbackDarkTheme);

    /** Qt-Advanced-Stylesheets 引擎；initialize 前为 nullptr，isInitialized() 据此判断 */
    acss::QtAdvancedStylesheet *m_acss = nullptr;

    /** updateStylesheet 前执行的扩展钩子列表（如 AOI applyAoiGlobalThemeVariables） */
    QList<std::function<void()>> m_beforeStylesheetHooks;
};

#define TM (ThemeManager::instance())
