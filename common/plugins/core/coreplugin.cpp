#include "coreplugin.h"
#include "mainwindow.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "uibase/thememanager.h"

#include <Config.h>

#include <QCoreApplication>
#include <QDebug>

using namespace ExtensionSystem;

CorePlugin::CorePlugin() = default;
CorePlugin::~CorePlugin() = default;

bool CorePlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    // ── 1. 初始化主题 ─────────────────────────────────────────────────────
    // styles 目录由构建系统在 POST_BUILD 步骤复制到可执行文件同级目录
    const QString stylesDir = QCoreApplication::applicationDirPath() + QLatin1String("/styles");
    TM->initialize(stylesDir);

    // app.toml [ui] — ACSS 主题名（不含 .xml），与 styles/qt_material/themes/ 下文件名一致
    const QString lightTheme =
        QString::fromStdString(config::ConfigManager::get<std::string>("ui.light_theme",
                                                                        ThemeManager::kFallbackLightTheme));
    const QString darkTheme =
        QString::fromStdString(config::ConfigManager::get<std::string>("ui.dark_theme",
                                                                        ThemeManager::kFallbackDarkTheme));
    TM->setLightDarkAcssThemes(lightTheme, darkTheme);

    const QString initialTheme =
        QString::fromStdString(config::ConfigManager::get<std::string>("ui.theme", darkTheme.toStdString()));
    TM->applyAcssTheme(initialTheme);

    // ── 2. 创建主窗口 ─────────────────────────────────────────────────────
    m_mainWindow = new MainWindow();

    // 将 MainWindow 注册到对象池
    PluginManager::addObject(m_mainWindow);

    qDebug() << "[CorePlugin] initialize: MainWindow created (nav uses PermService after Permission plugin).";
    return true;
}

void CorePlugin::extensionsInitialized()
{
    // 所有插件已完成 initialize()，所有页面都已注册到 MainWindow。
    // 现在按 order 排序并构建导航，然后显示主窗口。
    if (m_mainWindow) {
        qDebug() << "[CorePlugin] extensionsInitialized: finalizing navigation...";
        m_mainWindow->finalizeNavigation();
    }
}

bool CorePlugin::delayedInitialize()
{
    return false;
}

IPlugin::ShutdownFlag CorePlugin::aboutToShutdown()
{
    if (m_mainWindow) {
        PluginManager::removeObject(m_mainWindow);
        m_mainWindow->close();
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }
    return SynchronousShutdown;
}
