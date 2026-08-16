/**
 * @file aoiplugin.cpp
 * @brief AOI (Automated Optical Inspection) 插件实现
 *
 * AOI 固晶检测插件，向主窗口注册六个顶级页面，提供完整的
 * 检测工作流支持：首页仪表盘、产品配方配置、检测结果浏览、
 * 缺陷复判、系统配置及流程宿主绑定。
 *
 * 架构上遵循 MVVM 模式，通过 IMainWindowService 注册页面，
 * 通过 ISchedulerService 绑定检测流程生命周期。
 *
 * ## 注册页面列表
 * | 页面 | 导航 Key | Order | 说明 |
 * |------|----------|-------|------|
 * | 首页 | aoi.nav.home | 20 | 设备状态、流程控制、虚拟地图 |
 * | 系统配置 | aoi.nav.system | 21 | 二级导航：开关/通用/阈值 |
 * | 产品配方 | aoi.nav.recipe | 22 | 配方管理、参数配置 |
 * | 检测结果 | aoi.nav.results | 23 | 检测历史查询 |
 * | 缺陷复判 | aoi.nav.review | 24 | 缺陷分类、复判操作 |
 *
 * ## 初始化阶段
 * 1. initialize()  — 注册页面、主题钩子、翻译文件
 * 2. extensionsInitialized() — 绑定流程宿主到调度服务
 * 3. delayedInitialize() — 预热检测引擎、检查 VisionAlgo 就绪
 *
 * @see AoiPlugin
 * @see IMainWindowService
 * @see ISchedulerService
 */

#include "aoiplugin.h"

#include "service/inspection/detect/inspection_detect_service.h"

#include "service/flow/scan_flow_lifecycle.h"

#include <flow_host/flow_host_coordinator.h>
#include <scheduler/api/ischeduler_service.h>

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "aoi_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"

#include "ui/view/home/home_view.h"
#include "ui/view/inspection_history/inspection_history_view.h"
#include "ui/view/review/defect_review_view.h"
#include "widgets/theme/aoi_theme_bootstrap.h"
#include "recipe_config/recipe_config_view.h"
#include "ui/view/system_config/system_config_switch_view.h"
#include "ui/view/system_config/system_config_general_view.h"
#include "ui/view/system_config/system_config_threshold_view.h"
#include "ui/view/system_config/system_config_view.h"
#include "ui/vm/system_config_view_model.h"

#include "component/dialog/themed_message_box.h"

#include <vision_algo/core/service_access.h>

#include <Logger.h>

#include <QApplication>
#include <QDebug>
#include <QTimer>

using namespace ExtensionSystem;

namespace {

void warnVisionAlgoUnavailableAtStartup()
{
    const QString detail = vision_algo::startupIssue();
    if (detail.isEmpty())
        return;

    LOG_WARN("[AoiPlugin] VisionAlgo 未就绪：{}", detail.toStdString());

    QString text = TR("aoi.vision_algo.startup_warning.summary");
    text += QStringLiteral("\n") + detail;
    ui::ThemedMessageBox::information(nullptr,
                                       TR("aoi.vision_algo.startup_warning.title"),
                                       text,
                                       QString(),
                                       ui::ThemedMessageBoxIcon::Warning);
}

} // namespace

bool AoiPlugin::initialize(QString *errorString)
{
    /**
     * @brief 插件初始化入口
     *
     * 在插件加载阶段执行以下操作：
     * 1. 注册全局主题钩子并应用 AOI 主题变量
     * 2. 注册插件翻译文件到 LanguageManager
     * 3. 通过 IMainWindowService 注册 5 个顶级页面及权限
     * 4. 系统配置以二级导航容器（SubPage）形式注册
     *
     * @param errorString 输出参数，初始化失败时填充错误描述
     * @return true 初始化成功；false 失败（如 IMainWindowService 不可用）
     */
    aoi::setPluginName(pluginSpec()->name());
    const PermCatalogScope catalogScope(aoi::pluginName());

    // 全局主题：钩子须在 refresh 前注册；启动时再完整刷新一次
    TM->registerBeforeStylesheetHook([] { aoi::applyAoiGlobalThemeVariables(); });
    aoi::applyAoiGlobalTheme();

    // 注册插件翻译文件
    LM->registerPluginTranslator(QStringLiteral("aoi"), QStringLiteral("aoi"));

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = QStringLiteral("AoiPlugin: IMainWindowService not found. "
                                           "Make sure CorePlugin is loaded first.");
        qWarning() << "[AoiPlugin] IMainWindowService not found!";
        return false;
    }

    svc->addPage(QStringLiteral("aoi.nav.home"), QStringLiteral("aoi.nav.home"), new aoi::HomeView(), 20);
    PermService::instance()->registerNav(QStringLiteral("aoi.nav.home"),
                                         QStringLiteral("aoi.nav.home"), 20);

    // 系统配置：平台二级导航容器（SecondaryNavigationManager + 全局 ACSS）
    // VM 由容器持有；三子页仅注入指针，各自绑定 config 中与本页相关的字段。
    // recipe_config_view.h 须在本 TU 中先于 system_config_view_model 间接包含，避免
    // system_config_model.h 末尾 #undef DEFINE_PARAM_TYPE_META 破坏 recipe 参数元数据宏。
    PageWithSubNav *sysCfg = svc->addContainerPage(QStringLiteral("aoi.nav.system"),
                                                   QStringLiteral("aoi.nav.system"), 21);
    PermService::instance()->registerNav(QStringLiteral("aoi.nav.system"),
                                         QStringLiteral("aoi.nav.system"), 21);
    auto *sysCfgVm = new aoi::SystemConfigViewModel(QStringLiteral("aoi.nav.system"), sysCfg);
    auto *sysSwitchView =
        new aoi::SystemConfigSwitchView(sysCfgVm, sysCfg);
    auto *sysGeneralView =
        new aoi::SystemConfigGeneralView(sysCfgVm, sysCfg);
    auto *sysThresholdView =
        new aoi::SystemConfigThresholdView(sysCfgVm, sysCfg);
    sysCfg->addSubPage(QStringLiteral("aoi.nav.system.switch"), QStringLiteral("aoi.nav.system.switch"),
                       sysSwitchView);
    PermService::instance()->registerSubPage(QStringLiteral("aoi.nav.system.switch"),
                                             QStringLiteral("aoi.nav.system"),
                                             QStringLiteral("aoi.nav.system.switch"), 1);
    sysCfg->addSubPage(QStringLiteral("aoi.nav.system.general"), QStringLiteral("aoi.nav.system.general"),
                       sysGeneralView);
    PermService::instance()->registerSubPage(QStringLiteral("aoi.nav.system.general"),
                                             QStringLiteral("aoi.nav.system"),
                                             QStringLiteral("aoi.nav.system.general"), 2);
    sysCfg->addSubPage(QStringLiteral("aoi.nav.system.threshold"), QStringLiteral("aoi.nav.system.threshold"),
                       sysThresholdView);
    PermService::instance()->registerSubPage(QStringLiteral("aoi.nav.system.threshold"),
                                             QStringLiteral("aoi.nav.system"),
                                             QStringLiteral("aoi.nav.system.threshold"), 3);
    aoi::mountSystemConfigSaveBar(sysCfg, sysCfgVm, sysSwitchView, sysGeneralView, sysThresholdView);

    svc->addPage(QStringLiteral("aoi.nav.recipe"),      QStringLiteral("aoi.nav.recipe"),
                 new aoi::RecipeConfigView(),      22);
    PermService::instance()->registerNav(QStringLiteral("aoi.nav.recipe"),
                                         QStringLiteral("aoi.nav.recipe"), 22);

    svc->addPage(QStringLiteral("aoi.nav.results"), QStringLiteral("aoi.nav.results"),
                 new aoi::InspectionHistoryView(), 23);
    PermService::instance()->registerNav(QStringLiteral("aoi.nav.results"),
                                         QStringLiteral("aoi.nav.results"), 23);

    svc->addPage(QStringLiteral("aoi.nav.review"), QStringLiteral("aoi.nav.review"),
                 new aoi::review::DefectReviewView(), 24);
    PermService::instance()->registerNav(QStringLiteral("aoi.nav.review"),
                                         QStringLiteral("aoi.nav.review"), 24);

    qDebug() << "[AoiPlugin] 5 top-level pages registered (order 20-24).";

    return true;
}

void AoiPlugin::extensionsInitialized()
{
    /**
     * @brief 所有插件初始化完成后调用
     *
     * 此时所有插件均已加载完毕，可以安全访问其他插件的服务。
     * 本阶段将 AOI 检测流程宿主绑定到调度服务。
     *
     * - 获取 ISchedulerService 接口
     * - 创建/获取 flowHostCoordinator 单例
     * - 绑定 AOI 扫描流程生命周期钩子
     * - 将流程宿主附加到调度服务
     */
    if (auto *sched = PluginManager::getInterface<ISchedulerService>()) {
        auto &host = aoi::flow_host::flowHostCoordinator();
        aoi::flow::bindAoiScanFlowHost(host);
        if (host.attachToSchedulerService(sched))
            LOG_INFO("[AoiPlugin] 流程宿主已绑定调度服务（流程加载后挂载钩子）");
        else
            LOG_WARN("[AoiPlugin] 流程宿主绑定失败");
    } else {
        LOG_INFO("[AoiPlugin] 调度服务不可用（未启用 scheduler？）");
    }
}

bool AoiPlugin::delayedInitialize()
{
    /**
     * @brief 延迟初始化（所有插件加载完成后执行）
     *
     * 在 loadPlugins 全部完成后由 PluginManager 通过 QTimer 调度。
     * 执行以下延迟任务：
     * 1. 检查 VisionAlgo 是否就绪，未就绪则弹窗警告
     * 2. 预热检测引擎 InspectionDetectService（异常时静默跳过）
     *
     * @note 不在此处用 singleShot(0) 延后，否则 pumpGuiEventsDuringPluginLoad
     *       会在插件未全部加载时触发 DL/句柄池初始化。
     * @return false 表示无需再次延迟
     */
    QTimer::singleShot(0, qApp, warnVisionAlgoUnavailableAtStartup);
    QTimer::singleShot(0, qApp, []() {
        try {
            aoi::inspection::InspectionDetectService::instance().warmUp();
        } catch (const std::exception &ex) {
            LOG_WARN("[AoiPlugin] 检测引擎预热跳过：{}", ex.what());
        } catch (...) {
            LOG_WARN("[AoiPlugin] 检测引擎预热跳过：未知异常");
        }
    });
    return false;
}
