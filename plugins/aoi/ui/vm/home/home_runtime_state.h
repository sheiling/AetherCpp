#pragma once

/**
 * @file home_runtime_state.h
 * @brief 主页运行时快照：KPI、硬件链路、日志、检测快照、预览图与工艺步骤
 *
 * 作为 `HomeViewModel::runtime` 的值类型，经 DataBinding 单向/双向同步到各区域控件。
 * 字段变更由 ViewModel 的 `modifyRuntime` / `assignProperty` 触发 `propertyChanged("runtime")`。
 * hardware 列表由 `HomeHardwareStatusViewModel` 写入。
 */

#include "inspection_types.h"
#include "component/logView/log_entry.h"
#include "component/controls/label/status_indicator_row.h"
#include <QDateTime>
#include <QImage>
#include <QRectF>
#include <QSet>
#include <QString>
#include <QVector>

namespace aoi {

/** 主页三工步高亮：上料 / 检测 / 下料 */
enum class HomeProcessStep { Feed, Inspect, Discharge };

/** KPI 区数值与文本快照（产量、良率、节拍、运行模式等） */
struct KpiSnapshot
{
    qint64  totalPanels  = 0;
    qint64  okCount      = 0;
    qint64  ngCount      = 0;
    qint64  rwCount      = 0;
    double  passRate     = 0.0;
    double  cycleTimeSec = 0.0;
    QString runMode;
    QString deviceState;
    QString currentRecipe;
};

/** 硬件类型（与 message_define.h 中 EVENT_* 参数语义对应） */
enum class HardwareDeviceKind {
    Camera,
    Flash,
    Motion,
    Scanner,
};

/** 硬件链路一行：DeviceManager 注册名、显示名、链路指示灯、主/次级状态 i18n key */
struct HardwareStatusRow
{
    HardwareDeviceKind       kind = HardwareDeviceKind::Camera;
    QString                  deviceKey;
    QString                  displayName;
    ui::LinkIndicatorState   state = ui::LinkIndicatorState::Unknown;
    QString                  statusKey;
    QString                  subStatusKey;
};

struct HomePreviewImages
{
    QImage mark1;
    QImage mark2;
    QImage currentFov;
    QRectF fovFocusRect;
    QRectF fovRoiRect;
};

struct PanelVirtualMapState
{
    int       panelGroupCols  = 100;
    int       panelGroupRows  = 160;
    int       fovGridCols     = 1;
    int       fovGridRows     = 1;
    int       currentFovIndex = 0;
    int       totalFovCount   = 0;
    int       activeFovGridRow = -1;
    int       activeFovGridCol = -1;
    int       activeTrack     = 1;
    bool      dualTrackMode   = false;
    bool      inspecting      = false;
    QString   statusText;
    QRectF    activeFovRectNorm;
    QSet<int> capturedCells;
};

struct HomeRuntimeState
{
    KpiSnapshot                kpi;
    QVector<HardwareStatusRow> hardware;
    QVector<ui::LogEntry>      logs;
    CurrentInspectionSnapshot  snapshot;
    HomePreviewImages          preview;
    PanelVirtualMapState       virtualMap;
    HomeProcessStep            activeProcessStep = HomeProcessStep::Inspect;

    static HomeRuntimeState defaultDemo();
    QStringList             kpiDisplayValues() const;
    static QString          formatCountCell(int value);
};

} // namespace aoi

Q_DECLARE_METATYPE(aoi::HomeRuntimeState)
