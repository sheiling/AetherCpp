#pragma once

/**
 * @file lighten_device_ops.h
 * @brief 点亮检测 service 层：软屏显示与相机采图公共接口。
 *
 * 将 DeviceManager 查找、参数映射、曝光设置、RGB 缓冲转换等重复逻辑
 * 从 step/ 调度层抽离，供 LightenDetectStep 等工步统一调用。
 */

#include <chrono>
#include <memory>
#include <string>

#include <common/base/algorithm/algorithm_packet.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/device/displayControl/software_screen/software_screen_base.h>
#include <common/utility/errors/include/error.h>

namespace step {

// 软屏显示参数，供 lighten 相关工步统一调用 SoftDisplay。
struct SoftDisplayOptions {
    // DeviceManager 中软屏设备的配置键。
    std::string              soft_screen_key;
    // 显示颜色。
    common::device::SoftScreenColor color   = common::device::COLOR_WHITE;
    // 显示模式（全屏 / 奇偶行等）。
    common::device::SoftScreenMode  mode    = common::device::FULL_MODE;
    // 亮度，范围 0-255。
    int                      brightness       = 255;
    // 面板物理宽高（override 未设置时使用）。
    int                      panel_w          = 0;
    int                      panel_h          = 0;
    // 覆盖显示分辨率；大于 0 时优先于 panel_w / panel_h。
    int                      override_w       = 0;
    int                      override_h       = 0;
    // 显示刷新间隔。
    int                      interval         = 1;
    // 显示完成后的稳定等待时间，用于等待屏幕/背光稳定后再采图。
    std::chrono::milliseconds settle_delay{ 0 };
};

// 构造打屏公共参数（全屏、面板尺寸、稳定等待）；各色/各模式循环中仅覆盖 color/mode/brightness 等差异项。
SoftDisplayOptions MakeBaseSoftDisplay(
    const std::string& soft_screen_key,
    int panel_w,
    int panel_h,
    int disp_delay_ms);

// 统一软屏显示入口：查找软屏设备、映射参数并调用 Display。
// 成功返回 nullptr，失败返回 ErrorPtr。
common::errors::ErrorPtr SoftDisplay(
    const std::shared_ptr<common::device::DeviceManager>& dm,
    const SoftDisplayOptions& opt);

// 同步采图并转换为 RGB ImageBuffer，供算法步骤消费。
// exposure_us 传给 CameraBase::SetExposureTime（与现场配置一致，通常为微秒）；
// 小于等于 0 时跳过曝光设置。成功返回 nullptr，失败返回 ErrorPtr。
common::errors::ErrorPtr GrabRgbBuffer(
    const std::shared_ptr<common::device::DeviceManager>& dm,
    const std::string& camera_name,
    int exposure_us,
    std::shared_ptr<common::algorithm::ImageBuffer>* out_image);

} // namespace step
