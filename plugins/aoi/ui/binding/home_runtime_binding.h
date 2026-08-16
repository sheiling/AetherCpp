#pragma once

/**
 * @file home_runtime_binding.h
 * @brief 首页 `HomeRuntimeState` → 各 UI 区块的单向细粒度 property 绑定
 *
 * ## 设计动机
 *
 * 旧实现：`HomeViewModel::runtimeChanged` → `HomeView::refreshFromRuntime()` 整页刷新，
 * 任意字段变化都会重绘 KPI、硬件、流程、虚拟图、日志、快照、预览全部控件。
 *
 * 现实现：监听 `ViewModel::propertyChanged("runtime")`，按 **7 个独立分区**
 * 各自 `apply*Section`，仅更新本区控件（与配方页 `bindStructField` 模式一致）。
 *
 * ## 数据流
 *
 * ```
 * 工步 / FSM / Bridge / MessageCenter
 *        → HomeViewModel::modifyRuntime / publishRuntimeChange
 *        → propertyChanged("runtime", HomeRuntimeState)
 *        → bindHomeRuntime 各分区 handler（QueuedConnection 同线程则直接）
 *        → 对应 View 控件
 * ```
 *
 * ## 与相机 role 预览的关系
 *
 * Mark/FOV 大图另由 `bindRoleImagePreview` 订阅 `ICameraService::roleImageReady`；
 * 本模块的 `applyPreviewSection` 负责 runtime 内的 QImage 快照、ROI/聚焦框，
 * 以及演示数据（`defaultDemo`）路径。
 *
 * ## 主题 / 语言切换
 *
 * View 在 `applyTheme` / `retranslateUi` 后调用 `HomeViewModel::refreshRuntimeBindings()`，
 * 重发 `propertyChanged("runtime")`，各分区 handler 重新 TR 并刷新依赖 ThemeTokens 的样式。
 */

#include "vm/home/home_view_model.h"

#include <functional>

class QLabel;

namespace ui {

class ImagePreview;
class KpiMetricCard;
class LogView;
class ThemedTableWidget;

} // namespace ui

namespace aoi {

class HardwareStatusPanel;
class HomeProcessFlowWidget;
class PanelVirtualMapWidget;

} // namespace aoi

namespace aoi::binding {

/**
 * @brief 首页 runtime 绑定目标控件集合
 *
 * 由 `HomeView::setupBindings()` 在布局完成后填充；指针生命周期须 ≥ `context`。
 * 未使用的指针可留 nullptr，对应分区 handler 内会跳过。
 */
struct HomeRuntimeBindingTargets
{
    /** KPI 顶栏 9 卡，顺序与 `HomeRuntimeState::kpiDisplayValues()` 一致 */
    QVector<ui::KpiMetricCard *>        kpiCards;
    /** 侧栏硬件连接状态面板（内含 `StatusIndicatorRow` 列表） */
    aoi::HardwareStatusPanel           *hardwarePanel     = nullptr;
    /** 中区进料/检测/出料流程条 */
    aoi::HomeProcessFlowWidget         *processFlow       = nullptr;
    /** 灯板虚拟图（FOV 网格高亮） */
    aoi::PanelVirtualMapWidget         *virtualMapWidget  = nullptr;
    /** 运行日志（`LogView::syncEntries` 本身增量，重复调用安全） */
    ui::LogView                     *logView           = nullptr;
    /** 快照区 SN 行 */
    QLabel                          *snapshotSnLbl     = nullptr;
    /** 快照区总判 OK/NG 行（objectName 随结果切换以应用 QSS） */
    QLabel                          *snapshotStatusLbl = nullptr;
    /** 11 行 × 6 列缺陷统计表 */
    ui::ThemedTableWidget           *snapshotTable     = nullptr;
    /** Mark1 预览（与 role 通道互补） */
    ui::ImagePreview                *mark1Preview      = nullptr;
    ui::ImagePreview                *mark2Preview      = nullptr;
    /** 当前 FOV 预览 + ROI/聚焦框 */
    ui::ImagePreview                *fovPreview        = nullptr;
    /**
     * 快照总判 label 的 objectName 变更后回调，用于重刷 `applyHomeDecorStyles`。
     * 由 HomeView 传入 lambda，避免 binding 层依赖 View 私有方法。
     */
    std::function<void()>            applyDecorStyles;
};

/**
 * @brief 挂载 runtime 全部分区绑定
 *
 * 对 `HomeViewModel::PROP_RUNTIME` 注册 7 路 `BindingInternal::connectStructVmToView`；
 * 首次调用会立即 apply 当前 runtime 初值。
 *
 * @param vm      首页 ViewModel，须已 `registerProperty(PROP_RUNTIME, ...)`
 * @param targets 各分区控件指针集合
 * @param context Qt 连接生命周期宿主（通常为 `HomeView`）
 */
void bindHomeRuntime(aoi::HomeViewModel *vm, const HomeRuntimeBindingTargets &targets, QObject *context);

} // namespace aoi::binding
