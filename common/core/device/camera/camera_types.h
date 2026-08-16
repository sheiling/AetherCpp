#ifndef COMMON_CORE_DEVICE_CAMERA_CAMERA_TYPES_H
#define COMMON_CORE_DEVICE_CAMERA_CAMERA_TYPES_H

#include <common/utility/json_tool/reflector.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace common {
namespace device {

enum CameraFeatureType {
    CAMERA_EXPOSURE_TIME,           // 曝光时间
    CAMERA_FILTER_WHEEL,            // 滤镜轮位置
    CAMERA_ND_FILTER_WHEEL,         // 减光镜（ND）滤镜轮
    CAMERA_TRIGGER_ENABLE,          // 触发模式开关
    CAMERA_TRIGGER_SOURCE,          // 触发源（软触发 / 硬触发线）
    CAMERA_BALANCE_RATIO_SELECTOR,  // 白平衡通道选择（R/G/B）
    CAMERA_LIGHT_SOURCE_PRESET,     // 光源预设
    CAMERA_BALANCE_WHITE_AUTO,      // 自动白平衡
    CAMERA_BALANCE_RATIO,           // 白平衡比例
    CAMERA_OFFSETX,                 // ROI 水平偏移
    CAMERA_OFFSETY,                 // ROI 垂直偏移
    CAMERA_HEIGHT,                  // ROI 高度
    CAMERA_WIDTH                    // ROI 宽度
};

enum class CameraTriggerSourceType {
    SOFTWARE_TRIGGER,
    HARD_TRIGGER_LINE0,
    HARD_TRIGGER_LINE1,
    HARD_TRIGGER_LINE2,
    HARD_TRIGGER_LINE3
};
NLOHMANN_JSON_SERIALIZE_ENUM(CameraTriggerSourceType, {
{ CameraTriggerSourceType::SOFTWARE_TRIGGER,   "SOFTWARE_TRIGGER" },
{ CameraTriggerSourceType::HARD_TRIGGER_LINE0, "HARD_TRIGGER_LINE0" },
{ CameraTriggerSourceType::HARD_TRIGGER_LINE1, "HARD_TRIGGER_LINE1" },
{ CameraTriggerSourceType::HARD_TRIGGER_LINE2, "HARD_TRIGGER_LINE2" },
{ CameraTriggerSourceType::HARD_TRIGGER_LINE3, "HARD_TRIGGER_LINE3" }
    });

/** @brief 传感器像素排列类型（由 PixelFormat 字符串推断） */
enum class PixelFormatType {
    Unknown = 0,
    Mono,   // 黑白/单色
    Bayer,  // Bayer 彩色
    RGB,    // 已解马赛克 RGB/BGR 彩色
    Filter  // XYZ 滤片轮相机：多通道灰度 RAW，非 RGB，一般不去马赛克
};
NLOHMANN_JSON_SERIALIZE_ENUM(PixelFormatType, {
{ PixelFormatType::Unknown, "Unknown" },
{ PixelFormatType::Mono,     "Mono" },
{ PixelFormatType::Bayer,    "Bayer" },
{ PixelFormatType::RGB,      "RGB" },
{ PixelFormatType::Filter,   "Filter" }
    });

inline std::string toLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

/** 将 Galaxy PixelFormat 字符串（如 Mono8、BayerRG8）映射为 PixelFormatType */
inline PixelFormatType pixelFormatTypeFromSdkString(const std::string &pixelFormat)
{
    if (pixelFormat.empty()) {
        return PixelFormatType::Unknown;
    }
    const std::string lower = toLowerCopy(pixelFormat);
    if (lower.find("mono") != std::string::npos) {
        return PixelFormatType::Mono;
    }
    if (lower.find("bayer") != std::string::npos) {
        return PixelFormatType::Bayer;
    }
    if (lower.find("rgb") != std::string::npos || lower.find("bgr") != std::string::npos) {
        return PixelFormatType::RGB;
    }
    if (lower.find("filter") != std::string::npos) {
        return PixelFormatType::Filter;
    }
    return PixelFormatType::Unknown;
}

inline const char *pixelFormatTypeToString(PixelFormatType type)
{
    switch (type) {
    case PixelFormatType::Mono:
        return "Mono";
    case PixelFormatType::Bayer:
        return "Bayer";
    case PixelFormatType::RGB:
        return "RGB";
    case PixelFormatType::Filter:
        return "Filter";
    case PixelFormatType::Unknown:
    default:
        return "Unknown";
    }
}

enum class CameraFilterWheelType {
    FILTER_UNDEFINED,
    FILTER_X,
    FILTER_Y,
    FILTER_Z,
    FILTER_XYZ,
};

NLOHMANN_JSON_SERIALIZE_ENUM(CameraFilterWheelType, {
{ CameraFilterWheelType::FILTER_UNDEFINED,   "FILTER_UNDEFINED" },
{ CameraFilterWheelType::FILTER_X, "FILTER_X" },
{ CameraFilterWheelType::FILTER_Y, "FILTER_Y" },
{ CameraFilterWheelType::FILTER_Z, "FILTER_Z" },
{ CameraFilterWheelType::FILTER_XYZ, "FILTER_XYZ" }
    });

} // namespace device
} // namespace common

#endif
