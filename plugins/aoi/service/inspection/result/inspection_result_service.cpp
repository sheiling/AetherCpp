#include "inspection_result_service.h"

#include "inspection_step_utils.h"
#include "inspection_history_coordinator.h"
#if COMMON_ORM_ENABLED
#include "repository/inspection_history/inspection_history_repository.h"
#endif
#include "service/fov_planner/fov_layout_types.h"
#include "service/fov_planner/fov_plan_service.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QMap>

namespace aoi::inspection {
namespace {

InspectionSnapshotRow makeRow(InspectionDefectType type, int r, int g, int b, int total,
                              aoi::InspectionOverallResult result)
{
    InspectionSnapshotRow row;
    row.kind   = type;
    row.r      = r;
    row.g      = g;
    row.b      = b;
    row.total  = total;
    row.result = result;
    return row;
}

void countInspectionDefects(InspectionDefectTypeMask mask, QMap<InspectionDefectType, int> &totals)
{
    for (int i = 0; i < kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<InspectionDefectType>(i);
        if (inspectionMaskHasDefect(mask, type))
            totals[type]++;
    }
}

aoi::Rect algoRectToAoiRect(const vision_algo::AlgoRect2f &rect)
{
    aoi::Rect roi;
    roi.x = rect.x;
    roi.y = rect.y;
    roi.w = rect.width;
    roi.h = rect.height;
    return roi;
}

InspectionDefectTypeMask buildValidDefectTypeMask(const aoi::DefectSettings &defects)
{
    InspectionDefectTypeMask mask = 0;
    for (int i = 0; i < kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<InspectionDefectType>(i);
        if (defects.item(type).enabled)
            mask |= inspectionDefectTypeBit(type);
    }
    return mask;
}

QString capturePathForFovIndex(const InspectionSession &session, int fovIndex)
{
    for (const FovCaptureRecord &cap : session.fovCaptures) {
        if (cap.fovIndex == fovIndex)
            return cap.diskPath;
    }
    if (fovIndex > 0 && fovIndex <= session.fovCaptures.size())
        return session.fovCaptures[fovIndex - 1].diskPath;
    return {};
}

const FovCaptureRecord *captureRecordForFovIndex(const InspectionSession &session, int fovIndex)
{
    for (const FovCaptureRecord &cap : session.fovCaptures) {
        if (cap.fovIndex == fovIndex)
            return &cap;
    }
    if (fovIndex > 0 && fovIndex <= session.fovCaptures.size())
        return &session.fovCaptures[fovIndex - 1];
    return nullptr;
}

/**
 * @brief 晶粒坐标换算结果
 *
 * `rowIndexInPanel/colIndexInPanel` 仍表示芯片组（灯珠组）坐标，供列表和调试展示；
 * `dieRowIndexInPanel/dieColIndexInPanel` 才是复判 MAP 的晶粒级主键。
 */
struct PanelDieCoordinate
{
    int panelGroupRow = -1;
    int panelGroupCol = -1;
    int dieRow        = -1;
    int dieCol        = -1;
    int dieRowCount   = 0;
    int dieColCount   = 0;
};

/**
 * @brief 按当前产品配方的组内芯片排列，将“芯片组 + 组内芯片索引”展开为晶粒坐标
 *
 * - Vertical：三颗芯片沿行方向展开，同一组的三颗芯片映射为连续三行。
 * - Horizontal：三颗芯片沿列方向展开，同一组的三颗芯片映射为连续三列。
 *
 * 该函数是检测结果到复判 MAP 坐标的唯一换算入口，避免在 UI / VM 中写死当前产品公式。
 */
PanelDieCoordinate mapGroupChipToPanelDie(int groupRow, int groupCol, int chipType,
                                          int groupRowCount, int groupColCount,
                                          aoi::fov_planner::FovChipArrangement arrangement)
{
    constexpr int kChipsPerGroup = 3;

    PanelDieCoordinate out;
    out.panelGroupRow = groupRow;
    out.panelGroupCol = groupCol;

    if (arrangement == aoi::fov_planner::FovChipArrangement::Vertical) {
        out.dieRow      = groupRow * kChipsPerGroup + chipType;
        out.dieCol      = groupCol;
        out.dieRowCount = std::max(groupRowCount, 0) * kChipsPerGroup;
        out.dieColCount = std::max(groupColCount, 0);
        return out;
    }

    out.dieRow      = groupRow;
    out.dieCol      = groupCol * kChipsPerGroup + chipType;
    out.dieRowCount = std::max(groupRowCount, 0);
    out.dieColCount = std::max(groupColCount, 0) * kChipsPerGroup;
    return out;
}

DefectInfoList buildDefectInfoList(const InspectionSession &session, const QString &serialNumber)
{
    DefectInfoList list;
    list.detectTime      = QDateTime::currentDateTime().toString(Qt::ISODate);
    list.validDefectType = buildValidDefectTypeMask(session.sysConfig.defects);
    list.append          = false;
    list.productName     = session.recipe.basic.name;
    list.SN              = serialNumber;
    list.chipGroupRowCount = session.recipe.basic.width;
    list.chipGroupColCount = session.recipe.basic.height;

    const auto arrangement =
        static_cast<aoi::fov_planner::FovChipArrangement>(session.recipe.basic.chipArrangement);
    const PanelDieCoordinate panelScale =
        mapGroupChipToPanelDie(0, 0, 0, list.chipGroupRowCount, list.chipGroupColCount, arrangement);
    list.dieRowCount = panelScale.dieRowCount;
    list.dieColCount = panelScale.dieColCount;

    const QString resultBase = session.sysConfig.paths.resultPath.trimmed();
    if (!resultBase.isEmpty()) {
        const QString subDir =
            QDir(resultBase).filePath(QStringLiteral("%1_%2")
                                          .arg(serialNumber)
                                          .arg(QDateTime::currentDateTime().toString(
                                              QStringLiteral("yyyyMMdd_hhmmss"))));
        list.defectInfoFullFileName =
            QDir(subDir).filePath(QStringLiteral("defect_info.json"));
    }

    const fov_planner::FovChipIndexMap &chipMap = session.fovGeometryPlan.chipMap;

    for (const FovDetectRecord &fov : session.fovDetects) {
        DefectInfoPartition partition;
        partition.partitionIndex           = fov.fovIndex > 0 ? fov.fovIndex - 1 : 0;
        if (const FovCaptureRecord *cap = captureRecordForFovIndex(session, fov.fovIndex)) {
            partition.partitionGridRow = cap->gridRow;
            partition.partitionGridCol = cap->gridCol;
        }
        partition.partitionImageOriginName = capturePathForFovIndex(session, fov.fovIndex);
        partition.partitionImageDetectName = partition.partitionImageOriginName;

        for (const vision_algo::ChipGroupDetectInfo &group : fov.groups) {
            for (int ci = 0; ci < 3; ++ci) {
                const vision_algo::ChipDetectInfo &chip = group.chipRgbInfo[ci];
                const InspectionDefectTypeMask defectMask =
                    chipDefectToInspectionMask(chip.chipDefect);
                if (defectMask == 0)
                    continue;

                DefectInfoItem item;
                item.chipType            = static_cast<int>(chip.chipType);
                item.defectType          = defectMask;
                item.rejudgeDefectType   = 0;
                item.isRejudge           = false;
                item.rowIndexInPartition = group.row;
                item.colIndexInPartition = group.col;
                item.roi                 = algoRectToAoiRect(chip.chipImageRect);

                PanelDieCoordinate coord = mapGroupChipToPanelDie(
                    group.row, group.col, item.chipType, list.chipGroupRowCount,
                    list.chipGroupColCount, arrangement);
                item.rowIndexInPanel    = coord.panelGroupRow;
                item.colIndexInPanel    = coord.panelGroupCol;
                item.dieRowIndexInPanel = coord.dieRow;
                item.dieColIndexInPanel = coord.dieCol;

                if (chipMap.isValid()) {
                    if (auto binding = chipMap.findChip(group.row, group.col, ci)) {
                        coord = mapGroupChipToPanelDie(binding->groupRow, binding->groupCol, item.chipType,
                                                       list.chipGroupRowCount, list.chipGroupColCount,
                                                       arrangement);
                        item.rowIndexInPanel    = coord.panelGroupRow;
                        item.colIndexInPanel    = coord.panelGroupCol;
                        item.dieRowIndexInPanel = coord.dieRow;
                        item.dieColIndexInPanel = coord.dieCol;
                    } else if (chip.chipCoordinate.x != 0.0f || chip.chipCoordinate.y != 0.0f) {
                        const int imageIdx = fov.fovIndex > 0 ? fov.fovIndex - 1 : 0;
                        if (auto binding = chipMap.findChipByImage(
                                imageIdx, chip.chipCoordinate.x, chip.chipCoordinate.y)) {
                            coord = mapGroupChipToPanelDie(binding->groupRow, binding->groupCol,
                                                           item.chipType, list.chipGroupRowCount,
                                                           list.chipGroupColCount, arrangement);
                            item.rowIndexInPanel    = coord.panelGroupRow;
                            item.colIndexInPanel    = coord.panelGroupCol;
                            item.dieRowIndexInPanel = coord.dieRow;
                            item.dieColIndexInPanel = coord.dieCol;
                        }
                    }
                }

                partition.defectItems.append(item);
            }
        }

        if (!partition.defectItems.isEmpty())
            list.defectPartitions.append(partition);
    }

    return list;
}

} // namespace

DefectInfoList buildAdHocDefectInfoList(const FovDetectRecord &fov, const RecipeDocument &recipe,
                                        const aoi::DefectSettings &defects,
                                        const QString &originImagePath,
                                        const QString &detectImagePath,
                                        const QString &serialNumber)
{
    DefectInfoList list;
    list.detectTime      = QDateTime::currentDateTime().toString(Qt::ISODate);
    list.validDefectType = buildValidDefectTypeMask(defects);
    list.append          = false;
    list.productName     = recipe.basic.name;
    list.SN              = serialNumber.isEmpty()
                               ? QStringLiteral("VI-%1")
                                     .arg(QDateTime::currentDateTime().toString(
                                         QStringLiteral("yyyyMMdd-hhmmss")))
                               : serialNumber;
    list.chipGroupRowCount = recipe.basic.width;
    list.chipGroupColCount = recipe.basic.height;

    const auto arrangement =
        static_cast<aoi::fov_planner::FovChipArrangement>(recipe.basic.chipArrangement);
    const PanelDieCoordinate panelScale = mapGroupChipToPanelDie(
        0, 0, 0, list.chipGroupRowCount, list.chipGroupColCount, arrangement);
    list.dieRowCount = panelScale.dieRowCount;
    list.dieColCount = panelScale.dieColCount;

    const fov_planner::FovPlanGenerateResult planResult =
        fov_planner::FovPlanService::generate(recipe);
    const fov_planner::FovChipIndexMap *chipMap =
        planResult.ok ? &planResult.plan.chipMap : nullptr;

    DefectInfoPartition partition;
    partition.partitionIndex           = fov.fovIndex > 0 ? fov.fovIndex - 1 : 0;
    partition.partitionGridRow         = 0;
    partition.partitionGridCol         = 0;
    partition.partitionImageOriginName = originImagePath;
    partition.partitionImageDetectName =
        detectImagePath.isEmpty() ? originImagePath : detectImagePath;

    for (const vision_algo::ChipGroupDetectInfo &group : fov.groups) {
        for (int ci = 0; ci < 3; ++ci) {
            const vision_algo::ChipDetectInfo &chip = group.chipRgbInfo[ci];
            const InspectionDefectTypeMask defectMask =
                chipDefectToInspectionMask(chip.chipDefect);
            if (defectMask == 0)
                continue;

            DefectInfoItem item;
            item.chipType            = static_cast<int>(chip.chipType);
            item.defectType          = defectMask;
            item.rejudgeDefectType   = 0;
            item.isRejudge           = false;
            item.rowIndexInPartition = group.row;
            item.colIndexInPartition = group.col;
            item.roi                 = algoRectToAoiRect(chip.chipImageRect);

            PanelDieCoordinate coord = mapGroupChipToPanelDie(
                group.row, group.col, item.chipType, list.chipGroupRowCount,
                list.chipGroupColCount, arrangement);
            item.rowIndexInPanel    = coord.panelGroupRow;
            item.colIndexInPanel    = coord.panelGroupCol;
            item.dieRowIndexInPanel = coord.dieRow;
            item.dieColIndexInPanel = coord.dieCol;

            if (chipMap && chipMap->isValid()) {
                if (auto binding = chipMap->findChip(group.row, group.col, ci)) {
                    coord = mapGroupChipToPanelDie(binding->groupRow, binding->groupCol,
                                                   item.chipType, list.chipGroupRowCount,
                                                   list.chipGroupColCount, arrangement);
                    item.rowIndexInPanel    = coord.panelGroupRow;
                    item.colIndexInPanel    = coord.panelGroupCol;
                    item.dieRowIndexInPanel = coord.dieRow;
                    item.dieColIndexInPanel = coord.dieCol;
                } else if (chip.chipCoordinate.x != 0.0f || chip.chipCoordinate.y != 0.0f) {
                    const int imageIdx = fov.fovIndex > 0 ? fov.fovIndex - 1 : 0;
                    if (auto binding = chipMap->findChipByImage(imageIdx, chip.chipCoordinate.x,
                                                              chip.chipCoordinate.y)) {
                        coord = mapGroupChipToPanelDie(binding->groupRow, binding->groupCol,
                                                       item.chipType, list.chipGroupRowCount,
                                                       list.chipGroupColCount, arrangement);
                        item.rowIndexInPanel    = coord.panelGroupRow;
                        item.colIndexInPanel    = coord.panelGroupCol;
                        item.dieRowIndexInPanel = coord.dieRow;
                        item.dieColIndexInPanel = coord.dieCol;
                    }
                }
            }

            partition.defectItems.append(item);
        }
    }

    if (!partition.defectItems.isEmpty())
        list.defectPartitions.append(partition);

    return list;
}

aoi::CurrentInspectionSnapshot buildSnapshotFromDefectInfoList(const DefectInfoList &list,
                                                               const aoi::DefectSettings &defects)
{
    aoi::CurrentInspectionSnapshot snapshot;
    snapshot.serialNumber = list.SN;

    QMap<InspectionDefectType, int> defectTotals;
    QMap<InspectionDefectType, int> defectR;
    QMap<InspectionDefectType, int> defectG;
    QMap<InspectionDefectType, int> defectB;
    for (int k = 0; k < kInspectionDefectTypeCount; ++k) {
        const auto type = static_cast<InspectionDefectType>(k);
        defectTotals[type] = 0;
        defectR[type]      = 0;
        defectG[type]      = 0;
        defectB[type]      = 0;
    }

    for (const DefectInfoPartition &partition : list.defectPartitions) {
        for (const DefectInfoItem &item : partition.defectItems) {
            for (int i = 0; i < kInspectionDefectTypeCount; ++i) {
                const auto type = static_cast<InspectionDefectType>(i);
                if (!inspectionMaskHasDefect(item.defectType, type))
                    continue;
                defectTotals[type]++;
                if (item.chipType == 0)
                    defectR[type]++;
                else if (item.chipType == 1)
                    defectG[type]++;
                else if (item.chipType == 2)
                    defectB[type]++;
            }
        }
    }

    int totalDefectCount = 0;
    for (int k = 0; k < kInspectionDefectTypeCount; ++k)
        totalDefectCount += defectTotals[static_cast<InspectionDefectType>(k)];

    const int totalLimit = defects.total.upper;
    snapshot.overallResult = totalDefectCount <= totalLimit ? aoi::InspectionOverallResult::Ok
                                                            : aoi::InspectionOverallResult::Ng;
    snapshot.rows.clear();
    snapshot.rows.reserve(kInspectionDefectTypeCount);

    for (int k = 0; k < kInspectionDefectTypeCount; ++k) {
        const auto type  = static_cast<InspectionDefectType>(k);
        const int  total = defectTotals[type];
        const auto rowResult =
            total == 0 ? aoi::InspectionOverallResult::Ok : aoi::InspectionOverallResult::Ng;
        snapshot.rows.append(
            makeRow(type, defectR[type], defectG[type], defectB[type], total, rowResult));
    }

    return snapshot;
}

PanelInspectionResult aggregatePanelResult(const InspectionSession &session)
{
    PanelInspectionResult result;
    result.serialNumber =
        QStringLiteral("SN-%1").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss")));
    result.totalFovCount = session.fovCaptures.size();

    QMap<InspectionDefectType, int> defectTotals;
    for (int k = 0; k < kInspectionDefectTypeCount; ++k)
        defectTotals[static_cast<InspectionDefectType>(k)] = 0;

    QMap<InspectionDefectType, int> defectR;
    QMap<InspectionDefectType, int> defectG;
    QMap<InspectionDefectType, int> defectB;

    for (const FovDetectRecord &fov : session.fovDetects) {
        if (fov.ok)
            result.okFovCount++;
        else
            result.ngFovCount++;

        for (const vision_algo::ChipGroupDetectInfo &group : fov.groups) {
            for (int ci = 0; ci < 3; ++ci) {
                const vision_algo::ChipDetectInfo &chip = group.chipRgbInfo[ci];
                QMap<InspectionDefectType, int> *channelMap = nullptr;
                if (ci == 0)
                    channelMap = &defectR;
                else if (ci == 1)
                    channelMap = &defectG;
                else
                    channelMap = &defectB;

                const InspectionDefectTypeMask mask = chipDefectToInspectionMask(chip.chipDefect);
                countInspectionDefects(mask, defectTotals);
                if (mask != 0 && channelMap) {
                    QMap<InspectionDefectType, int> tmp;
                    countInspectionDefects(mask, tmp);
                    for (auto it = tmp.begin(); it != tmp.end(); ++it)
                        (*channelMap)[it.key()] += it.value();
                }
            }
        }
    }

    result.totalDefectCount = 0;
    for (int k = 0; k < kInspectionDefectTypeCount; ++k) {
        const auto type = static_cast<InspectionDefectType>(k);
        result.totalDefectCount += defectTotals[type];
    }

    const int totalLimit = session.sysConfig.defects.total.upper;
    result.overallResult = (result.totalDefectCount <= totalLimit && result.ngFovCount == 0)
                               ? aoi::InspectionOverallResult::Ok
                               : aoi::InspectionOverallResult::Ng;

    result.snapshot.serialNumber  = result.serialNumber;
    result.snapshot.overallResult = result.overallResult;
    result.snapshot.rows.clear();
    result.snapshot.rows.reserve(kInspectionDefectTypeCount);

    for (int k = 0; k < kInspectionDefectTypeCount; ++k) {
        const auto type  = static_cast<InspectionDefectType>(k);
        const int  total = defectTotals[type];
        const int  r     = defectR.value(type, 0);
        const int  g     = defectG.value(type, 0);
        const int  b     = defectB.value(type, 0);
        const auto rowResult =
            total == 0 ? aoi::InspectionOverallResult::Ok : aoi::InspectionOverallResult::Ng;
        result.snapshot.rows.append(makeRow(type, r, g, b, total, rowResult));
    }

    result.defectInfoList = buildDefectInfoList(session, result.serialNumber);

    return result;
}

scheduler::StepResult runAggregateResultsService(const std::shared_ptr<InspectionSession> &session,
                                                 scheduler::FlowContext &ctx)
{
    Q_UNUSED(ctx);

    if (!session)
        return scheduler::StepResult::fail("inspection session missing");

    session->panelResult = aggregatePanelResult(*session);

    QString historyError;
#if COMMON_ORM_ENABLED
    const qint64 historyId =
        InspectionHistoryRepository::instance().appendRecord(session->panelResult.defectInfoList, &historyError);
    if (historyId > 0) {
        session->historyRecordId = historyId;
        InspectionHistoryCoordinator::instance().setLastAppendedRecordId(historyId);
    } else if (!historyError.isEmpty()) {
        qWarning() << "[InspectionHistory] append failed:" << historyError;
    }
#endif

    session->progressMessage =
        QStringLiteral("板级汇总: %1, FOV OK/NG=%2/%3, 缺陷总数=%4")
            .arg(session->panelResult.overallResult == aoi::InspectionOverallResult::Ok
                     ? QStringLiteral("OK")
                     : QStringLiteral("NG"))
            .arg(session->panelResult.okFovCount)
            .arg(session->panelResult.ngFovCount)
            .arg(session->panelResult.totalDefectCount);
    reportSessionLog(session, session->progressMessage);

    if (session->progressBridge) {
        session->progressBridge->reportInspectionSnapshot(session->panelResult.snapshot);
        session->progressBridge->reportReviewSession(session->panelResult.defectInfoList);
    }

    return scheduler::StepResult::ok();
}

} // namespace aoi::inspection
