/**
 * @file home_view_model.cpp
 * @brief `HomeViewModel` 构造、Command 注册与 MachineFSM / Scheduler 对接
 */

#include "home_view_model.h"

#include "home_hardware_status_view_model.h"
#include "vm/review/defect_review_debug_data.h"

#include "binding/fsm/home_fsm_binding.h"
#include "binding/fsm/machine_status_display.h"

#include <flow_host/flow_host_coordinator.h>
#include <service/flow/scan_flow_progress_bridge.h>

#include <scheduler/api/ischeduler_service.h>
#include <scheduler/machine/machine_fsm.h>

#include "extensionsystem/pluginmanager.h"
#include "messagecenter/message_center.h"
#include "messagecenter/message_define.h"
#include "uibase/languagemanager.h"
#include "aoi_plugin_name.h"
#include "uibase/permission/perm_catalog_scope.h"
#include "uibase/permission/permservice.h"

#include <QDateTime>
#include "image/image_info_display.h"
#include <QRectF>
#include <QVariantList>
#include <QMetaType>

#include <cmath>

namespace aoi {

namespace {

int virtualMapCellKey(int row, int col)
{
    return row * 10000 + col;
}

int normToGridIndex(qreal normStart, int count)
{
    if (count <= 0)
        return 0;
    const qreal cellSize = 1.0 / static_cast<qreal>(count);
    const int   idx      = static_cast<int>(std::floor((normStart + cellSize * 0.5) / cellSize));
    return qBound(0, idx, count - 1);
}

void markCapturedCell(PanelVirtualMapState &vm, int row, int col)
{
    if (row < 0 || col < 0 || vm.fovGridRows <= 0 || vm.fovGridCols <= 0)
        return;
    if (row >= vm.fovGridRows || col >= vm.fovGridCols)
        return;
    vm.capturedCells.insert(virtualMapCellKey(row, col));
}

void markCapturedCell(PanelVirtualMapState &vm, const QRectF &normRect)
{
    if (normRect.isNull() || vm.fovGridCols <= 0 || vm.fovGridRows <= 0)
        return;
    const int col = normToGridIndex(normRect.left(), vm.fovGridCols);
    const int row = normToGridIndex(normRect.top(), vm.fovGridRows);
    markCapturedCell(vm, row, col);
}

ISchedulerService *resolveSchedulerService()
{
    return ExtensionSystem::PluginManager::getInterface<ISchedulerService>();
}

} // namespace

HomeViewModel::HomeViewModel(QObject *parent)
    : ViewModel(parent)
    , m_inspectionBridge(std::make_unique<flow::ScanFlowProgressBridge>(this))
{
    qRegisterMetaType<HomeRuntimeState>("HomeRuntimeState");
    registerProperty(PROP_RUNTIME, QVariant::fromValue(HomeRuntimeState::defaultDemo()));
    flow_host::flowHostCoordinator().setActiveBridge(m_inspectionBridge.get());
    setupInspectionBridge();
    setupRuntimeLogBinding();
    loadDemoData();
    m_hardwareStatusVm = new HomeHardwareStatusViewModel(this, this);
    registerDeviceCommands();
}

void HomeViewModel::publishRuntimeChange(const std::function<void(HomeRuntimeState &)> &modifier)
{
    HomeRuntimeState state = runtime();
    modifier(state);
    assignProperty(PROP_RUNTIME, QVariant::fromValue(state));
}

void HomeViewModel::refreshRuntimeBindings()
{
    notifyPropertyChanged(PROP_RUNTIME);
}

HomeViewModel::~HomeViewModel()
{
    MessageCenter::Center::Detach(
        EVENT_RUNTIME_LOG, this, SLOT(onRuntimeLog(const QVariantList &)));
}

void HomeViewModel::loadDemoData()
{
    setRuntime(HomeRuntimeState::defaultDemo());
}

void HomeViewModel::refreshHardwareStatus()
{
    if (m_hardwareStatusVm)
        m_hardwareStatusVm->refresh();
}

void HomeViewModel::applyMachineStatusToKpi(const scheduler::MachineStatus &status)
{
    const QString deviceKey  = binding::fsm::deviceStateKey(status);
    const QString runModeKey = binding::fsm::runModeKey(status);
    modifyRuntime([&](HomeRuntimeState &state) {
        if (state.kpi.deviceState == deviceKey && state.kpi.runMode == runModeKey)
            return;
        state.kpi.deviceState = deviceKey;
        state.kpi.runMode     = runModeKey;
    });
}

void HomeViewModel::refreshDeviceCommands()
{
    const auto touch = [](FunctionalCommand *c) {
        if (c)
            c->updateCanExecute();
    };
    touch(m_startCommand);
    touch(m_stopCommand);
    touch(m_resetCommand);
    touch(m_clearAlarmCommand);
    touch(m_clearMaterialCommand);
    touch(m_singleInspectCommand);
}

void HomeViewModel::appendLog(const QString &message, const QString &level)
{
    modifyRuntime([&](HomeRuntimeState &state) {
        ui::LogEntry entry;
        entry.time    = QDateTime::currentDateTime();
        entry.level   = level;
        entry.message = message;
        state.logs.append(entry);
    });
}

void HomeViewModel::applyInspectionSnapshot(const CurrentInspectionSnapshot &snapshot)
{
    modifyRuntime([&](HomeRuntimeState &state) {
        state.snapshot = snapshot;
        if (snapshot.overallResult == InspectionOverallResult::Ok)
            state.kpi.okCount += 1;
        else if (snapshot.overallResult == InspectionOverallResult::Ng)
            state.kpi.ngCount += 1;
    });
}

void HomeViewModel::setupInspectionBridge()
{
    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::logAppended,
            this, &HomeViewModel::appendLog, Qt::QueuedConnection);

    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::progressUpdated,
            this,
            [this](const QString &message, int currentIndex, int totalCount,
                   const std::shared_ptr<common::device::ImageInfo> &fovImage) {
                const QImage preview = image_display::imageInfoToQImage(fovImage);
                publishRuntimeChange([&](HomeRuntimeState &state) {
                    state.activeProcessStep = HomeProcessStep::Inspect;
                    if (!preview.isNull())
                        state.preview.currentFov = preview;
                    if (currentIndex > 0 && totalCount > 0) {
                        state.preview.fovFocusRect = QRectF(
                            0.0, 0.0,
                            static_cast<qreal>(currentIndex) / static_cast<qreal>(totalCount),
                            1.0);
                    }

                    PanelVirtualMapState &vm = state.virtualMap;
                    if (totalCount > 0)
                        vm.totalFovCount = totalCount;
                    if (currentIndex >= 0)
                        vm.currentFovIndex = currentIndex;
                    vm.inspecting = totalCount > 0 && currentIndex > 0;
                    if (!message.isEmpty())
                        vm.statusText = message;
                });
            },
            Qt::QueuedConnection);

    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::flowFinished,
            this,
            [this](bool success, const QString &message) {
                if (success)
                    publishRuntimeChange([&](HomeRuntimeState &state) { state.kpi.totalPanels += 1; });
                publishRuntimeChange([&](HomeRuntimeState &state) {
                    state.virtualMap.inspecting = false;
                    if (success && state.virtualMap.currentFovIndex > 0) {
                        PanelVirtualMapState &vm = state.virtualMap;
                        if (vm.activeFovGridRow >= 0 && vm.activeFovGridCol >= 0)
                            markCapturedCell(vm, vm.activeFovGridRow, vm.activeFovGridCol);
                        else if (!vm.activeFovRectNorm.isNull())
                            markCapturedCell(vm, vm.activeFovRectNorm);
                    }
                });
                if (!message.isEmpty())
                    appendLog(message, success ? QStringLiteral("INFO") : QStringLiteral("ERROR"));
            },
            Qt::QueuedConnection);

    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::virtualFovUpdated,
            this,
            [this](const QString &message, int currentIndex, int totalCount,
                   const std::shared_ptr<common::device::ImageInfo> &fovImage,
                   const QRectF &focusRect, int panelGroupCols, int panelGroupRows, int fovGridRows,
                   int fovGridCols, int fovGridRow, int fovGridCol, int activeTrack,
                   bool dualTrackMode) {
                const QImage preview = image_display::imageInfoToQImage(fovImage);
                publishRuntimeChange([&](HomeRuntimeState &state) {
                    state.activeProcessStep = HomeProcessStep::Inspect;
                    if (!preview.isNull())
                        state.preview.currentFov = preview;
                    state.preview.fovFocusRect = focusRect;

                    PanelVirtualMapState &vm = state.virtualMap;
                    if (totalCount > 0 && vm.totalFovCount != totalCount)
                        vm.capturedCells.clear();
                    if (currentIndex <= 0 && totalCount > 0)
                        vm.capturedCells.clear();
                    if (currentIndex == 1 && vm.currentFovIndex > 1)
                        vm.capturedCells.clear();

                    if (currentIndex > vm.currentFovIndex && vm.currentFovIndex > 0) {
                        if (vm.activeFovGridRow >= 0 && vm.activeFovGridCol >= 0)
                            markCapturedCell(vm, vm.activeFovGridRow, vm.activeFovGridCol);
                        else if (!vm.activeFovRectNorm.isNull())
                            markCapturedCell(vm, vm.activeFovRectNorm);
                    }

                    vm.panelGroupCols    = qMax(1, panelGroupCols);
                    vm.panelGroupRows    = qMax(1, panelGroupRows);
                    vm.fovGridCols       = qMax(1, fovGridCols);
                    vm.fovGridRows       = qMax(1, fovGridRows);
                    vm.currentFovIndex   = currentIndex;
                    vm.totalFovCount     = totalCount;
                    vm.activeTrack       = activeTrack;
                    vm.dualTrackMode     = dualTrackMode;
                    vm.inspecting        = totalCount > 0 && currentIndex > 0;
                    vm.statusText        = message;
                    vm.activeFovRectNorm = focusRect;
                    vm.activeFovGridRow  = fovGridRow;
                    vm.activeFovGridCol  = fovGridCol;
                });
            },
            Qt::QueuedConnection);

    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::markPreviewUpdated,
            this,
            [this](const std::shared_ptr<common::device::ImageInfo> &mark1,
                   const std::shared_ptr<common::device::ImageInfo> &mark2) {
                const QImage preview1 = image_display::imageInfoToQImage(mark1);
                const QImage preview2 = image_display::imageInfoToQImage(mark2);
                modifyRuntime([&](HomeRuntimeState &state) {
                    if (!preview1.isNull())
                        state.preview.mark1 = preview1;
                    if (!preview2.isNull())
                        state.preview.mark2 = preview2;
                });
            },
            Qt::QueuedConnection);

    connect(m_inspectionBridge.get(), &flow::ScanFlowProgressBridge::inspectionSnapshotUpdated,
            this, &HomeViewModel::applyInspectionSnapshot, Qt::QueuedConnection);
}

void HomeViewModel::setupRuntimeLogBinding()
{
    MessageCenter::Center::Attach(
        EVENT_RUNTIME_LOG, this, SLOT(onRuntimeLog(const QVariantList &)));
}

void HomeViewModel::onRuntimeLog(const QVariantList &data)
{
    if (data.size() < 2)
        return;

    const QString level   = data.at(0).toString();
    const QString message = data.at(1).toString();
    if (message.isEmpty())
        return;

    appendLog(message, level.isEmpty() ? QStringLiteral("INFO") : level);
}

void HomeViewModel::startInspectionFlow()
{
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady()) {
        appendLog(QStringLiteral("调度服务未就绪，请确认 app.toml 中 scheduler.enabled=true"),
                  QStringLiteral("ERROR"));
        return;
    }

    auto &coordinator = flow_host::flowHostCoordinator();
    coordinator.setActiveBridge(m_inspectionBridge.get());

    auto fsm = svc->machineFsm();
    if (!fsm) {
        appendLog(QStringLiteral("整机状态机不可用"), QStringLiteral("ERROR"));
        return;
    }

    modifyRuntime([&](HomeRuntimeState &state) {
        state.activeProcessStep = HomeProcessStep::Inspect;
    });

    fsm->uiStartButton();
}

void HomeViewModel::stopInspectionFlow()
{
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady()) {
        executeReservedCommand(QStringLiteral("aoi.sidebar.btn.pause"));
        return;
    }
    if (auto fsm = svc->machineFsm()) {
        fsm->uiStopButton();
        return;
    }
    executeReservedCommand(QStringLiteral("aoi.sidebar.btn.pause"));
}

void HomeViewModel::resetInspectionFlow()
{
    // 联调阶段：整机复位后额外投递一份调试用待复判会话，绕过真实检测工步验证复判链路。
    // triggerDirectReviewFlow();
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady()) {
        executeReservedCommand(QStringLiteral("aoi.sidebar.btn.machine_home"));
        return;
    }
    if (auto fsm = svc->machineFsm()) {
        fsm->uiResetButton();
        return;
    }
    executeReservedCommand(QStringLiteral("aoi.sidebar.btn.machine_home"));
}

void HomeViewModel::clearAlarmFlow()
{
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady()) {
        executeReservedCommand(QStringLiteral("aoi.sidebar.btn.clear_alarm"));
        return;
    }
    if (auto fsm = svc->machineFsm()) {
        fsm->uiClearAlarm();
        return;
    }
    executeReservedCommand(QStringLiteral("aoi.sidebar.btn.clear_alarm"));
}

void HomeViewModel::singleInspectFlow()
{
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady()) {
        appendLog(QStringLiteral("调度服务未就绪"), QStringLiteral("ERROR"));
        return;
    }

    auto &coordinator = flow_host::flowHostCoordinator();
    coordinator.setActiveBridge(m_inspectionBridge.get());

    modifyRuntime([&](HomeRuntimeState &state) {
        state.activeProcessStep = HomeProcessStep::Inspect;
    });

    QString err;
    if (!coordinator.submitSingleShot(&err)) {
        appendLog(err.isEmpty() ? QStringLiteral("单次检测提交失败") : err,
                  QStringLiteral("ERROR"));
    } else {
        appendLog(QStringLiteral("单次检测工步流已提交"), QStringLiteral("INFO"));
    }
}

void HomeViewModel::triggerDirectReviewFlow()
{
    if (!m_inspectionBridge)
        return;

    auto &coordinator = flow_host::flowHostCoordinator();
    coordinator.setActiveBridge(m_inspectionBridge.get());

    const inspection::DefectInfoList debugList = review::buildReviewDebugDefectInfoList();

    /**
     * 这里显式绕过真实检测工步，直接模拟“检测完成后的结果汇总已就绪”：
     * - 不启动 MachineFSM / Scheduler；
     * - 只通过 bridge 把完整 `DefectInfoList` 投递到复判页；
     * - 用于快速联调缺陷复判入口、MAP 显示、列表筛选与确认落盘。
     */
    m_inspectionBridge->reportReviewSession(debugList);
    appendLog(QStringLiteral("已直接构造调试用 DefectInfoList，并发送到复判界面"),
              QStringLiteral("INFO"));
}

void HomeViewModel::registerDeviceCommands()
{
    auto *svc = resolveSchedulerService();
    if (!svc || !svc->isReady())
        return;

    auto fsm = svc->machineFsm();
    if (!fsm)
        return;

    binding::fsm::HomeSidebarCommands cmds;
    binding::fsm::bindHomeMachineFsm(
        this,
        fsm,
        binding::fsm::HomeSidebarActions{
            [this]() { startInspectionFlow(); },
            [this]() { stopInspectionFlow(); },
            [this]() { resetInspectionFlow(); },
            [this]() { clearAlarmFlow(); },
            [this]() {
                executeReservedCommand(QStringLiteral("aoi.sidebar.btn.clear_material"));
            },
            [this]() { singleInspectFlow(); },
        },
        &cmds);

    m_startCommand         = cmds.start;
    m_stopCommand          = cmds.stop;
    m_resetCommand         = cmds.reset;
    m_clearAlarmCommand    = cmds.clearAlarm;
    m_clearMaterialCommand = cmds.clearMaterial;
    m_singleInspectCommand = cmds.singleInspect;

    registerDeviceActionPermissions();
    refreshDeviceCommands();
}

void HomeViewModel::registerDeviceActionPermissions()
{
    // 首页 ViewModel 在 PageWidget::onLoad 创建，已离开 AoiPlugin::initialize 的 PermCatalogScope
    const PermCatalogScope catalogScope(aoi::pluginName());

    static constexpr const char *kHomePageKey = "aoi.nav.home";
    PermService *ps = PermService::instance();
    ps->registerAction(QStringLiteral("home.start"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.start"));
    ps->registerAction(QStringLiteral("home.pause"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.pause"));
    ps->registerAction(QStringLiteral("home.reset"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.machine_home"));
    ps->registerAction(QStringLiteral("home.clear_alarm"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.clear_alarm"));
    ps->registerAction(QStringLiteral("home.clear_material"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.clear_material"));
    ps->registerAction(QStringLiteral("home.single_inspect"), QString::fromUtf8(kHomePageKey),
                       QStringLiteral("aoi.sidebar.btn.single_inspect"));
}

void HomeViewModel::executeReservedCommand(const QString &actionKey)
{
    appendLog(QStringLiteral("%1 (%2)")
                  .arg(TR("aoi.home.command.reserved"), TR(actionKey.toUtf8().constData())),
              QStringLiteral("INFO"));
}

} // namespace aoi
