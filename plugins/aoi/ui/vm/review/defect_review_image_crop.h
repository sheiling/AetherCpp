#pragma once

/**
 * @file defect_review_image_crop.h
 * @brief 复判区图像：从分区路径 + DefectInfoItem::roi 裁切晶粒原图/结果图
 *
 * 数据约定（由检测汇总填充，本模块不负责落盘）：
 * - `DefectInfoPartition::partitionImageOriginName` — 分区原图完整路径
 * - `DefectInfoPartition::partitionImageDetectName` — 分区结果图完整路径（须非空方可展示结果图）
 * - `DefectInfoItem::roi` — 缺陷芯片在分区图像上的像素矩形
 */

#include "inspection_types.h"

#include <QImage>
#include <QString>

namespace aoi::review {

/** 单颗晶粒缺陷项对应的图像来源（分区路径 + ROI） */
struct DefectChipImageSource
{
    QString   originPartitionPath;
    QString   detectPartitionPath;
    aoi::Rect roi;

    bool isValid() const
    {
        return !originPartitionPath.trimmed().isEmpty();
    }
};

/**
 * @brief 从 workingCopy 解析指定 defectItem 的图像来源
 * @return 索引非法或路径为空时返回默认构造（无效）
 */
DefectChipImageSource resolveChipImageSource(const inspection::DefectInfoList &workingCopy,
                                             int partitionIndex, int itemIndex);

/**
 * @brief 按分区图路径与 ROI 裁切
 *
 * 路径为空、ROI 无效、ROI 越界或读图失败时记 warning 并返回空图；**不**回退整幅分区图。
 */
QImage cropPartitionImage(const QString &path, const aoi::Rect &roi);

/** 结果图裁切：`path` 为空时 warning 并返回空图（不回退原图） */
QImage cropDetectPartitionImage(const QString &path, const aoi::Rect &roi);

} // namespace aoi::review
