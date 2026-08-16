#pragma once

/**
 * @file defect_review_types.h
 * @brief 缺陷复判 Widget 共享类型：晶粒键、网格、筛选、缺陷色与 F 键映射
 *
 * 设计见 `plugins/aoi/docs/AOI 检测软件 设计方案.md` §3.1.4。
 * MAP 一格对应一颗芯片（晶粒）；`LampKey` 为晶粒行列主键。
 */

#include "inspection_types.h"

#include <QColor>
#include <QMetaType>
#include <QVector>

#include <optional>

namespace aoi::review {

/** 晶粒网格规模（MAP 底图行列） */
struct DefectReviewDieGrid
{
    int rowCount = 0;
    int colCount = 0;

    bool isValid() const { return rowCount > 0 && colCount > 0; }
};

/** MAP / 坐标表灯点主键（晶粒索引） */
struct LampKey
{
    int dieRow = 0;
    int dieCol = 0;

    bool operator==(const LampKey &other) const
    {
        return dieRow == other.dieRow && dieCol == other.dieCol;
    }
};

inline bool operator!=(const LampKey &a, const LampKey &b)
{
    return !(a == b);
}

/** 晶粒布局快照：行列与 RGB 类型均来自 `DefectInfoItem`（复判 MAP 必填） */
struct DefectDieChipInfo
{
    int dieRowIndexInPanel = -1; ///< DefectInfoItem::dieRowIndexInPanel
    int dieColIndexInPanel = -1; ///< DefectInfoItem::dieColIndexInPanel
    int chipType           = 0;  ///< DefectInfoItem::chipType

    LampKey lampKey() const { return LampKey{dieRowIndexInPanel, dieColIndexInPanel}; }
};

/** 指向 workingCopy 中可写回位置的句柄 */
struct DefectItemRef
{
    int partitionIndex = -1;
    int itemIndex      = -1;
};

/**
 * MAP / 列表灯点运行时聚合；图像与 ROI 经 `itemRefs` → `workingCopy` 解析，不在此缓存。
 */
struct DefectReviewLampRecord
{
    LampKey                   key;
    int                       groupRow            = 0;
    int                       groupCol            = 0;
    int                       chipType            = 0;
    inspection::InspectionDefectTypeMask defectType         = 0;
    inspection::InspectionDefectTypeMask rejudgeDefectType  = 0;
    bool                      isRejudge           = false;
    bool                      inDieGridBounds     = true;
    QVector<DefectItemRef>    itemRefs;
    /** F 键切换顺序；MAP 同心标记仅取末 kMaxMapMarkerCount 项 */
    QVector<inspection::InspectionDefectType> rejudgeMarkerOrder;
};

/** MAP 同心方格最多绘制层数 */
constexpr int kMaxMapMarkerCount = 3;

/** 每颗物理灯珠行内芯片数（R/G/B）；晶粒行 = 灯珠行 × 本常量 + chipType */
constexpr int kChipsPerLampRow = 3;

/** 按切换顺序（无顺序时按枚举序）取可见缺陷类型的末 N 项，供 MAP 绘制 */
QVector<inspection::InspectionDefectType> markerTypesForMapDraw(
    const DefectReviewLampRecord &rec,
    inspection::InspectionDefectTypeMask visibleTypeMask,
    int maxCount = kMaxMapMarkerCount);

/** 11 类缺陷在 MAP / 侧栏的默认色（对齐原型） */
QColor defectTypeColor(inspection::InspectionDefectType type);

/** F1→index0 … F11→index10；非法返回 nullopt */
std::optional<inspection::InspectionDefectType> defectTypeForFunctionKey(int keyIndex);

/** 侧栏展示：缺陷名 + 快捷键，如「漏固(F1)」 */
QString defectTypeFilterLabel(inspection::InspectionDefectType type);

/** 侧栏仅缺陷名（无快捷键后缀） */
QString defectTypeSidebarLabel(inspection::InspectionDefectType type);

/** 复判 OK 色块（原型图 2 顶部绿色） */
QColor okSwatchColor();

/** 芯片 RGB 通道文案：R / G / B */
QString chipTypeLabel(int chipType);

} // namespace aoi::review

Q_DECLARE_METATYPE(aoi::review::LampKey)

inline size_t qHash(const aoi::review::LampKey &key, uint seed = 0)
{
    return ::qHash(key.dieRow, seed) ^ ::qHash(key.dieCol);
}
