#pragma once

/**
 * @file lighten_types.h
 * @brief 点亮插件公共枚举与显示文本转换函数
 */

#include <interface/ilighten_inspection_history_service.h>

#include <QString>
#include <QMetaType>

namespace lighten {

/** 检测结果（与首页 KPI / 历史筛选一致） */
using InspectionResult = ::LightenInspectionResult;

/** 设备运行模式（首页 KPI） */
enum class RunMode
{
    Auto   = 0,   ///< 自动模式
    Manual = 1,   ///< 手动模式
};

/** 设备运行状态（首页 KPI） */
enum class DeviceState
{
    Run    = 0,   ///< 运行中
    Stop   = 1,   ///< 停止
    Idle   = 2,   ///< 空闲
    Alarm  = 3,   ///< 报警
};

/** 硬件连接状态（首页侧栏） */
enum class HardwareLinkState
{
    Connected    = 0,
    Standby      = 1,
    Disconnected = 2,
};

/** 配方列表右侧标签：默认配方显示「默认」，其余为空白 */
enum class RecipeListTag
{
    Default = 0, ///< 当前默认配方
    None    = 1, ///< 非默认（界面不显示文字）
};

/** 预览/调试颜色通道（首页、配方、虚拟像素） */
enum class ColorChannel
{
    Red   = 0,
    Green = 1,
    Blue  = 2,
    Black = 3,
    White = 4,
};

/** 相机索引（主/前/左/右/后） */
enum class CameraIndex
{
    Main  = 0,
    Front = 1,
    Left  = 2,
    Right = 3,
    Back  = 4,
};

/** 检测项（9 项 + 串死/串活，含 Flash） */
enum class DetectItemKind
{
    DeadLed        = 0,
    DimLed         = 1,
    BrightLed      = 2,
    ColorVariation = 3,
    SeriesLed      = 4,
    MixedColor     = 5,
    BlackScreen    = 6,
    BlackScreenOn  = 7,
    Flash          = 8,
    ColorMapDead   = 9,  ///< 串死（配方调试区串灯检测 map_dead，非首页检测项）
    ColorMapLive   = 10, ///< 串活（配方调试区串灯检测 map_live，非首页检测项）
};

constexpr int kDetectItemCount = 9;  ///< 首页/系统配置检测项总数（不含串死/串活）
constexpr int kCameraCount     = 5;  ///< 相机数量
constexpr int kKpiCount        = 9;  ///< 首页 KPI 项数（旧版，现首页为 10 项）
constexpr int kDefectRgbCount = kLightenInspectionDefectRgbCount;

/** 首页检测项进度：死灯～混色按 R/G/B 拆分为 18 项 + 黑屏检测/黑屏亮灯/Flash 共 21 项 */
constexpr int kRgbSplitDetectItemCount = 6;  ///< 按 RGB 拆分的检测项数（死灯～混色）
constexpr int kHomeProgressRgbSlots    = 18; ///< 首页 RGB 进度槽位数
constexpr int kHomeProgressCount         = 21; ///< 首页检测项进度总槽位数

/** 将 InspectionResult 转为界面显示文本 */
QString inspectionResultText(InspectionResult r);
/** 将 Flash 结果整型码转为界面显示文本（0=OK, 1=NG） */
QString flashResultText(int flashResultCode);
/** 将 RunMode 转为界面显示文本 */
QString runModeText(RunMode m);
/** 将 DeviceState 转为界面显示文本 */
QString deviceStateText(DeviceState s);
/** 将 HardwareLinkState 转为界面显示文本 */
QString hardwareLinkStateText(HardwareLinkState s);
/** 将 RecipeListTag 转为界面显示文本 */
QString recipeListTagText(RecipeListTag tag);
/** 将 DetectItemKind 转为检测项名称 */
QString detectItemName(DetectItemKind kind);
/** 检测项是否在首页进度区按 R/G/B 分色展示 */
bool detectItemHasRgbProgress(DetectItemKind kind);
/** 首页进度槽位索引（RGB 项须指定 colorIdx=0..2，其余传 -1） */
int homeProgressSlotIndex(DetectItemKind kind, int colorIdx = -1);
/** 由首页进度槽位反查检测项与颜色（无分色时 colorIdx 输出 -1） */
void decodeHomeProgressSlot(int slot, DetectItemKind *outKind, int *outColorIdx);
/** 首页进度行显示名（如「红色死灯」） */
QString homeProgressItemName(DetectItemKind kind, ColorChannel ch);
/** 检测项是否有「总计」数量（false 时 UI 显示 "-"，仅 OK/NG 判定） */
bool detectItemHasTotalCount(DetectItemKind kind);
/** 将 ColorChannel 转为颜色通道名称 */
QString colorChannelName(ColorChannel ch);
/** 将 CameraIndex 转为相机名称 */
QString cameraName(CameraIndex cam);

} // namespace lighten

Q_DECLARE_METATYPE(lighten::InspectionResult)
Q_DECLARE_METATYPE(lighten::RunMode)
Q_DECLARE_METATYPE(lighten::DeviceState)
