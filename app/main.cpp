#include "bootstrap/application_bootstrap.h"
#include "bootstrap/bootstrap_config.h"
#include "bootstrap/core_modules.h"
#include "extensionsystem/extensionsystem_global.h"

#include <QLatin1String>
#include <QString>
#include <QDebug>
#include <memory>

int main(int argc, char *argv[])
{
    BootstrapConfig cfg;
    // 配置加载失败或未启用 [app] 时的回退值
    cfg.applicationName = QStringLiteral("PluginApp");
    cfg.applicationVersion = QStringLiteral("1.0.0");
    cfg.pluginIid = QLatin1String(EXTENSIONSYSTEM_IID);

    if (cfg.config.enabled && !loadBootstrapAppConfig(cfg, argc > 0 ? argv[0] : nullptr)) {
        qWarning() << "[bootstrap] config load failed, using BootstrapConfig defaults";
    }

    cfg.logger.appName = cfg.applicationName;

    ApplicationBootstrap bootstrap(cfg);
    registerDefaultCoreModules(bootstrap, cfg);

    const int exitCode = bootstrap.run(argc, argv);

    shutdownBootstrapAppConfig(cfg);
    return exitCode;
}
