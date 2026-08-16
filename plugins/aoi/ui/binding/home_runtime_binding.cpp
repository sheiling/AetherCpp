/**
 * @file home_runtime_binding.cpp
 * @brief HomeRuntimeState 分区 → View 控件单向绑定实现
 *
 * 每个 `apply*Section` 对应 runtime 内一组字段；`bindRuntimeSection` 将其挂到
 * `propertyChanged("runtime")`。runtime 任意字段变化时 7 个 handler 均会触发，
 * 但各 handler 只 touch 本区控件，避免整页 refresh 函数耦合。
 */

#include "home_runtime_binding.h"

#include "home/hardware_status_panel.h"
#include "home/home_process_flow_widget.h"
#include "home/panel_virtual_map_widget.h"
#include "inspection_types.h"
#include "vm/home/home_view_model.h"
#include "component/controls/label/kpi_metric_card.h"
#include "component/controls/label/status_indicator_row.h"
#include "component/image/image_preview.h"
#include "component/logView/log_view.h"
#include "component/table/themed_table_widget.h"
#include "component/theme/theme_tokens.h"

#include "binding_internal.h"
#include "viewmodel.h"
#include "uibase/languagemanager.h"

#include <QLabel>
#include <QTableWidgetItem>

namespace aoi::binding {

namespace {

using namespace ui;

/** KPI 列着色：索引 1=OK、2=NG、3=RW 用语义色，其余 Normal */
KpiValueRole kpiRoleForIndex(int index)
{
    if (index == 1)
        return KpiValueRole::Ok;
    if (index == 2)
        return KpiValueRole::Ng;
    if (index == 3)
        return KpiValueRole::Warning;
    return KpiValueRole::Normal;
}

/** 快照/表格「结果」列固定文案（OK/NG 缩写不进入翻译表） */
QString overallResultText(InspectionOverallResult result)
{
    switch (result) {
    case InspectionOverallResult::Ok:
        return QStringLiteral("OK");
    case InspectionOverallResult::Ng:
        return QStringLiteral("NG");
    case InspectionOverallResult::Unknown:
    default:
        return QStringLiteral("-");
    }
}

/** 分区 1：KPI 顶栏（索引 6/7 的 runMode/deviceState 存 i18n key，此处 TR） */
void applyKpiSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    const QStringList values = state.kpiDisplayValues();
    for (int i = 0; i < targets.kpiCards.size() && i < values.size(); ++i) {
        if (!targets.kpiCards[i])
            continue;
        QString text = values[i];
        if (i == 6 || i == 7)
            text = TR(text.toUtf8().constData());
        targets.kpiCards[i]->setValue(text);
        targets.kpiCards[i]->setValueRole(kpiRoleForIndex(i));
    }
}

/** 分区 2：侧栏硬件行（displayName 直显；status/subStatus 为 i18n key） */
void applyHardwareSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.hardwarePanel)
        targets.hardwarePanel->syncRowCount(state.hardware.size());

    const auto rows =
        targets.hardwarePanel ? targets.hardwarePanel->hardwareRows() : QVector<ui::StatusIndicatorRow *>{};
    for (int i = 0; i < rows.size() && i < state.hardware.size(); ++i) {
        const auto &row = state.hardware[i];
        rows[i]->setName(row.displayName);
        rows[i]->setStatusText(TR(row.statusKey.toUtf8().constData()));
        rows[i]->setSubStatusText(
            row.subStatusKey.isEmpty() ? QString() : TR(row.subStatusKey.toUtf8().constData()));
        rows[i]->setIndicatorState(row.state);
    }
}

/** 分区 3：三工步流程条高亮 */
void applyProcessFlowSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.processFlow)
        targets.processFlow->setActiveStep(state.activeProcessStep);
}

/** 分区 4：灯板虚拟图（FOV 网格、已采格、capturedCells 等） */
void applyVirtualMapSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.virtualMapWidget)
        targets.virtualMapWidget->applyState(state.virtualMap);
}

/** 分区 5：运行日志（增量 sync，appendLog 高频场景友好） */
void applyLogSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.logView)
        targets.logView->syncEntries(state.logs);
}

/**
 * 分区 6：当前板快照
 * - SN / 总判 meta 行
 * - 固定 kInspectionDefectTypeCount 行缺陷表（缺行用 "-" 占位）
 * - Result 列前景色随 ThemeTokens；总判 objectName 切换后触发 applyDecorStyles
 */
void applySnapshotSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.snapshotSnLbl)
        targets.snapshotSnLbl->setText(TR("aoi.home.snapshot.sn").arg(state.snapshot.serialNumber));

    if (targets.snapshotStatusLbl) {
        const QString result = overallResultText(state.snapshot.overallResult);
        targets.snapshotStatusLbl->setText(TR("aoi.home.snapshot.overall").arg(result));
        targets.snapshotStatusLbl->setObjectName(
            state.snapshot.overallResult == InspectionOverallResult::Ok
                ? QStringLiteral("homeSnapshotStatusOk")
                : QStringLiteral("homeSnapshotStatusNg"));
    }

    if (!targets.snapshotTable)
        return;

    const ui::ThemeTokens tokens = ui::ThemeTokens::resolve();
    targets.snapshotTable->setHorizontalHeaderLabels({
        TR("aoi.home.result.item"),
        TR("aoi.home.result.r"),
        TR("aoi.home.result.g"),
        TR("aoi.home.result.b"),
        TR("aoi.home.result.total"),
        TR("aoi.home.result.result"),
    });

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const InspectionSnapshotRow *src =
            i < state.snapshot.rows.size() ? &state.snapshot.rows[i] : nullptr;
        const inspection::InspectionDefectType kind =
            src ? src->kind : static_cast<inspection::InspectionDefectType>(i);

        auto itemText = [&](int col) -> QString {
            if (!src)
                return HomeRuntimeState::formatCountCell(-1);
            switch (col) {
            case 0:
                return TR(inspection::inspectionDefectTypeDisplayKey(kind).toUtf8().constData());
            case 1:
                return HomeRuntimeState::formatCountCell(src->r);
            case 2:
                return HomeRuntimeState::formatCountCell(src->g);
            case 3:
                return HomeRuntimeState::formatCountCell(src->b);
            case 4:
                return HomeRuntimeState::formatCountCell(src->total);
            case 5:
                return overallResultText(src->result);
            default:
                return QString();
            }
        };

        for (int col = 0; col < 6; ++col) {
            QTableWidgetItem *item = targets.snapshotTable->item(i, col);
            if (!item) {
                item = new QTableWidgetItem;
                targets.snapshotTable->setItem(i, col, item);
            }
            item->setText(itemText(col));
            item->setTextAlignment(Qt::AlignCenter);
            if (col == 5 && src) {
                if (src->result == InspectionOverallResult::Ok)
                    item->setForeground(tokens.accent);
                else if (src->result == InspectionOverallResult::Ng)
                    item->setForeground(tokens.dangerText);
            }
        }
    }

    if (targets.applyDecorStyles)
        targets.applyDecorStyles();
}

/** 分区 7：预览区 QImage + FOV ROI/聚焦框（空图 clear） */
void applyPreviewSection(const HomeRuntimeBindingTargets &targets, const HomeRuntimeState &state)
{
    if (targets.mark1Preview) {
        if (state.preview.mark1.isNull())
            targets.mark1Preview->clearImage();
        else
            targets.mark1Preview->setImage(state.preview.mark1);
    }
    if (targets.mark2Preview) {
        if (state.preview.mark2.isNull())
            targets.mark2Preview->clearImage();
        else
            targets.mark2Preview->setImage(state.preview.mark2);
    }
    if (targets.fovPreview) {
        if (state.preview.currentFov.isNull())
            targets.fovPreview->clearImage();
        else
            targets.fovPreview->setImage(state.preview.currentFov);
        targets.fovPreview->setRoiRect(state.preview.fovRoiRect);
        targets.fovPreview->setFocusRect(state.preview.fovFocusRect);
    }
}

using SectionApplier = void (*)(const HomeRuntimeBindingTargets &, const HomeRuntimeState &);

/**
 * @brief 将单个分区 applier 挂到 runtime propertyChanged
 *
 * 内部使用 `BindingInternal::connectStructVmToView`：首次 apply 当前值，
 * 之后仅在 `name == PROP_RUNTIME` 时分发。
 */
void bindRuntimeSection(ViewModel *vm, const HomeRuntimeBindingTargets &targets, QObject *context,
                        SectionApplier applySection)
{
    // 必须按值捕获 targets：HomeView::setupBindings 传入的是栈上局部变量，
    // 若用 [&targets] 则 propertyChanged 回调触发时结构体已失效 → KPI 卡指针野引用崩溃。
    const auto apply = [targets, applySection](const QVariant &val) {
        applySection(targets, val.value<HomeRuntimeState>());
    };
    BindingInternal::connectStructVmToView(vm, HomeViewModel::PROP_RUNTIME, context, apply, nullptr);
}

} // namespace

void bindHomeRuntime(aoi::HomeViewModel *vm, const HomeRuntimeBindingTargets &targets, QObject *context)
{
    if (!vm || !context)
        return;

    bindRuntimeSection(vm, targets, context, applyKpiSection);
    bindRuntimeSection(vm, targets, context, applyHardwareSection);
    bindRuntimeSection(vm, targets, context, applyProcessFlowSection);
    bindRuntimeSection(vm, targets, context, applyVirtualMapSection);
    bindRuntimeSection(vm, targets, context, applyLogSection);
    bindRuntimeSection(vm, targets, context, applySnapshotSection);
    bindRuntimeSection(vm, targets, context, applyPreviewSection);
}

} // namespace aoi::binding
