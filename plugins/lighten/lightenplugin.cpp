/**
 * @file lightenplugin.cpp
 * @brief 点亮插件初始化：注册页面与 cam_async 生命周期管理
 */

#include "lightenplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginspec.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "lighten_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"
#include "repository/datastore.h"
#include "repository/storageconfig.h"
#include "service/lighten_inspection_history_service_impl.h"
#include "service/lighten_production_kpi_service_impl.h"
#include <common/core/device/camAsync/cam_async_api.h>
#include "ui/view/home_view.h"
#include "ui/view/system_config_view.h"
#include "ui/view/recipe_config_view.h"
#include "ui/view/inspection_results_view.h"
#include "ui/view/virtual_pixel_view.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

using namespace ExtensionSystem;

namespace {

/** 是否启用 MySQL 存储（false 时使用 SQLite） */
constexpr bool kUseMysqlStorage = false;

} // namespace

/** @brief 初始化检测历史 DataStore 并向主窗口注册各配置页面 */
bool LightenPlugin::initialize(QString* errorString)
{
    lighten::setPluginName(pluginSpec()->name());
    const PermCatalogScope catalogScope(lighten::pluginName());

    if (kUseMysqlStorage) {
        lighten::DataStore::configure(lighten::StorageConfig::mysql(
            QStringLiteral("ess_lighten"), QStringLiteral("127.0.0.1"), 3306,
            QStringLiteral("root"), QStringLiteral("123456")));
    } else {
        const QString dbPath = QDir(QCoreApplication::applicationDirPath())
                                   .filePath(QStringLiteral("data/lighten_inspection.db"));
        lighten::DataStore::configure(lighten::StorageConfig::sqlite(QDir::cleanPath(dbPath)));
    }

    QString storageErr;
    if (!lighten::DataStore::initialize(&storageErr)) {
        qWarning() << "[LightenPlugin] Inspection storage init failed, fallback to Mock:"
                   << storageErr;
        lighten::DataStore::configure(lighten::StorageConfig::mock());
        if (!lighten::DataStore::initialize(&storageErr)) {
            qWarning() << "[LightenPlugin] Mock storage init failed:" << storageErr;
            if (errorString)
                *errorString = storageErr;
            return false;
        }
    }

    const auto &activeCfg = lighten::DataStore::currentConfig();
    qDebug() << "[LightenPlugin] Inspection storage ready, backend:"
             << lighten::storageBackendName(activeCfg.backend);

    m_inspectionHistoryService = new LightenInspectionHistoryServiceImpl();
    m_productionKpiService     = new LightenProductionKpiServiceImpl();
    PluginManager::addObject(m_inspectionHistoryService);
    PluginManager::addObject(m_productionKpiService);
    qDebug() << "[LightenPlugin] ILightenInspectionHistoryService and ILightenProductionKpiService registered.";

    IMainWindowService* svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "LightenPlugin: IMainWindowService not found. "
            "Make sure CorePlugin is loaded first.";
        qWarning() << "[LightenPlugin] IMainWindowService not found!";
        return false;
    }

    svc->addPage("lighten.home", "nav.lighten.home", new lighten::HomeView(), 20);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.home"), TR("nav.lighten.home"), 20);
    svc->addPage("lighten.system_config", "nav.lighten.system", new lighten::SystemConfigView(), 21);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.system"), TR("nav.lighten.system"), 21);
    PermService::instance()->registerAction(QStringLiteral("systemConfig.save"),
                                            QStringLiteral("nav.lighten.system"),
                                            QStringLiteral("更改参数"));
    PermService::instance()->registerAction(QStringLiteral("systemConfig.reset"),
                                            QStringLiteral("nav.lighten.system"),
                                            QStringLiteral("恢复默认"));
    PermService::instance()->registerAction(QStringLiteral("systemConfig.clearUseCount"),
                                            QStringLiteral("nav.lighten.system"),
                                            QStringLiteral("使用次数清零"));
    svc->addPage("lighten.recipe_config", "nav.lighten.recipe", new lighten::RecipeConfigView(), 22);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.recipe"), TR("nav.lighten.recipe"), 22);
    PermService::instance()->registerAction(QStringLiteral("recipe.save"),
                                            QStringLiteral("nav.lighten.recipe"),
                                            QStringLiteral("保存配方"));
    PermService::instance()->registerAction(QStringLiteral("recipe.setDefault"),
                                            QStringLiteral("nav.lighten.recipe"),
                                            QStringLiteral("设为默认配方"));
    svc->addPage("lighten.inspection_results", "nav.lighten.results", new lighten::InspectionResultsView(), 23);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.results"), TR("nav.lighten.results"), 23);
    svc->addPage("lighten.virtual_pixel", "nav.lighten.virtualpx", new lighten::VirtualPixelView(), 24);
    PermService::instance()->registerNav(QStringLiteral("nav.lighten.virtualpx"), TR("nav.lighten.virtualpx"), 24);
    PermService::instance()->registerAction(QStringLiteral("virtualPixel.save"),
                                            QStringLiteral("nav.lighten.virtualpx"),
                                            QStringLiteral("保存虚拟像素"));

    qDebug() << "[LightenPlugin] 5 top-level pages registered.";
    return true;
}

/** @brief 延迟初始化 cam_async 相机异步模块 */
bool LightenPlugin::delayedInitialize()
{
    if (!cam_async::Init()) {
        qWarning() << "[LightenPlugin] cam_async::Init failed.";
        return false;
    }

    qDebug() << "[LightenPlugin] cam_async initialized.";
    return false;
}

/** @brief 关闭检测历史 DataStore 并释放 cam_async */
ExtensionSystem::IPlugin::ShutdownFlag LightenPlugin::aboutToShutdown()
{
    if (m_inspectionHistoryService) {
        PluginManager::removeObject(m_inspectionHistoryService);
        delete m_inspectionHistoryService;
        m_inspectionHistoryService = nullptr;
    }
    if (m_productionKpiService) {
        PluginManager::removeObject(m_productionKpiService);
        delete m_productionKpiService;
        m_productionKpiService = nullptr;
    }
    lighten::DataStore::shutdown();
    cam_async::Exit();
    return ExtensionSystem::IPlugin::SynchronousShutdown;
}
