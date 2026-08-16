#include "containerplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"

#include "appservices.h"
#include "container/serviceprovider.h"
#include "container/facade.h"
#include "containertypeview.h"
#include "containernamedview.h"

#include <QDebug>

using namespace ExtensionSystem;

// =============================================================================
// Service Providers（服务提供者）
// =============================================================================

/**
 * @brief AppLoggerProvider — 日志服务提供者
 *
 * doRegister() 阶段注册：
 *   1. 类型绑定（瞬态）：IAppLogger → ConsoleAppLogger
 *      供 ContainerTypeView 演示 bind<>/singleton<>/instance<>/extend<>/contextual/tag
 *   2. 命名单例绑定："logger" → FileAppLogger（含完整反射方法表）
 *      供 ContainerNamedView 演示 makeNamed/call/get/set
 */
class AppLoggerProvider : public Container::ServiceProvider
{
public:
    using ServiceProvider::ServiceProvider;

    void doRegister() override
    {
        // ── 1. 类型绑定（瞬态）：供 ContainerTypeView 内部演示使用 ──────────
        //    注意：ContainerTypeView 每次演示都创建独立的本地 Container，
        //    此处注册主要供 Facade 演示（通过 Application::instance() 访问）
        app_.bind<IAppLogger>([] {
            return std::make_shared<ConsoleAppLogger>();
        });

        // ── 2. 命名单例绑定："logger" → FileAppLogger ────────────────────────
        //    注册反射方法表，使 ContainerNamedView 无需引入 FileAppLogger 头文件
        //    即可通过 handle.call("info", ...) 动态调用方法
        app_.singletonNamed(
            "logger",
            [](Container::Container&) -> std::shared_ptr<void> {
                return std::make_shared<FileAppLogger>("container_demo.log");
            },
            [](Container::MethodDispatcher& d) {
                // ── 方法注册 ─────────────────────────────────────────────────
                d.on("info",    &FileAppLogger::info);     // void info(string)
                d.on("warn",    &FileAppLogger::warn);     // void warn(string)
                d.on("name",    &FileAppLogger::name);     // string name() const
                d.on("lastMsg", &FileAppLogger::lastMsg);  // string lastMsg() const

                // ── 属性注册（读写）──────────────────────────────────────────
                // "get:path" → FileAppLogger::path()
                // "set:path" → FileAppLogger::setPath()
                d.readWrite("path", &FileAppLogger::path, &FileAppLogger::setPath);
            }
        );
    }

    void boot() override
    {
        qDebug() << "[ContainerPlugin] AppLoggerProvider::boot() —"
                 << "IAppLogger 绑定就绪，命名绑定 \"logger\" 已注册";
    }
};

/**
 * @brief AppStorageProvider — 存储服务提供者
 *
 * doRegister() 阶段注册：
 *   1. 类型绑定（单例）：IAppStorage → MemoryAppStorage
 *   2. 命名单例绑定："storage" → MemoryAppStorage（含反射方法表）
 */
class AppStorageProvider : public Container::ServiceProvider
{
public:
    using ServiceProvider::ServiceProvider;

    void doRegister() override
    {
        // ── 1. 类型单例绑定：IAppStorage → MemoryAppStorage ─────────────────
        app_.singleton<IAppStorage>([] {
            return std::make_shared<MemoryAppStorage>();
        });

        // ── 2. 命名单例绑定："storage" ────────────────────────────────────────
        app_.singletonNamed(
            "storage",
            [](Container::Container&) -> std::shared_ptr<void> {
                return std::make_shared<MemoryAppStorage>();
            },
            [](Container::MethodDispatcher& d) {
                // 双参数方法：store(key, value)
                d.on("store",    &MemoryAppStorage::store);

                // 单参数 const 方法
                d.on("retrieve", &MemoryAppStorage::retrieve);
                d.on("has",      &MemoryAppStorage::has);

                // 无参 const 方法
                d.on("count",    &MemoryAppStorage::count);

                // 无参方法（void）
                d.on("clear",    &MemoryAppStorage::clear);

                // 只读属性
                d.readOnly("name", &MemoryAppStorage::storageName);
            }
        );
    }

    void boot() override
    {
        qDebug() << "[ContainerPlugin] AppStorageProvider::boot() —"
                 << "IAppStorage 单例绑定就绪，命名绑定 \"storage\" 已注册";
    }
};

// =============================================================================
// Plugin Implementation（插件实现）
// =============================================================================

ContainerPlugin::ContainerPlugin()
{
}

ContainerPlugin::~ContainerPlugin() = default;

bool ContainerPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "ContainerPlugin: IMainWindowService not found.";
        qWarning() << "[ContainerPlugin] IMainWindowService not found!";
        return false;
    }

    // 使用全局单例容器：在 Application::instance() 上完成两阶段引导，
    // 这样 Facade<T> 才能通过 Application::instance() 访问到绑定。
    app_ = Container::Application::instance();

    // ── 1. 注册 ServiceProvider 并执行两阶段引导 ──────────────────────────
    //   第一阶段：registerProviders() → 依次调用每个 Provider 的 doRegister()
    //   第二阶段：bootProviders()     → 依次调用每个 Provider 的 boot()
    app_->registerProvider<AppLoggerProvider>()
        .registerProvider<AppStorageProvider>();

    // 如果之前已引导，则 bootstrap() 内部有幂等保护
    app_->bootstrap();

    qDebug() << "[ContainerPlugin] Container::Application bootstrapped."
             << "已注册绑定：IAppLogger(transient), IAppStorage(singleton),"
             << "\"logger\"(named singleton), \"storage\"(named singleton)";

    // ── 3. 注册带二级导航的容器页（order=80，排在 Invoker 之后）────────────
    PageWithSubNav *page = svc->addContainerPage("container", "nav.container", 80);

    // ── 4. 添加子页面 ────────────────────────────────────────────────────
    // 子页面①：类型绑定演示（bind/singleton/instance/extend/contextual/tag）
    page->addSubPage("container_type",  "sub.container_type",
                     new ContainerTypeView());

    // 子页面②：命名绑定演示（singletonNamed/makeNamed/call/get/set）
    page->addSubPage("container_named", "sub.container_named",
                     new ContainerNamedView(app_));

    qDebug() << "[ContainerPlugin] initialize: Container demo pages registered.";
    return true;
}
