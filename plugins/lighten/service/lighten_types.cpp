/**
 * @file lighten_types.cpp
 * @brief 点亮插件枚举到显示文本的转换实现
 */

#include "lighten_types.h"

/** @brief 点亮插件枚举与界面显示文本互转实现 */
namespace lighten {

QString inspectionResultText(InspectionResult r)
{
    switch (r) {
    case InspectionResult::Ok: return QStringLiteral("OK");
    case InspectionResult::Ng: return QStringLiteral("NG");
    case InspectionResult::Rw: return QStringLiteral("RW");
    }
    return QStringLiteral("—");
}

QString flashResultText(int flashResultCode)
{
    switch (flashResultCode) {
    case 0: return QStringLiteral("OK");
    case 1: return QStringLiteral("NG");
    }
    return QStringLiteral("—");
}

/** 运行模式 / 设备状态 / 硬件连接状态 */
QString runModeText(RunMode m)
{
    return m == RunMode::Auto ? QStringLiteral("AUTO") : QStringLiteral("MANUAL");
}

QString deviceStateText(DeviceState s)
{
    switch (s) {
    case DeviceState::Run:   return QStringLiteral("RUN");
    case DeviceState::Stop:  return QStringLiteral("STOP");
    case DeviceState::Idle:  return QStringLiteral("IDLE");
    case DeviceState::Alarm: return QStringLiteral("ALARM");
    }
    return QStringLiteral("—");
}

QString hardwareLinkStateText(HardwareLinkState s)
{
    switch (s) {
    case HardwareLinkState::Connected:    return QStringLiteral("已连接");
    case HardwareLinkState::Standby:      return QStringLiteral("待机");
    case HardwareLinkState::Disconnected: return QStringLiteral("断开");
    }
    return QStringLiteral("—");
}

/** 配方标签、检测项名称与颜色通道 */
QString recipeListTagText(RecipeListTag tag)
{
    if (tag == RecipeListTag::Default)
        return QStringLiteral("默认");
    return QString();
}

QString detectItemName(DetectItemKind kind)
{
    switch (kind) {
    case DetectItemKind::DeadLed:        return QStringLiteral("死灯");
    case DetectItemKind::DimLed:         return QStringLiteral("暗灯");
    case DetectItemKind::BrightLed:      return QStringLiteral("亮灯");
    case DetectItemKind::ColorVariation: return QStringLiteral("串色");
    case DetectItemKind::SeriesLed:      return QStringLiteral("串灯");
    case DetectItemKind::MixedColor:     return QStringLiteral("混色");
    case DetectItemKind::BlackScreen:    return QStringLiteral("黑屏检测");
    case DetectItemKind::BlackScreenOn:  return QStringLiteral("黑屏亮灯");
    case DetectItemKind::Flash:          return QStringLiteral("Flash");
    case DetectItemKind::ColorMapDead:   return QStringLiteral("串死");
    case DetectItemKind::ColorMapLive:   return QStringLiteral("串活");
    }
    return QString();
}

/** 首页检测项进度槽位索引与显示名 */
bool detectItemHasRgbProgress(DetectItemKind kind)
{
    return static_cast<int>(kind) <= static_cast<int>(DetectItemKind::MixedColor);
}

int homeProgressSlotIndex(DetectItemKind kind, int colorIdx)
{
    const int itemInt = static_cast<int>(kind);
    if (itemInt <= static_cast<int>(DetectItemKind::MixedColor)) {
        if (colorIdx < 0 || colorIdx > 2)
            return -1;
        return itemInt * 3 + colorIdx;
    }
    if (itemInt < static_cast<int>(DetectItemKind::BlackScreen)
        || itemInt > static_cast<int>(DetectItemKind::Flash)) {
        return -1;
    }
    return kHomeProgressRgbSlots + (itemInt - static_cast<int>(DetectItemKind::BlackScreen));
}

void decodeHomeProgressSlot(int slot, DetectItemKind *outKind, int *outColorIdx)
{
    if (!outKind || !outColorIdx)
        return;
    if (slot >= 0 && slot < kHomeProgressRgbSlots) {
        *outKind     = static_cast<DetectItemKind>(slot / 3);
        *outColorIdx = slot % 3;
        return;
    }
    if (slot >= kHomeProgressRgbSlots && slot < kHomeProgressCount) {
        *outKind = static_cast<DetectItemKind>(
            static_cast<int>(DetectItemKind::BlackScreen) + (slot - kHomeProgressRgbSlots));
        *outColorIdx = -1;
        return;
    }
    *outKind     = DetectItemKind::DeadLed;
    *outColorIdx = -1;
}

QString homeProgressItemName(DetectItemKind kind, ColorChannel ch)
{
    return colorChannelName(ch) + QStringLiteral("色") + detectItemName(kind);
}

bool detectItemHasTotalCount(DetectItemKind kind)
{
    switch (kind) {
    case DetectItemKind::BlackScreen:
    case DetectItemKind::Flash:
        return false;
    default:
        return true;
    }
}

QString colorChannelName(ColorChannel ch)
{
    static const char *kNames[] = {"红", "绿", "蓝", "黑", "白"};
    const int i = static_cast<int>(ch);
    if (i >= 0 && i <= static_cast<int>(ColorChannel::White))
        return QString::fromUtf8(kNames[i]);
    return QString();
}

/** 相机索引显示名 */
QString cameraName(CameraIndex cam)
{
    static const char *kNames[] = {"主相机", "前相机", "左相机", "右相机", "后相机"};
    const int i = static_cast<int>(cam);
    if (i >= 0 && i < kCameraCount)
        return QString::fromUtf8(kNames[i]);
    return QString();
}

} // namespace lighten
