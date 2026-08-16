/**
 * @file home_model.cpp
 * @brief HomeRuntimeState 演示数据与 KPI 格式化实现
 */

#include "home_model.h"

#include "recipe_model.h"
#include "system_config_model.h"

#include <QLocale>

/** @brief HomeRuntimeState 演示数据构造与 KPI 展示格式化实现 */
namespace lighten {

void KpiSnapshot::syncUseCountFromConfig()
{
    const GeneralSettings &gs = SystemConfigRepository::load().generalSettings;
    currentUseCount = gs.currentUseCount;
    maxUseCount     = gs.maxUseCount;
}

void KpiSnapshot::syncMaxUseCountFromConfig()
{
    maxUseCount = SystemConfigRepository::load().generalSettings.maxUseCount;
}

void KpiSnapshot::syncCurrentRecipeFromDefault()
{
    const QString id = RecipeRepository::defaultRecipeId();
    if (!id.isEmpty())
        currentRecipe = id;
}

/** 构造空快照：各检测项 OK，RGB 项计数为 0，无分色项显示 "-" */
CurrentInspectionSnapshot CurrentInspectionSnapshot::defaultEmpty()
{
    CurrentInspectionSnapshot snapshot;
    snapshot.serialNumber.clear();
    snapshot.overallResult = InspectionResult::Ok;
    snapshot.rows.reserve(kDetectItemCount);

    for (int i = 0; i < kDetectItemCount; ++i) {
        const auto kind = static_cast<DetectItemKind>(i);
        ResultSnapshotRow row;
        row.item   = kind;
        row.result = InspectionResult::Ok;

        if (kind <= DetectItemKind::MixedColor) {
            row.r = row.g = row.b = row.total = 0;
        } else if (kind == DetectItemKind::BlackScreenOn) {
            row.r = row.g = row.b = -1;
            row.total = 0;
        } else {
            row.r = row.g = row.b = row.total = -1;
        }
        snapshot.rows.append(row);
    }
    return snapshot;
}

/** 填充首页演示用 KPI、进度、日志与工位状态 */
HomeRuntimeState HomeRuntimeState::defaultDemo()
{
    HomeRuntimeState s;
    s.kpi = KpiSnapshot{};
    s.kpi.syncMaxUseCountFromConfig();
    s.kpi.syncCurrentRecipeFromDefault();

    const QVector<QString> demoStatuses = {
        QStringLiteral("已完成"), QStringLiteral("已完成"), QStringLiteral("已完成"),
        QStringLiteral("进行中"), QStringLiteral("进行中"), QStringLiteral("进行中"),
        QStringLiteral("进行中"), QStringLiteral("进行中"), QStringLiteral("进行中"),
        QStringLiteral("进行中"), QStringLiteral("进行中"), QStringLiteral("进行中"),
        QStringLiteral("进行中"), QStringLiteral("进行中"), QStringLiteral("进行中"),
        QStringLiteral("进行中"), QStringLiteral("进行中"), QStringLiteral("进行中"),
        QStringLiteral("进行中"), QStringLiteral("未开始"), QStringLiteral("未开始"),
    };
    s.progress.resize(kHomeProgressCount);
    for (int i = 0; i < kHomeProgressCount; ++i) {
        s.progress[i].statusText = demoStatuses.value(i, QStringLiteral("未开始"));
    }

    s.snapshot = CurrentInspectionSnapshot::defaultEmpty();

    s.stations = StationTrackState{};
    s.preview  = HomePreviewState{};

    return s;
}

const QStringList &HomeRuntimeState::kpiLabels()
{
    static const QStringList labels = {
        QStringLiteral("总点亮板数"), QStringLiteral("OK"), QStringLiteral("NG"),
        QStringLiteral("RW"),       QStringLiteral("对插次数"), QStringLiteral("通过率"),
        QStringLiteral("TT时间"),   QStringLiteral("运行模式"), QStringLiteral("设备状态"),
        QStringLiteral("当前配方"),
    };
    return labels;
}

/** 对插次数展示：当前/上限，千分位格式化 */
QString HomeRuntimeState::formatInsertionCount(int currentUseCount, int maxUseCount)
{
    return QStringLiteral("%1/%2")
        .arg(QLocale().toString(currentUseCount))
        .arg(QLocale().toString(maxUseCount));
}

QStringList HomeRuntimeState::kpiDisplayValues() const
{
    return {
        QLocale().toString(kpi.totalPanels),
        QLocale().toString(kpi.okCount),
        QLocale().toString(kpi.ngCount),
        QLocale().toString(kpi.rwCount),
        formatInsertionCount(kpi.currentUseCount, kpi.maxUseCount),
        QStringLiteral("%1%").arg(QString::number(kpi.passRate, 'f', 2)),
        QStringLiteral("%1s").arg(QString::number(kpi.cycleTimeSec, 'f', 1)),
        runModeText(kpi.runMode),
        deviceStateText(kpi.deviceState),
        kpi.currentRecipe,
    };
}

/** 快照单元格：负值显示 "-" */
QString HomeRuntimeState::formatResultCell(int value)
{
    return value < 0 ? QStringLiteral("-") : QString::number(value);
}

QString HomeRuntimeState::formatSnapshotTotal(DetectItemKind item, int total)
{
    if (!detectItemHasTotalCount(item))
        return QStringLiteral("-");
    return formatResultCell(total);
}

} // namespace lighten
