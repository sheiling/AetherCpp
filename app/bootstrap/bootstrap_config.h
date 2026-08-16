#pragma once

#include <QString>
#include <QStringList>

struct ConfigModuleOptions {
    bool enabled = true;
    QString configFilePath = QStringLiteral("config/app.toml");
    bool autoReload = false;
    int autoReloadIntervalMs = 2000;
};

struct LoggerModuleOptions {
    bool enabled = true;
    QString appName;
    QString logDir = QStringLiteral("logs");
};

struct OrmModuleOptions {
    bool enabled = false;
};

struct DeviceManagerModuleOptions {
    /// ConfigModule 未启用或未配置 deviceManager.enabled 时的回退值。
    bool enabled = false;
    /// 设备清单 JSON（相对 exe 目录）；可被 app.toml [deviceManager] deviceConfigPath 覆盖。
    QString deviceConfigPath = QStringLiteral("config/device_config.json");
};

struct ButtonMonitorModuleOptions {
    bool enabled = false;
    int pollIntervalMs = 20;
    QString startBtn = QStringLiteral("启动按钮");
    QString stopBtn = QStringLiteral("停止按钮");
    QString resetBtn = QStringLiteral("复位按钮");
    /// 手自动切换 DI；空则跳过
    QString modeSwitch = QStringLiteral("手动");
    QString estop = QStringLiteral("急停按钮");
};

struct SchedulerModuleOptions {
    bool enabled = false;
    /// 流程 JSON 目录（相对 exe）；可被 app.toml [scheduler] flowsDir 覆盖
    QString flowsDir = QStringLiteral("config/flows");
    /// 默认产线流程名（持续运行 / 主页启动绑定；须与 flows 目录内 JSON 的 name 一致）
    QString flowsName = QStringLiteral("machine_normally");
    /// 整机复位工步流名（须与 flows 目录内 JSON 的 name 一致，如 machine_reset）
    QString resetFlowName = QStringLiteral("machine_reset");
    /// 整机急停工步流名（须与 flows 目录内 JSON 的 name 一致，如 machine_estop）
    QString estopFlowName = QStringLiteral("machine_estop");
    int maxConcurrent = 1;
    int continuousIntervalMs = 0;
    bool continuousResubmitOnFail = false;
    ButtonMonitorModuleOptions buttonMonitor;
};

struct SplashScreenOptions {
    bool enabled = true;
    /// 启动图路径；空则依次尝试 exe/config/splash.* 与内置 app_splash.ico。
    /// 相对路径相对于可执行文件所在目录解析。
    /// TOML：[splash] enabled / imagePath（见 app.toml）
    QString imagePath;
};

/// 应用级启动参数（与 Qt / 插件系统解耦的配置入口）。
struct BootstrapConfig {
    /// 应用显示名；默认由 main 设置，loadBootstrapAppConfig 成功后由 app.toml [app] name 覆盖。
    QString applicationName;
    /// 应用版本；loadBootstrapAppConfig 成功后由 app.toml [app] version 覆盖。
    QString applicationVersion;
    /// 插件 IID，须与各插件 Q_PLUGIN_METADATA 一致；空则跳过插件 IID 设置。
    QString pluginIid;
    /// 插件搜索目录；空列表表示使用默认：可执行目录下 plugins/
    QStringList pluginSearchPaths;
    /// 是否在加载失败时继续启动（仅记录警告）；false 时若有插件错误则 run() 返回非 0。
    bool continueOnPluginErrors = true;

    ConfigModuleOptions config;
    LoggerModuleOptions logger;
    OrmModuleOptions orm;
    DeviceManagerModuleOptions deviceManager;
    SchedulerModuleOptions scheduler;
    SplashScreenOptions splash;
};
