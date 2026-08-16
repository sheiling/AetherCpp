#include "lighten_device_ops.h"

#include <cstring>
#include <thread>

/**
 * @file lighten_device_ops.cpp
 * @brief 点亮检测 service 层：SoftDisplay / GrabRgbBuffer 实现（位于 service/）。
 *
 * SoftDisplay：查找软屏设备、映射 SoftDisplayOptions → SoftScreenParam 并打屏。
 * GrabRgbBuffer：查找相机、可选设置曝光、同步采图并转为 RGB ImageBuffer。
 */

namespace step {

SoftDisplayOptions MakeBaseSoftDisplay(
    const std::string& soft_screen_key,
    int panel_w,
    int panel_h,
    int disp_delay_ms)
{
    SoftDisplayOptions opt{};
    opt.soft_screen_key = soft_screen_key;
    opt.mode            = common::device::FULL_MODE;
    opt.panel_w         = panel_w;
    opt.panel_h         = panel_h;
    opt.interval        = 1;
    opt.settle_delay    = std::chrono::milliseconds(disp_delay_ms);
    return opt;
}

// 查找软屏设备，映射显示参数并调用 Display；可选 settle_delay 等待画面稳定。
common::errors::ErrorPtr SoftDisplay(
    const std::shared_ptr<common::device::DeviceManager>& dm,
    const SoftDisplayOptions& opt)
{
    if (!dm) {
        return common::errors::New(-1, "DeviceManager is null");
    }

    auto screen = dm->SoftScreenHandler(opt.soft_screen_key);
    if (!screen) {
        return common::errors::New(-2, "SoftScreen handler missing: " + opt.soft_screen_key);
    }

    // 软屏参数映射集中在此，避免各步骤重复转换。
    common::device::SoftScreenParam p{};
    p.color       = opt.color;
    p.mode        = opt.mode;
    p.brightness  = opt.brightness;
    p.interval    = opt.interval;
    if (opt.override_w > 0 && opt.override_h > 0) {
        p.width  = opt.override_w;
        p.height = opt.override_h;
    } else {
        p.width  = opt.panel_w;
        p.height = opt.panel_h;
    }

    screen->Display(p);
    if (opt.settle_delay.count() > 0) {
        std::this_thread::sleep_for(opt.settle_delay);
    }
    return nullptr;
}

// 同步采图并转换为 RGB ImageBuffer；设备查找与连通性检查在此完成，步骤层仅负责编排。
common::errors::ErrorPtr GrabRgbBuffer(
    const std::shared_ptr<common::device::DeviceManager>& dm,
    const std::string& camera_name,
    int exposure_us,
    std::shared_ptr<common::algorithm::ImageBuffer>* out_image)
{
    if (!dm) {
        return common::errors::New(-1, "DeviceManager is null");
    }

    auto cam = dm->CameraHandler(camera_name);
    if (!cam) {
        return common::errors::New(-3, "Camera not found: " + camera_name);
    }
    if (!cam->IsConnected()) {
        return common::errors::New(-4, "Camera disconnected: " + camera_name);
    }

    if (exposure_us > 0) {
        if (auto ee = cam->SetExposureTime(exposure_us)) {
            return ee;
        }
    }

    int h = 0;
    int w = 0;
    unsigned char* sdk_buf = nullptr;
    int len = 0;
    if (auto ge = cam->SyncGrab(h, w, sdk_buf, len)) {
        return ge;
    }
    if (!sdk_buf || len <= 0 || h <= 0 || w <= 0) {
        return common::errors::New(-5, "SyncGrab returned empty buffer");
    }

    // 检测算法消费 RGB 布局缓冲；将 SDK 原始字节归一化为 ImageBuffer。
    const int channels = 1;
    auto img = common::algorithm::ImageBuffer::Create(w, h, channels, 2);
    const size_t cap = img->ByteSize();
    const size_t n   = static_cast<size_t>(len);
    const size_t copy_n = (cap < n) ? cap : n;
    std::memcpy(img->data.get(), sdk_buf, copy_n);

    *out_image = std::move(img);
    return nullptr;
}

} // namespace step
