/**
 * @file home_runtime_state.cpp
 * @brief `HomeRuntimeState` 演示数据、KPI 格式化与计数单元格显示
 */

#include "home_runtime_state.h"

#include <QLocale>

namespace aoi {

namespace {

InspectionSnapshotRow makeSnapshotRow(inspection::InspectionDefectType kind, int r, int g, int b, int total,
                                      InspectionOverallResult result)
{
    InspectionSnapshotRow row;
    row.kind   = kind;
    row.r      = r;
    row.g      = g;
    row.b      = b;
    row.total  = total;
    row.result = result;
    return row;
}

} // namespace

HomeRuntimeState HomeRuntimeState::defaultDemo()
{
    HomeRuntimeState state;

    state.kpi.totalPanels   = 12640;
    state.kpi.okCount       = 12101;
    state.kpi.ngCount       = 428;
    state.kpi.rwCount       = 111;
    state.kpi.passRate      = 95.74;
    state.kpi.cycleTimeSec  = 4.2;
    state.kpi.runMode       = QStringLiteral("aoi.home.run_mode.auto");
    state.kpi.deviceState   = QStringLiteral("aoi.home.machine_state.running");
    state.kpi.currentRecipe = QStringLiteral("PANEL-A01");

    state.snapshot.serialNumber  = QStringLiteral("SN-20260422-00128");
    state.snapshot.overallResult = InspectionOverallResult::Ng;
    state.snapshot.rows          = {
        makeSnapshotRow(inspection::InspectionDefectType::MissingChip, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::ChipOffset, 1, 0, 0, 1, InspectionOverallResult::Ng),
        makeSnapshotRow(inspection::InspectionDefectType::WrongComponent, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::Reversed, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::Flipped, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::Tombstone, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::Duplicate, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::Damage, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::ForeignObject, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::ExcessChip, 0, 0, 0, 0, InspectionOverallResult::Ok),
        makeSnapshotRow(inspection::InspectionDefectType::InsufficientChip, 0, 0, 0, 0, InspectionOverallResult::Ok),
    };

    const QDateTime now = QDateTime::currentDateTime();
    state.logs          = {
        {now.addSecs(-120), QStringLiteral("INFO"), QStringLiteral("System initialized.")},
        {now.addSecs(-60), QStringLiteral("WARN"), QStringLiteral("Serial port disconnected.")},
        {now.addSecs(-30), QStringLiteral("ERROR"), QStringLiteral("Inspection paused.")},
    };

    state.preview.fovFocusRect = QRectF(40, 30, 120, 90);
    state.preview.fovRoiRect   = QRectF(20, 20, 160, 120);
    state.activeProcessStep    = HomeProcessStep::Inspect;

    state.virtualMap.panelGroupCols  = 100;
    state.virtualMap.panelGroupRows  = 160;
    state.virtualMap.fovGridCols     = 5;
    state.virtualMap.fovGridRows     = 4;
    state.virtualMap.currentFovIndex = 15;
    state.virtualMap.totalFovCount   = 20;
    state.virtualMap.dualTrackMode   = true;
    state.virtualMap.activeTrack     = 1;
    state.virtualMap.inspecting      = true;
    state.virtualMap.activeFovRectNorm = QRectF(0.6, 0.75, 0.2, 0.25);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (row * 5 + col < 14)
                state.virtualMap.capturedCells.insert(row * 10000 + col);
        }
    }

    return state;
}

QStringList HomeRuntimeState::kpiDisplayValues() const
{
    const QLocale locale;
    return {
        locale.toString(kpi.totalPanels),
        locale.toString(kpi.okCount),
        locale.toString(kpi.ngCount),
        locale.toString(kpi.rwCount),
        QStringLiteral("%1%").arg(QString::number(kpi.passRate, 'f', 2)),
        QStringLiteral("%1s").arg(QString::number(kpi.cycleTimeSec, 'f', 1)),
        kpi.runMode,
        kpi.deviceState,
        kpi.currentRecipe,
    };
}

QString HomeRuntimeState::formatCountCell(int value)
{
    if (value < 0)
        return QStringLiteral("-");
    return QString::number(value);
}

} // namespace aoi
