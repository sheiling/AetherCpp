#pragma once

/**
 * @file home_view_model.h
 * @brief 主页 ViewModel：聚合 `HomeRuntimeState` 并暴露设备侧栏 Command
 *
 * 侧栏 6 钮：**启动** / **暂停** / **整机复位** / **清除报警** / **清料** / **单次检测**，
 * 前四项对接 `MachineFSM`（v0.6：`Start` 含继续语义）。KPI 与 `MachineStatus` 映射见 `home_view_model.cpp`。
 * 硬件连接状态由 `HomeHardwareStatusViewModel` 子模块维护。
 */

#include "home_runtime_state.h"

#include "command.h"
#include "viewmodel.h"

#include <scheduler/machine/machine_status.h>

namespace aoi::flow {
class ScanFlowProgressBridge;
} // namespace aoi::flow

namespace aoi {

class HomeHardwareStatusViewModel;

class HomeViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_RUNTIME = "runtime";

signals:
    void runtimeChanged();

public:
    VIEWMODEL_STRUCT_PROPERTY(HomeRuntimeState, runtime, Runtime)

public:
    /** 主题/语言切换后重推 runtime 至 View 分区绑定 */
    void refreshRuntimeBindings();
    explicit HomeViewModel(QObject *parent = nullptr);
    ~HomeViewModel() override;

    Command *startCommand() const { return m_startCommand; }
    Command *stopCommand() const { return m_stopCommand; }
    Command *resetCommand() const { return m_resetCommand; }
    Command *clearAlarmCommand() const { return m_clearAlarmCommand; }
    Command *clearMaterialCommand() const { return m_clearMaterialCommand; }
    Command *singleInspectCommand() const { return m_singleInspectCommand; }

    HomeHardwareStatusViewModel *hardwareStatusViewModel() const { return m_hardwareStatusVm; }

    void loadDemoData();

    void applyMachineStatusToKpi(const scheduler::MachineStatus &status);
    void refreshDeviceCommands();

    /** 委托 `HomeHardwareStatusViewModel::refresh` */
    void refreshHardwareStatus();

public slots:
    void appendLog(const QString &message, const QString &level = QStringLiteral("INFO"));
    void applyInspectionSnapshot(const CurrentInspectionSnapshot &snapshot);

private slots:
    void onRuntimeLog(const QVariantList &data);

private:
    void registerDeviceCommands();
    void registerDeviceActionPermissions();
    void executeReservedCommand(const QString &actionKey);
    void startInspectionFlow();
    void stopInspectionFlow();
    void resetInspectionFlow();
    void clearAlarmFlow();
    void singleInspectFlow();
    /**
     * 联调入口：经 `defect_review_debug_data` 构造 `DefectInfoList` 并投递到复判页。
     * 当前由 `resetInspectionFlow()` 在整机复位逻辑执行后调用。
     */
    void triggerDirectReviewFlow();
    void setupInspectionBridge();
    void setupRuntimeLogBinding();
    void publishRuntimeChange(const std::function<void(HomeRuntimeState &)> &modifier);

    std::unique_ptr<flow::ScanFlowProgressBridge> m_inspectionBridge;
    HomeHardwareStatusViewModel                   *m_hardwareStatusVm = nullptr;

    FunctionalCommand *m_startCommand         = nullptr;
    FunctionalCommand *m_stopCommand          = nullptr;
    FunctionalCommand *m_resetCommand         = nullptr;
    FunctionalCommand *m_clearAlarmCommand    = nullptr;
    FunctionalCommand *m_clearMaterialCommand = nullptr;
    FunctionalCommand *m_singleInspectCommand = nullptr;
};

} // namespace aoi
