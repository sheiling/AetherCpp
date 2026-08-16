/**
 * @file thememanager.cpp
 * @brief ThemeManager 实现：ACSS 初始化、亮/暗切换、变量钩子与全局 QSS 应用
 *
 * ACSS 两阶段语义（避免 hook 时序误解）：
 *  1. setCurrentTheme(name) — 仅加载/切换 xml，更新 isCurrentThemeDark()，**不**生成 QSS
 *  2. updateStylesheet()    — 按当前主题与变量生成 QSS，emit stylesheetChanged
 *
 * 因此 registerBeforeStylesheetHook 注册的逻辑插在 (1) 与 (2) 之间是正确时机。
 */

#include "thememanager.h"

#include <QtAdvancedStylesheet.h>

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

ThemeManager *ThemeManager::instance()
{
    static ThemeManager inst;
    return &inst;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
}

// ---------------------------------------------------------------------------
// 初始化
// ---------------------------------------------------------------------------

void ThemeManager::initialize(const QString &stylesDirPath, const QString &outputDirPath)
{
    // 允许重复 initialize（如热重载场景）：先释放旧 ACSS 实例
    if (m_acss) {
        qWarning() << "[ThemeManager] Already initialized, re-initializing.";
        delete m_acss;
    }

    m_acss = new acss::QtAdvancedStylesheet(this);

    // ACSS 会将 SCSS/变量编译结果写入 outputDir；未指定时用 AppLocalData/styles_cache
    const QString outDir = outputDirPath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
              + QLatin1String("/styles_cache")
        : outputDirPath;

    QDir().mkpath(outDir);

    m_acss->setStylesDirPath(stylesDirPath);
    m_acss->setOutputDirPath(outDir);

    // stylesheetChanged：ACSS 完成 updateStylesheet 后触发
    //  - applyCurrentStylesheet：把 QSS 挂到 qApp
    //  - stylesheetUpdated：通知 PageWidget 等做控件级 refreshTheme()
    connect(m_acss, &acss::QtAdvancedStylesheet::stylesheetChanged,
            this, &ThemeManager::applyCurrentStylesheet);
    connect(m_acss, &acss::QtAdvancedStylesheet::stylesheetChanged,
            this, &ThemeManager::stylesheetUpdated);

    // 选定 style 包（如 qt_material），失败仅告警，不阻断启动
    if (!m_acss->setCurrentStyle(QLatin1String(kFallbackStyle))) {
        qWarning() << "[ThemeManager] Style not found:" << kFallbackStyle << " in" << stylesDirPath;
    }

    // 按 m_theme 映射到 m_lightAcssTheme / m_darkAcssTheme（config 可在 initialize 前注入）
    if (!m_acss->setCurrentTheme(acssThemeNameFor(m_theme))) {
        qWarning() << "[ThemeManager] Theme not found:" << acssThemeNameFor(m_theme);
    }

    // 首次生成 QSS 并 apply（内部会先 runBeforeStylesheetHooks）
    refreshAcssStylesheet();

    if (m_acss->error() != acss::QtAdvancedStylesheet::NoError) {
        qWarning() << "[ThemeManager] ACSS error:" << m_acss->errorString();
    }
}

bool ThemeManager::isInitialized() const
{
    return m_acss != nullptr;
}

bool ThemeManager::isCurrentAcssThemeDark() const
{
    return m_acss && m_acss->isCurrentThemeDark();
}

QString ThemeManager::acssThemeNameFor(Theme theme) const
{
    return (theme == Dark) ? m_darkAcssTheme : m_lightAcssTheme;
}

void ThemeManager::setLightDarkAcssThemes(const QString &lightTheme, const QString &darkTheme)
{
    // 空串表示保留当前值，便于只改一侧
    if (!lightTheme.isEmpty())
        m_lightAcssTheme = lightTheme;
    if (!darkTheme.isEmpty())
        m_darkAcssTheme = darkTheme;
}

void ThemeManager::runBeforeStylesheetHooks()
{
    for (const std::function<void()> &hook : m_beforeStylesheetHooks) {
        if (hook)
            hook();
    }
}

void ThemeManager::refreshAcssStylesheet()
{
    if (!m_acss)
        return;
    runBeforeStylesheetHooks();
    m_acss->updateStylesheet();
}

void ThemeManager::registerBeforeStylesheetHook(std::function<void()> hook)
{
    if (hook)
        m_beforeStylesheetHooks.append(std::move(hook));
}

void ThemeManager::syncThemeEnumFromAcss()
{
    if (!m_acss)
        return;
    m_theme = m_acss->isCurrentThemeDark() ? Dark : Light;
}

// ---------------------------------------------------------------------------
// Light / Dark 枚举切换（标题栏 toggleTheme 等）
// ---------------------------------------------------------------------------

void ThemeManager::setTheme(Theme theme)
{
    // ACSS 未初始化：仅更新内存枚举并通知（极少见，通常 initialize 在前）
    if (!m_acss) {
        const bool changed = (m_theme != theme);
        m_theme            = theme;
        if (changed)
            emit themeChanged(theme);
        return;
    }

    const Theme   previous  = m_theme;
    const QString themeName = acssThemeNameFor(theme);
    if (!m_acss->setCurrentTheme(themeName)) {
        qWarning() << "[ThemeManager] Failed to set ACSS theme:" << themeName
                   << "available:" << m_acss->themes();
        return;
    }

    refreshAcssStylesheet();

    // 仅在 ACSS 切换成功后再改 m_theme，避免枚举与 xml 不一致
    m_theme = theme;

    if (previous != m_theme) {
        emit themeChanged(m_theme);
    }
}

void ThemeManager::toggleTheme()
{
    setTheme(m_theme == Light ? Dark : Light);
}

// ---------------------------------------------------------------------------
// ACSS 细粒度查询
// ---------------------------------------------------------------------------

QStringList ThemeManager::availableStyles() const
{
    return m_acss ? m_acss->styles() : QStringList{};
}

QStringList ThemeManager::availableThemes() const
{
    return m_acss ? m_acss->themes() : QStringList{};
}

QString ThemeManager::currentStyle() const
{
    return m_acss ? m_acss->currentStyle() : QString{};
}

QString ThemeManager::currentAcssTheme() const
{
    return m_acss ? m_acss->currentTheme() : QString{};
}

void ThemeManager::setCurrentStyle(const QString &style)
{
    if (!m_acss)
        return;
    m_acss->setCurrentStyle(style);
    refreshAcssStylesheet();
}

void ThemeManager::setCurrentAcssTheme(const QString &theme)
{
    applyAcssTheme(theme);
}

void ThemeManager::applyAcssTheme(const QString &theme, const QHash<QString, QString> &variables)
{
    if (!m_acss)
        return;

    const bool wasDark = m_acss->isCurrentThemeDark();
    if (!m_acss->setCurrentTheme(theme)) {
        qWarning() << "[ThemeManager] Failed to set ACSS theme:" << theme;
        return;
    }

    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
        m_acss->setThemeVariableValue(it.key(), it.value());

    refreshAcssStylesheet();

    syncThemeEnumFromAcss();
    const bool isDarkNow = m_acss->isCurrentThemeDark();
    // 仅亮/暗属性变化时 emit themeChanged（同 dark 属性换主题名不通知）
    if (wasDark != isDarkNow) {
        emit themeChanged(m_theme);
    }
}

void ThemeManager::setThemeVariable(const QString &variableId, const QString &value)
{
    if (!m_acss)
        return;
    m_acss->setThemeVariableValue(variableId, value);
    refreshAcssStylesheet();
}

void ThemeManager::setThemeVariables(const QHash<QString, QString> &variables)
{
    setThemeVariableValues(variables);
    refreshAcssStylesheet();
}

void ThemeManager::setThemeVariableValues(const QHash<QString, QString> &variables)
{
    if (!m_acss)
        return;
    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
        m_acss->setThemeVariableValue(it.key(), it.value());
}

void ThemeManager::refreshStylesheet()
{
    refreshAcssStylesheet();
}

QColor ThemeManager::themeColor(const QString &variableId) const
{
    return m_acss ? m_acss->themeColor(variableId) : QColor{};
}

QString ThemeManager::themeVariable(const QString &variableId) const
{
    return m_acss ? m_acss->themeVariableValue(variableId) : QString{};
}

// ---------------------------------------------------------------------------
// 全局 QSS 应用
// ---------------------------------------------------------------------------

void ThemeManager::applyCurrentStylesheet()
{
    if (!qApp)
        return;

    if (m_acss) {
        qApp->setStyleSheet(m_acss->styleSheet());
    }
}
