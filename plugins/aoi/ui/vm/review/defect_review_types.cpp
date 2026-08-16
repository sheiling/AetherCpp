/**
 * @file defect_review_types.cpp
 * @brief 缺陷复判类型：缺陷色与 F 键映射实现
 */

#include "defect_review_types.h"

#include "component/theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <array>

namespace aoi::review {
namespace {

constexpr std::array<QColor, inspection::kInspectionDefectTypeCount> kDefectColors = {
    QColor(0xf5, 0x9e, 0x0b), // 漏固
    QColor(0xfa, 0xcc, 0x15), // 固偏
    QColor(0x84, 0xcc, 0x16), // 固错
    QColor(0xa7, 0x8b, 0xfa), // 固反
    QColor(0xf4, 0x72, 0xb6), // 固翻
    QColor(0x22, 0xd3, 0xee), // 侧立
    QColor(0xef, 0x44, 0x44), // 固重
    QColor(0x3b, 0x82, 0xf6), // 破损
    QColor(0x7d, 0xd3, 0xfc), // 异物
    QColor(0x1e, 0x40, 0xaf), // 多锡
    QColor(0xd9, 0x77, 0x06), // 少锡
};

} // namespace

QColor defectTypeColor(inspection::InspectionDefectType type)
{
    const int idx = static_cast<int>(type);
    if (idx < 0 || idx >= inspection::kInspectionDefectTypeCount)
        return QColor(0x94, 0xa3, 0xb8);
    return kDefectColors[static_cast<size_t>(idx)];
}

std::optional<inspection::InspectionDefectType> defectTypeForFunctionKey(int keyIndex)
{
    if (keyIndex < 0 || keyIndex >= inspection::kInspectionDefectTypeCount)
        return std::nullopt;
    return static_cast<inspection::InspectionDefectType>(keyIndex);
}

QString defectTypeSidebarLabel(inspection::InspectionDefectType type)
{
    return TR(inspection::inspectionDefectTypeDisplayKey(type));
}

QColor okSwatchColor()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    return t.successText.isValid() ? t.successText : ui::htmlFallbackSuccess();
}

QString chipTypeLabel(int chipType)
{
    switch (static_cast<inspection::InspectionChipType>(chipType)) {
    case inspection::InspectionChipType::R:
        return QStringLiteral("R");
    case inspection::InspectionChipType::G:
        return QStringLiteral("G");
    case inspection::InspectionChipType::B:
        return QStringLiteral("B");
    }
    return QStringLiteral("-");
}

QVector<inspection::InspectionDefectType> markerTypesForMapDraw(
    const DefectReviewLampRecord &rec,
    inspection::InspectionDefectTypeMask visibleTypeMask,
    int maxCount)
{
    if (maxCount <= 0)
        return {};

    const auto effectiveMask = (rec.defectType | rec.rejudgeDefectType) & visibleTypeMask;
    QVector<inspection::InspectionDefectType> ordered;

    const auto appendIfVisible = [&](inspection::InspectionDefectType type) {
        if (inspection::inspectionMaskHasDefect(effectiveMask, type))
            ordered.append(type);
    };

    if (!rec.rejudgeMarkerOrder.isEmpty()) {
        for (const inspection::InspectionDefectType type : rec.rejudgeMarkerOrder)
            appendIfVisible(type);
    } else {
        for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i)
            appendIfVisible(static_cast<inspection::InspectionDefectType>(i));
    }

    if (ordered.size() <= maxCount)
        return ordered;
    return ordered.mid(ordered.size() - maxCount);
}

QString defectTypeFilterLabel(inspection::InspectionDefectType type)
{
    const QString name = TR(inspection::inspectionDefectTypeDisplayKey(type));
    const int     fNum = static_cast<int>(type) + 1;
    return QStringLiteral("%1(F%2)").arg(name).arg(fNum);
}

} // namespace aoi::review
