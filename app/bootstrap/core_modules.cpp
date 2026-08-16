#include "core_modules.h"

#include "application_bootstrap.h"
#include "bootstrap_context.h"
#include "plugin_loader_module.h"
#include "splash_screen_module.h"

#include <Config.h>
#include <Logger.h>
#if COMMON_ORM_ENABLED
#include <Orm.h>
#endif
#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/device/motionControl/motion_controller.h>
#include <extensionsystem/pluginmanager.h>
#include <scheduler/api/scheduler_runtime.h>
#include <scheduler/api/scheduler_service_impl.h>

#include <QDebug>
#include <QString>

#include <cstdio>
#include <filesystem>

namespace {

std::filesystem::path resolveExecutableDir(const char *argv0)
{
    std::filesystem::path exeDir;
    if (argv0 != nullptr && argv0[0] != '\0') {
        exeDir = std::filesystem::path(argv0).parent_path();
    }
    if (exeDir.empty()) {
        exeDir = std::filesystem::current_path();
    }
    return exeDir;
}

/// 绝对路径原样返回；相对路径拼到可执行文件所在目录（无 argv0 时用当前工作目录）。
std::string resolveRuntimeRelativePath(const std::string &path, const char *argv0)
{
    if (path.empty())
        return path;

    std::filesystem::path resolved(path);
    if (resolved.is_absolute())
        return resolved.lexically_normal().string();

    return (resolveExecutableDir(argv0) / resolved).lexically_normal().string();
}

void applyBootstrapAppMetadataFromToml(BootstrapConfig &cfg)
{
    if (!config::ConfigManager::isLoaded())
        return;

    const std::string name = config::ConfigManager::get<std::string>(
        "app.name", cfg.applicationName.toStdString());
    if (!name.empty())
        cfg.applicationName = QString::fromStdString(name);

    const std::string version = config::ConfigManager::get<std::string>(
        "app.version", cfg.applicationVersion.toStdString());
    if (!version.empty())
        cfg.applicationVersion = QString::fromStdString(version);
}

std::string resolveBootstrapConfigFilePath(const BootstrapConfig &cfg, const char *argv0)
{
    const std::string relative = cfg.config.configFilePath.toStdString();
    if (relative.empty())
        return {};
    return resolveRuntimeRelativePath(relative, argv0);
}

bool moduleEnabledFromToml(const char *key, bool fallback)
{
    if (config::ConfigManager::isLoaded())
        return config::ConfigManager::get<bool>(key, fallback);
    return fallback;
}

#if COMMON_ORM_ENABLED
orm::DbDriver parseDbDriver(const std::string &name)
{
    const QString driver = QString::fromStdString(name).trimmed().toUpper();
    if (driver == "QMYSQL" || driver == "MYSQL")
        return orm::DbDriver::MySQL;
    if (driver == "QPSQL" || driver == "POSTGRESQL" || driver == "POSTGRES")
        return orm::DbDriver::PostgreSQL;
    if (driver == "QODBC" || driver == "ODBC")
        return orm::DbDriver::ODBC;
    if (driver == "QSQLITE" || driver == "SQLITE")
        return orm::DbDriver::SQLite;
    return orm::DbDriver::Custom;
}
#endif

void fillButtonMonitorOptionsFromToml(SchedulerRuntime::ButtonMonitorOptions &bm,
                                      const ButtonMonitorModuleOptions &defaults)
{
    bm.enabled = defaults.enabled;
    bm.pollIntervalMs = defaults.pollIntervalMs;
    bm.startBtn = defaults.startBtn.toStdString();
    bm.stopBtn = defaults.stopBtn.toStdString();
    bm.resetBtn = defaults.resetBtn.toStdString();
    bm.modeSwitch = defaults.modeSwitch.toStdString();
    bm.estop = defaults.estop.toStdString();

    if (!config::ConfigManager::isLoaded())
        return;

    bm.enabled = config::ConfigManager::get<bool>(
        "scheduler.buttonMonitor.enabled", bm.enabled);
    bm.pollIntervalMs = config::ConfigManager::get<int>(
        "scheduler.buttonMonitor.pollIntervalMs", bm.pollIntervalMs);
    bm.startBtn = config::ConfigManager::get<std::string>(
        "scheduler.buttonMonitor.startBtn", bm.startBtn);
    bm.stopBtn = config::ConfigManager::get<std::string>(
        "scheduler.buttonMonitor.stopBtn", bm.stopBtn);
    bm.resetBtn = config::ConfigManager::get<std::string>(
        "scheduler.buttonMonitor.resetBtn", bm.resetBtn);
    bm.modeSwitch = config::ConfigManager::get<std::string>(
        "scheduler.buttonMonitor.modeSwitch", bm.modeSwitch);
    bm.estop = config::ConfigManager::get<std::string>(
        "scheduler.buttonMonitor.estop", bm.estop);

    const bool virtualMotion = config::ConfigManager::get<bool>(
        "virtual_debug.virtual_motion_enable", false);
    bm.physicalButtonsEnabled = !virtualMotion;
}

scheduler::ButtonMonitor::IoReader makeDeviceManagerIoReader()
{
    std::shared_ptr<common::device::MotionController> motion;
    try {
        auto manager = Container::Application::instance()->make<common::device::DeviceManager>();
        if (manager)
            motion = manager->MotionHandler();
    } catch (const std::exception &e) {
        LOG_WARN("[bootstrap] DeviceManager unavailable for ButtonMonitor: {}", e.what());
    }

    if (!motion) {
        LOG_WARN("[bootstrap] MotionController unavailable, ButtonMonitor IoReader disabled");
        return {};
    }

    return [motion = std::move(motion)](const std::string &ioName, bool &value) -> bool {
        return motion->ReadIO(ioName, value) == nullptr;
    };
}

} // namespace

namespace {

std::shared_ptr<SchedulerRuntime>     g_schedulerRuntime;
std::unique_ptr<SchedulerServiceImpl> g_schedulerService;

} // namespace


QString LoggerModule::name() const { return QStringLiteral("Logger"); }
int LoggerModule::priority() const { return 5; }
bool LoggerModule::onRegister(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("logger.enabled", ctx.config->logger.enabled))
        return true;

    logger::LogConfig cfg;
    const std::string defaultAppName = ctx.config->logger.appName.isEmpty()
                                           ? ctx.config->applicationName.toStdString()
                                           : ctx.config->logger.appName.toStdString();
    cfg.appName = config::ConfigManager::get<std::string>("logger.appName", defaultAppName);
    if (cfg.appName.empty())
        cfg.appName = ctx.config->applicationName.toStdString();
    if (cfg.appName.empty())
        cfg.appName = "app";
    cfg.logDir = resolveRuntimeRelativePath(
        config::ConfigManager::get<std::string>("logger.logDir", cfg.logDir),
        (ctx.argv != nullptr && ctx.argc != nullptr && *ctx.argc > 0) ? ctx.argv[0] : nullptr);
    cfg.pattern = config::ConfigManager::get<std::string>("logger.pattern", cfg.pattern);

    cfg.level = static_cast<logger::LogLevel>(
        config::ConfigManager::get<int>("logger.level", static_cast<int>(cfg.level)));
    cfg.target = static_cast<logger::OutputTarget>(
        config::ConfigManager::get<int>("logger.target", static_cast<int>(cfg.target)));
    cfg.enableColor = config::ConfigManager::get<bool>("logger.enableColor", cfg.enableColor);
    cfg.rotateStrategy = static_cast<logger::RotateStrategy>(
        config::ConfigManager::get<int>("logger.rotateStrategy", static_cast<int>(cfg.rotateStrategy)));

    cfg.maxFileSize = static_cast<std::size_t>(config::ConfigManager::get<int64_t>(
        "logger.maxFileSize", static_cast<int64_t>(cfg.maxFileSize)));
    cfg.maxRotatedFiles = static_cast<std::size_t>(config::ConfigManager::get<int64_t>(
        "logger.maxRotatedFiles", static_cast<int64_t>(cfg.maxRotatedFiles)));
    cfg.rotationHour = config::ConfigManager::get<int>("logger.rotationHour", cfg.rotationHour);
    cfg.rotationMinute = config::ConfigManager::get<int>("logger.rotationMinute", cfg.rotationMinute);
    cfg.maxDailyFiles = static_cast<uint16_t>(
        config::ConfigManager::get<int>("logger.maxDailyFiles", static_cast<int>(cfg.maxDailyFiles)));
    cfg.flushMode = static_cast<logger::FlushMode>(
        config::ConfigManager::get<int>("logger.flushMode", static_cast<int>(cfg.flushMode)));

    if (!logger::Logger::init(cfg)) {
        qWarning() << "[bootstrap] Logger init failed";
        return false;
    }
    LOG_INFO("Logger initialized from bootstrap");
    return true;
}
void LoggerModule::onShutdown(BootstrapContext &ctx){
    if (moduleEnabledFromToml("logger.enabled", ctx.config->logger.enabled))
        logger::Logger::shutdown();
}


#if COMMON_ORM_ENABLED
QString OrmModule::name() const { return QStringLiteral("Orm"); }
int OrmModule::priority() const { return 20; }
bool OrmModule::onRegister(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("orm.enabled", ctx.config->orm.enabled))
        return true;

    orm::DbConfig cfg;
    cfg.driverName = QString::fromStdString(
        config::ConfigManager::get<std::string>("orm.driverName", cfg.driverName.toStdString()));
    if (!cfg.driverName.isEmpty())
        cfg.driver = parseDbDriver(cfg.driverName.toStdString());

    cfg.database = QString::fromStdString(
        config::ConfigManager::get<std::string>("orm.database", cfg.database.toStdString()));
    cfg.host = QString::fromStdString(
        config::ConfigManager::get<std::string>("orm.host", cfg.host.toStdString()));
    cfg.port = config::ConfigManager::get<int>("orm.port", cfg.port);
    cfg.username = QString::fromStdString(
        config::ConfigManager::get<std::string>("orm.username", cfg.username.toStdString()));
    cfg.password = QString::fromStdString(
        config::ConfigManager::get<std::string>("orm.password", cfg.password.toStdString()));
    cfg.connectOptions = QString::fromStdString(config::ConfigManager::get<std::string>(
        "orm.connectOptions", cfg.connectOptions.toStdString()));
    cfg.traceSql = config::ConfigManager::get<bool>("orm.traceSql", cfg.traceSql);
    cfg.traceSqlError = config::ConfigManager::get<bool>("orm.traceSqlError", cfg.traceSqlError);

    if (!orm::DbManager::instance().init(cfg)) {
        qWarning() << "[bootstrap] orm init failed:" << orm::DbManager::instance().lastError();
        return false;
    }

    qDebug() << "[bootstrap] orm initialized:" << orm::DbManager::instance().config().toString();
    return true;
}
void OrmModule::onShutdown(BootstrapContext &ctx){
    Q_UNUSED(ctx);
    orm::DbManager::instance().closeAll();
}
#endif


QString DeviceManagerModule::name() const { return QStringLiteral("DeviceManager"); }
int DeviceManagerModule::priority() const { return 40; }
bool DeviceManagerModule::onRegister(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("deviceManager.enabled", ctx.config->deviceManager.enabled))
        return true;

    auto *app = Container::Application::instance();
    if (!app->bound<common::device::DeviceManager>()) {
        app->singleton<common::device::DeviceManager>(
            [] { return std::make_shared<common::device::DeviceManager>(); });
    }

    try {
        auto manager = app->make<common::device::DeviceManager>();
        std::string configPath = ctx.config->deviceManager.deviceConfigPath.toStdString();
        
        if (config::ConfigManager::isLoaded())
        {
            configPath = config::ConfigManager::get<std::string>(
                "deviceManager.deviceConfigPath", configPath);
        }

        if (!manager->loadDeviceConfig(configPath)) {
            LOG_WARN("[bootstrap] DeviceManager 设备配置加载失败，路径: {}", configPath);
        }
    } catch (const std::exception &e) {
        LOG_ERROR("[bootstrap] DeviceManager 注册失败: {}", e.what());
        return false;
    }

    return true;
}
bool DeviceManagerModule::onBoot(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("deviceManager.enabled", ctx.config->deviceManager.enabled))
        return true;

    try {
        auto manager = Container::Application::instance()->make<common::device::DeviceManager>();
        const int rc = manager->Init();
        if (rc != 0) {
            LOG_ERROR("[bootstrap] DeviceManager 初始化失败，错误码={}", rc);
            manager.reset();
            return false;
        }
    } catch (const std::exception &e) {
        LOG_ERROR("[bootstrap] DeviceManager 解析失败: {}", e.what());
        return false;
    }

    LOG_INFO("[bootstrap] DeviceManager 初始化完成");
    return true;
}
void DeviceManagerModule::onShutdown(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("deviceManager.enabled", ctx.config->deviceManager.enabled))
        return;

    try {
        auto manager = Container::Application::instance()->make<common::device::DeviceManager>();
        if (manager)
            manager->Release();
    } catch (const std::exception &e) {
        LOG_WARN("[bootstrap] DeviceManager 关闭时解析失败: {}", e.what());
    }
}


QString SchedulerModule::name() const { return QStringLiteral("Scheduler"); }
int SchedulerModule::priority() const { return 50; }
bool SchedulerModule::onRegister(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("scheduler.enabled", ctx.config->scheduler.enabled))
        return true;

    auto *app = Container::Application::instance();
    if (!g_schedulerRuntime)
        g_schedulerRuntime = std::make_shared<SchedulerRuntime>();

    if (!app->bound<SchedulerRuntime>()) {
        app->singleton<SchedulerRuntime>([] {
            return g_schedulerRuntime;
        });
    }

    return true;
}
bool SchedulerModule::onBoot(BootstrapContext &ctx){
    if (!moduleEnabledFromToml("scheduler.enabled", ctx.config->scheduler.enabled))
        return true;

    if (!g_schedulerRuntime) {
        LOG_ERROR("[bootstrap] SchedulerRuntime 未创建");
        return false;
    }

    SchedulerRuntime::Options options;
    options.flowsName = ctx.config->scheduler.flowsName.toStdString();
    options.resetFlowName = ctx.config->scheduler.resetFlowName.toStdString();
    options.estopFlowName = ctx.config->scheduler.estopFlowName.toStdString();
    options.maxConcurrent = ctx.config->scheduler.maxConcurrent;
    options.continuousIntervalMs = ctx.config->scheduler.continuousIntervalMs;
    options.continuousResubmitOnFail = ctx.config->scheduler.continuousResubmitOnFail;

    std::string flowsDir = ctx.config->scheduler.flowsDir.toStdString();
    if (config::ConfigManager::isLoaded()) {
        flowsDir = config::ConfigManager::get<std::string>("scheduler.flowsDir", flowsDir);
        options.flowsName = config::ConfigManager::get<std::string>(
            "scheduler.flowsName", options.flowsName);
        options.resetFlowName = config::ConfigManager::get<std::string>(
            "scheduler.resetFlowName", options.resetFlowName);
        options.estopFlowName = config::ConfigManager::get<std::string>(
            "scheduler.estopFlowName", options.estopFlowName);
        options.maxConcurrent = config::ConfigManager::get<int>(
            "scheduler.maxConcurrent", options.maxConcurrent);
        options.continuousIntervalMs = config::ConfigManager::get<int>(
            "scheduler.continuousIntervalMs", options.continuousIntervalMs);
        options.continuousResubmitOnFail = config::ConfigManager::get<bool>(
            "scheduler.continuousResubmitOnFail", options.continuousResubmitOnFail);
    }
    options.flowsDir = resolveRuntimeRelativePath(
        flowsDir,
        (ctx.argv != nullptr && ctx.argc != nullptr && *ctx.argc > 0) ? ctx.argv[0] : nullptr);

    fillButtonMonitorOptionsFromToml(options.buttonMonitor, ctx.config->scheduler.buttonMonitor);

    if (!g_schedulerRuntime->init(options)) {
        LOG_ERROR("[bootstrap] SchedulerRuntime 初始化失败");
        return false;
    }

    if (!g_schedulerService)
        g_schedulerService = std::make_unique<SchedulerServiceImpl>(g_schedulerRuntime);
    ExtensionSystem::PluginManager::addObject(g_schedulerService.get());

    const bool deviceMgrEnabled =
        moduleEnabledFromToml("deviceManager.enabled", ctx.config->deviceManager.enabled);
    if (options.buttonMonitor.enabled) {
        if (!deviceMgrEnabled) {
            LOG_WARN("[bootstrap] 已启用物理按钮监控，但 deviceManager 未启用，跳过 ButtonMonitor");
        } else {
            auto reader = makeDeviceManagerIoReader();
            if (!g_schedulerRuntime->startButtonMonitor(std::move(reader))) {
                LOG_WARN("[bootstrap] 物理按钮监控（ButtonMonitor）启动失败");
            }
        }
    }

    LOG_INFO("[bootstrap] 调度引擎就绪，flowsDir={}，flowsName={}，resetFlowName={}，"
             "estopFlowName={}，buttonMonitor={}，physicalButtons={}（流程将在插件加载后注册）",
             options.flowsDir, options.flowsName, options.resetFlowName, options.estopFlowName,
             options.buttonMonitor.enabled, options.buttonMonitor.physicalButtonsEnabled);
    return true;
}
void SchedulerModule::onShutdown(BootstrapContext &ctx){
    Q_UNUSED(ctx);
    if (!moduleEnabledFromToml("scheduler.enabled", ctx.config->scheduler.enabled))
        return;

    if (g_schedulerRuntime)
        g_schedulerRuntime->shutdown();

    if (g_schedulerService) {
        //ExtensionSystem::PluginManager::removeObject(g_schedulerService.get());
        g_schedulerService.reset();
    }
}


bool loadBootstrapAppConfig(BootstrapConfig &cfg, const char *argv0){
    if (!cfg.config.enabled)
        return true;

    const std::string path = resolveBootstrapConfigFilePath(cfg, argv0);
    if (path.empty()) {
        std::fprintf(stderr, "[bootstrap] config file path is empty\n");
        return false;
    }

    if (!config::ConfigManager::load(path)) {
        std::fprintf(stderr, "[bootstrap] config load failed: %s\n", path.c_str());
        return false;
    }

    applyBootstrapAppMetadataFromToml(cfg);

    if (cfg.config.autoReload)
        config::ConfigManager::setAutoReload(true, cfg.config.autoReloadIntervalMs);

    return true;
}

void shutdownBootstrapAppConfig(const BootstrapConfig &cfg){
    if (cfg.config.enabled && cfg.config.autoReload)
        config::ConfigManager::setAutoReload(false);
}

void registerDefaultCoreModules(ApplicationBootstrap &bootstrap, const BootstrapConfig &config){
    if (moduleEnabledFromToml("logger.enabled", config.logger.enabled))
        bootstrap.addModule(std::make_unique<LoggerModule>());
#if COMMON_ORM_ENABLED
    if (moduleEnabledFromToml("orm.enabled", config.orm.enabled))
        bootstrap.addModule(std::make_unique<OrmModule>());
#endif
    if (moduleEnabledFromToml("deviceManager.enabled", config.deviceManager.enabled))
        bootstrap.addModule(std::make_unique<DeviceManagerModule>());
    if (moduleEnabledFromToml("scheduler.enabled", config.scheduler.enabled))
        bootstrap.addModule(std::make_unique<SchedulerModule>());

    bootstrap.addModule(std::make_unique<PluginLoaderModule>());
    
    // 须在 PluginLoader 之后注册，确保关闭时先释放 SplashScreen（Detach 依赖 MessageCenter）。
    if (moduleEnabledFromToml("splash.enabled", config.splash.enabled))
        bootstrap.addModule(std::make_unique<SplashScreenModule>());
}

void loadSchedulerFlowsAfterPlugins(){
    if (!g_schedulerRuntime || !g_schedulerRuntime->isReady())
        return;

    if (!g_schedulerRuntime->loadFlowDefinitions())
        return;

    if (g_schedulerService)
        g_schedulerService->notifyFlowsLoaded();
}
