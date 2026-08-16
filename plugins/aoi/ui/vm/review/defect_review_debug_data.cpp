/**
 * @file defect_review_debug_data.cpp
 */

#include "defect_review_debug_data.h"

#include <QDateTime>
#include <QDir>

namespace aoi::review {
namespace {

constexpr int kChipGroupRows = 100;
constexpr int kChipGroupCols = 160;

/**
 * @brief 芯片组坐标 + chipType 展开为晶粒坐标（与 `inspection_result_service` 规则一致）
 */
QPair<int, int> debugDieCoord(int groupRow, int groupCol, int chipType, bool verticalArrangement)
{
    if (verticalArrangement)
        return {groupRow * 3 + chipType, groupCol};
    return {groupRow, groupCol * 3 + chipType};
}

} // namespace

inspection::DefectInfoList buildReviewDebugDefectInfoList(bool verticalArrangement)
{
    using namespace aoi::inspection;

    DefectInfoList list;
    list.detectTime        = QDateTime::currentDateTime().toString(Qt::ISODate);
    list.productName       = QStringLiteral("Review Bridge Debug");
    list.SN                = QStringLiteral("SN-REVIEW-DEBUG");
    list.chipGroupRowCount = kChipGroupRows;
    list.chipGroupColCount = kChipGroupCols;
    list.dieRowCount       = verticalArrangement ? kChipGroupRows * 3 : kChipGroupRows;
    list.dieColCount       = verticalArrangement ? kChipGroupCols : kChipGroupCols * 3;
    list.validDefectType   = 0x7FFu;

    const QString debugDir =
        QDir(QDir::tempPath()).filePath(QStringLiteral("aoi_review_debug"));
    list.defectInfoFullFileName = QDir(debugDir).filePath(QStringLiteral("defect_info.json"));

    DefectInfoPartition part;
    part.partitionIndex           = 0;
    part.partitionGridRow         = 0;
    part.partitionGridCol         = 0;
    part.partitionImageOriginName = QString();
    part.partitionImageDetectName = QString();

    auto addItem = [&](int groupRow, int groupCol, int chipType, InspectionDefectTypeMask defectMask,
                       bool isRejudge, InspectionDefectTypeMask rejudgeMask) {
        const auto [dieRow, dieCol] = debugDieCoord(groupRow, groupCol, chipType, verticalArrangement);

        DefectInfoItem item;
        item.chipType            = chipType;
        item.defectType          = defectMask;
        item.rejudgeDefectType   = rejudgeMask;
        item.isRejudge           = isRejudge;
        item.rowIndexInPartition = groupRow;
        item.colIndexInPartition = groupCol;
        item.rowIndexInPanel     = groupRow;
        item.colIndexInPanel     = groupCol;
        item.dieRowIndexInPanel  = dieRow;
        item.dieColIndexInPanel  = dieCol;
        item.roi                 = aoi::Rect{32 + groupCol, 24 + groupRow, 48, 48};
        part.defectItems.append(item);
    };

    // 左上角三连点：待复判 / 复判 OK / 复判 NG
    addItem(0, 0, 0, inspectionDefectTypeBit(InspectionDefectType::MissingChip), false, 0);
    addItem(0, 0, 1, inspectionDefectTypeBit(InspectionDefectType::ChipOffset), true, 0);
    addItem(0, 0, 2, inspectionDefectTypeBit(InspectionDefectType::Reversed), true,
            inspectionDefectTypeBit(InspectionDefectType::ForeignObject));

    // 中部多缺陷叠加
    addItem(48, 80, 0, inspectionDefectTypeBit(InspectionDefectType::Damage), false, 0);
    addItem(48, 80, 1, inspectionDefectTypeBit(InspectionDefectType::ExcessChip), false, 0);
    addItem(48, 80, 2, inspectionDefectTypeBit(InspectionDefectType::InsufficientChip), true,
            inspectionDefectTypeBit(InspectionDefectType::InsufficientChip));

    // 右下角边界点
    addItem(kChipGroupRows - 1, kChipGroupCols - 1, 1,
            inspectionDefectTypeBit(InspectionDefectType::Tombstone), false, 0);

    part.defectItems.squeeze();
    list.defectPartitions.append(part);
    return list;
}

} // namespace aoi::review
