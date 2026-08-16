#pragma once

/**
 * @file inspection_types.h
 * @brief AOI 检测共享数据定义：缺陷类型枚举/位掩码、缺陷明细结构、FOV 记录
 *
 * 路径：plugins/aoi/data/dataDefinition/
 * InspectionDefectType 为缺陷类型**唯一入口**（配置键、UI 翻译键、位掩码序号一致）。
 * DefectInfoItem / DefectInfoPartition / DefectInfoList 字段与板级缺陷 protobuf 对齐。
 * `aoi::CurrentInspectionSnapshot` 等别名供 service / UI 共用，避免依赖 VM 头文件。
 */

#include "param_define.h"

#include <QString>
#include <QVector>

#include <cstdint>
#include <memory>
#include <vector>

namespace common::device {
struct ImageInfo;
}

#ifndef Q_MOC_RUN
#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>
#else
namespace vision_algo {
struct ChipGroupDetectInfo;
}
#endif

namespace aoi {

/** 单板或单项检测汇总结果 */
enum class InspectionOverallResult
{
    Ok,      ///< 合格
    Ng,      ///< 不合格
    Unknown, ///< 尚未判定
};

} // namespace aoi

namespace aoi::inspection {

/**
 * 检测缺陷类型（序号即位掩码 bit 索引，禁止调整已有数值；
 * 与 aoi.recipe.defect.* 翻译键一一对应）
 */
enum class InspectionDefectType
{
    MissingChip      = 0,  ///< 漏固
    ChipOffset       = 1,  ///< 偏移
    WrongComponent   = 2,  ///< 固错
    Reversed         = 3,  ///< 反向
    Flipped          = 4,  ///< 翻转
    Tombstone        = 5,  ///< 立碑
    Duplicate        = 6,  ///< 重复
    Damage           = 7,  ///< 破损
    ForeignObject    = 8,  ///< 异物
    ExcessChip       = 9,  ///< 多固
    InsufficientChip = 10, ///< 少固
};

/** 缺陷类型枚举项数量 */
constexpr int kInspectionDefectTypeCount = 11;

/** 缺陷位掩码（每位对应一种 InspectionDefectType） */
using InspectionDefectTypeMask = uint32_t;

/** 缺陷类型枚举 → 位掩码（bit = static_cast<uint32_t>(type)） */
inline InspectionDefectTypeMask inspectionDefectTypeBit(InspectionDefectType type)
{
    return 1u << static_cast<uint32_t>(type);
}

/** JSON / 配置持久化键（missing_chip 等） */
QString inspectionDefectTypeStorageKey(InspectionDefectType type);

/** 自持久化键解析缺陷类型 */
bool inspectionDefectTypeFromStorageKey(const QString &key, InspectionDefectType *out);

/** UI 翻译键（aoi.recipe.defect.*） */
QString inspectionDefectTypeDisplayKey(InspectionDefectType type);

/** CISDK chip_defect → InspectionDefectType 位掩码 */
InspectionDefectTypeMask chipDefectToInspectionMask(int chipDefect);

/** DefectSettings JSON 中 total 阈值键（非 InspectionDefectType 项） */
inline constexpr const char kDefectSettingsTotalStorageKey[] = "total";

/** 检测 mask 是否包含指定缺陷类型 */
inline bool inspectionMaskHasDefect(InspectionDefectTypeMask mask, InspectionDefectType type)
{
    return (mask & inspectionDefectTypeBit(type)) != 0u;
}

/** 芯片 RGB 通道，与 DefectInfoItem.chipType / DieBondingChipType 一致（0=R, 1=G, 2=B） */
enum class InspectionChipType : int
{
    R = 0, ///< 红色通道
    G = 1, ///< 绿色通道
    B = 2, ///< 蓝色通道
};

/** 单颗芯片缺陷项（对应 DefectInfoItem / 持久化 protobuf） */
struct DefectInfoItem
{
    int                      chipType            = 0; ///< RGB 通道（InspectionChipType）
    InspectionDefectTypeMask defectType          = 0; ///< 算法检出缺陷位掩码
    InspectionDefectTypeMask rejudgeDefectType   = 0; ///< 复判后缺陷位掩码
    bool                     isRejudge           = false; ///< 是否已人工复判
    int                      colIndexInPartition = 0; ///< 分区内列索引
    int                      rowIndexInPartition = 0; ///< 分区内行索引
    int                      colIndexInPanel     = 0; ///< 整板列索引
    int                      rowIndexInPanel     = 0; ///< 整板行索引
    int                      dieColIndexInPanel  = 0; ///< Die 级列索引
    int                      dieRowIndexInPanel  = 0; ///< Die 级行索引
    aoi::Rect                roi;                     ///< 缺陷 ROI（像素）
};

/** 单个分区（FOV/子图）的缺陷明细集合 */
struct DefectInfoPartition
{
    QVector<DefectInfoItem> defectItems;              ///< 本分区缺陷列表
    int                     partitionIndex           = 0; ///< 分区序号
    int                     partitionGridRow         = 0; ///< 分区在板上网格行
    int                     partitionGridCol         = 0; ///< 分区在板上网格列
    QString                 partitionImageOriginName;   ///< 原图文件名
    QString                 partitionImageDetectName;   ///< 检测图文件名
};

/** 整板缺陷信息列表（落盘 / MES 用） */
struct DefectInfoList
{
    QVector<DefectInfoPartition> defectPartitions;      ///< 各分区缺陷
    QString                      defectInfoFullFileName; ///< 缺陷信息文件路径
    QString                      detectTime;             ///< 检测时间字符串
    InspectionDefectTypeMask     validDefectType = 0;  ///< 配方启用的缺陷类型掩码
    bool                         append          = false; ///< 是否追加写入
    QString                      productName;            ///< 产品名
    QString                      SN;                     ///< 板级序列号
    int                          chipGroupRowCount = 0;  ///< 芯片组行数
    int                          chipGroupColCount = 0;  ///< 芯片组列数
    int                          dieRowCount         = 0; ///< Die 行数
    int                          dieColCount         = 0; ///< Die 列数
};

/** 单张 FOV 采图记录 */
struct FovCaptureRecord
{
    int     fovIndex   = 0;   ///< FOV 序号
    int     gridRow    = 0;   ///< 板上网格行
    int     gridCol    = 0;   ///< 板上网格列
    double  centerXmm  = 0.0; ///< 中心 X（毫米）
    double  centerYmm  = 0.0; ///< 中心 Y（毫米）
    /** 工步原图（Bayer/Mono） */
    std::shared_ptr<common::device::ImageInfo> rawImage;
    /** 单次 debayer 后的 RGB，供检测/预览共享 */
    std::shared_ptr<common::device::ImageInfo> processImage;
    QString diskPath; ///< 可选：磁盘缓存路径
};

/** 单张 FOV 算法检测结果 */
struct FovDetectRecord
{
    int                                  fovIndex = 0; ///< FOV 序号
    bool                                 ok       = false; ///< 本 FOV 是否通过
    QString                              error;            ///< 失败时的错误说明
    std::vector<vision_algo::ChipGroupDetectInfo> groups;  ///< 芯片组检测明细
    int                                  groupNum = 0;     ///< 有效芯片组数量
};

/** 缺陷统计一行：缺陷类型、RGB 分区计数、合计、该项判定 */
struct InspectionSnapshotRow
{
    InspectionDefectType    kind   = InspectionDefectType::MissingChip; ///< 缺陷类型
    int                     r      = -1; ///< R 区计数，-1 表示未适用
    int                     g      = -1; ///< G 区计数
    int                     b      = -1; ///< B 区计数
    int                     total  = -1; ///< 合计计数
    aoi::InspectionOverallResult result = aoi::InspectionOverallResult::Unknown; ///< 该项判定
};

/** 当前板检测快照：序列号、总判定、各缺陷行 */
struct CurrentInspectionSnapshot
{
    QString                      serialNumber; ///< 板级序列号
    InspectionOverallResult      overallResult = InspectionOverallResult::Unknown; ///< 总判定
    QVector<InspectionSnapshotRow> rows;       ///< 各缺陷类型统计行
};

/** 板级检测汇总（工步内聚合，最终映射到 UI snapshot 与 DefectInfoList） */
struct PanelInspectionResult
{
    QString                      serialNumber; ///< 板级序列号
    aoi::InspectionOverallResult overallResult = aoi::InspectionOverallResult::Unknown; ///< 总判定
    CurrentInspectionSnapshot    snapshot;     ///< UI 展示用快照
    DefectInfoList               defectInfoList; ///< 持久化缺陷明细
    int                          totalFovCount    = 0; ///< FOV 总数
    int                          okFovCount       = 0; ///< 通过 FOV 数
    int                          ngFovCount       = 0; ///< NG FOV 数
    int                          totalDefectCount = 0; ///< 缺陷颗数合计
};

} // namespace aoi::inspection

namespace aoi {
using InspectionSnapshotRow     = inspection::InspectionSnapshotRow;
using CurrentInspectionSnapshot = inspection::CurrentInspectionSnapshot;
} // namespace aoi

Q_DECLARE_METATYPE(aoi::inspection::DefectInfoList)
