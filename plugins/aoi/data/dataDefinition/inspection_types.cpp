#include "inspection_types.h"

namespace aoi::inspection {

QString inspectionDefectTypeStorageKey(InspectionDefectType type)
{
    switch (type) {
    case InspectionDefectType::MissingChip:
        return QStringLiteral("missing_chip");
    case InspectionDefectType::ChipOffset:
        return QStringLiteral("chip_offset");
    case InspectionDefectType::WrongComponent:
        return QStringLiteral("wrong_component");
    case InspectionDefectType::Reversed:
        return QStringLiteral("reversed");
    case InspectionDefectType::Flipped:
        return QStringLiteral("flipped");
    case InspectionDefectType::Tombstone:
        return QStringLiteral("tombstone");
    case InspectionDefectType::Duplicate:
        return QStringLiteral("duplicate");
    case InspectionDefectType::Damage:
        return QStringLiteral("damage");
    case InspectionDefectType::ForeignObject:
        return QStringLiteral("foreign_object");
    case InspectionDefectType::ExcessChip:
        return QStringLiteral("excess_chip");
    case InspectionDefectType::InsufficientChip:
        return QStringLiteral("insufficient_chip");
    }
    return QStringLiteral("unknown");
}

bool inspectionDefectTypeFromStorageKey(const QString &key, InspectionDefectType *out)
{
    if (!out || key.isEmpty())
        return false;

    const auto match = [&](const char *chipKey, InspectionDefectType type) -> bool {
        if (key == QLatin1String(chipKey)) {
            *out = type;
            return true;
        }
        return false;
    };

    if (match("missing_chip", InspectionDefectType::MissingChip))
        return true;
    if (match("chip_offset", InspectionDefectType::ChipOffset))
        return true;
    if (match("wrong_component", InspectionDefectType::WrongComponent))
        return true;
    if (match("reversed", InspectionDefectType::Reversed))
        return true;
    if (match("flipped", InspectionDefectType::Flipped))
        return true;
    if (match("tombstone", InspectionDefectType::Tombstone))
        return true;
    if (match("duplicate", InspectionDefectType::Duplicate))
        return true;
    if (match("damage", InspectionDefectType::Damage))
        return true;
    if (match("foreign_object", InspectionDefectType::ForeignObject))
        return true;
    if (match("excess_chip", InspectionDefectType::ExcessChip))
        return true;
    if (match("insufficient_chip", InspectionDefectType::InsufficientChip))
        return true;

    return false;
}

QString inspectionDefectTypeDisplayKey(InspectionDefectType type)
{
    switch (type) {
    case InspectionDefectType::MissingChip:
        return QStringLiteral("aoi.recipe.defect.missing_chip");
    case InspectionDefectType::ChipOffset:
        return QStringLiteral("aoi.recipe.defect.chip_offset");
    case InspectionDefectType::WrongComponent:
        return QStringLiteral("aoi.recipe.defect.wrong_component");
    case InspectionDefectType::Reversed:
        return QStringLiteral("aoi.recipe.defect.reversed");
    case InspectionDefectType::Flipped:
        return QStringLiteral("aoi.recipe.defect.flipped");
    case InspectionDefectType::Tombstone:
        return QStringLiteral("aoi.recipe.defect.tombstone");
    case InspectionDefectType::Duplicate:
        return QStringLiteral("aoi.recipe.defect.duplicate");
    case InspectionDefectType::Damage:
        return QStringLiteral("aoi.recipe.defect.damage");
    case InspectionDefectType::ForeignObject:
        return QStringLiteral("aoi.recipe.defect.foreign_object");
    case InspectionDefectType::ExcessChip:
        return QStringLiteral("aoi.recipe.defect.excess_chip");
    case InspectionDefectType::InsufficientChip:
        return QStringLiteral("aoi.recipe.defect.insufficient_chip");
    }
    return QStringLiteral("aoi.recipe.defect.unknown");
}

InspectionDefectTypeMask chipDefectToInspectionMask(int chipDefect)
{
    // 与 CISDK DieBondingDefectType 位定义一致（值固化于 AOI，不 include CISDK）
    constexpr int kChipMissing         = 0x00000001;
    constexpr int kChipDirty           = 0x00000002;
    constexpr int kChipBroken          = 0x00000004;
    constexpr int kSolderForeignObject = 0x00000008;
    constexpr int kChipBondingError    = 0x00000020;

    InspectionDefectTypeMask mask = 0;
    if (chipDefect & kChipMissing)
        mask |= inspectionDefectTypeBit(InspectionDefectType::MissingChip);
    if (chipDefect & kChipDirty)
        mask |= inspectionDefectTypeBit(InspectionDefectType::ForeignObject);
    if (chipDefect & kChipBroken)
        mask |= inspectionDefectTypeBit(InspectionDefectType::Damage);
    if (chipDefect & kSolderForeignObject)
        mask |= inspectionDefectTypeBit(InspectionDefectType::ForeignObject);
    if (chipDefect & kChipBondingError)
        mask |= inspectionDefectTypeBit(InspectionDefectType::WrongComponent);
    if (chipDefect & (1 << 13))
        mask |= inspectionDefectTypeBit(InspectionDefectType::InsufficientChip);
    return mask;
}

} // namespace aoi::inspection
