#include "plugin_loader_module.h"
#include "bootstrap_context.h"
#include "bootstrap_config.h"
#include "core_modules.h"

#include "../splashscreen.h"
#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginspec.h"
#include "uibase/imainwindowservice.h"

#include <Logger.h>

#include <QEventLoop>
#include <QApplication>
#include <QCoreApplication>
#include <QLatin1String>
#include <QWidget>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <exception>

namespace {

void dismissSplashScreen(BootstrapContext &ctx)
{
    if (!ctx.splashScreen)
        return;

    ctx.splashScreen->close();
    if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
        app->processEvents();

    if (ctx.releaseSplashScreen)
        ctx.releaseSplashScreen();
}

} // namespace

PluginLoaderModule::~PluginLoaderModule() = default;

QString PluginLoaderModule::name() const
{
    return QStringLiteral("PluginLoader");
}

int PluginLoaderModule::priority() const
{
    return 100;
}

bool PluginLoaderModule::onRegister(BootstrapContext &ctx)
{
    const BootstrapConfig *cfg = ctx.config;
    if (!cfg) {
        LOG_ERROR("[bootstrap] PluginLoaderModule：缺少 BootstrapConfig");
        return false;
    }

    if (!pluginManager_)
        pluginManager_ = std::make_unique<ExtensionSystem::PluginManager>();
    ctx.pluginManager = pluginManager_.get();

    if (!cfg->pluginIid.isEmpty())
        ExtensionSystem::PluginManager::setPluginIID(cfg->pluginIid);

    QStringList paths = cfg->pluginSearchPaths;
    if (paths.isEmpty()) {
        const QString pluginsDir =
            QCoreApplication::applicationDirPath() + QLatin1String("/plugins");
        paths << pluginsDir;
    }

    LOG_INFO("[bootstrap] 插件搜索路径: {}",
             paths.join(QStringLiteral("; ")).toStdString());
    ExtensionSystem::PluginManager::setPluginPaths(paths);
    return true;
}

bool PluginLoaderModule::onBoot(BootstrapContext &ctx)
{
    const BootstrapConfig *cfg = ctx.config;
    if (!cfg) {
        LOG_ERROR("[bootstrap] PluginLoaderModule：缺少 BootstrapConfig");
        return false;
    }
    if (!pluginManager_) {
        LOG_ERROR("[bootstrap] PluginLoaderModule：PluginManager 未创建");
        return false;
    }

#ifdef Q_OS_WIN
    // 插件 DLL 位于 bin/plugins/，其依赖在 exe 目录；显式加入搜索路径避免 LoadLibrary 失败。
    const QString appDir = QCoreApplication::applicationDirPath();
    SetDllDirectoryW(reinterpret_cast<LPCWSTR>(appDir.utf16()));
    QCoreApplication::addLibraryPath(appDir);
#endif

    if (ctx.splashScreen) {
        ctx.splashScreen->raise();
        if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
            app->processEvents();
    }

    try {
        ExtensionSystem::PluginManager::loadPlugins();

        // SchedulerModule(50) 早于插件加载；AOI 工步在 Aoi 插件 DLL 静态注册，须在此后加载 flows
        loadSchedulerFlowsAfterPlugins();
    } catch (const std::exception &ex) {
        LOG_ERROR("[bootstrap] 插件加载异常: {}", ex.what());
        dismissSplashScreen(ctx);
        return false;
    } catch (...) {
        LOG_ERROR("[bootstrap] 插件加载发生未知异常");
        dismissSplashScreen(ctx);
        return false;
    }

    IMainWindowService *mainWindowSvc =
        ExtensionSystem::PluginManager::getObject<IMainWindowService>();

    QWidget *mainWindow = mainWindowSvc ? mainWindowSvc->mainWindowWidget() : nullptr;
    if (mainWindowSvc)
        mainWindowSvc->showAfterSplash();

    if (mainWindow) {
        // 先以最大化状态显示并完成绘制，再关闭 Splash，避免白框且确保主窗口可见
        mainWindow->showMaximized();
        mainWindow->raise();
        mainWindow->activateWindow();
        if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
            app->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (ctx.splashScreen) {
        if (mainWindow) {
            ctx.splashScreen->finish(mainWindow);
        } else {
            LOG_WARN("[bootstrap] PluginLoaderModule：未找到主窗口，关闭启动画面");
            ctx.splashScreen->close();
        }

        if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
            app->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    if (ctx.releaseSplashScreen)
        ctx.releaseSplashScreen();

    // showAfterSplash 已在 finish 前完成离屏预渲染，此处无需再次 show

    if (ExtensionSystem::PluginManager::hasError() && !cfg->continueOnPluginErrors) {
        LOG_ERROR("[bootstrap] 插件加载出错且 continueOnPluginErrors=false，中止启动");
        return false;
    }

    if (ExtensionSystem::PluginManager::hasError()) {
        LOG_WARN("[bootstrap] 以下插件加载失败：");
        for (auto *spec : ExtensionSystem::PluginManager::plugins()) {
            if (spec->hasError()) {
                LOG_WARN("[bootstrap]   {}: {}",
                         spec->name().toStdString(),
                         spec->errorString().toStdString());
            }
        }
    }

    return true;
}

void PluginLoaderModule::onShutdown(BootstrapContext &ctx)
{
    if (ctx.pluginManager == pluginManager_.get())
        ctx.pluginManager = nullptr;
    if (!pluginManager_)
        return;
    pluginManager_->shutdown();
    pluginManager_.reset();
}
