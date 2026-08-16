#ifndef COMMON_CORE_DEVICE_CAMERA_CAMERA_ERROR_CODES_H
#define COMMON_CORE_DEVICE_CAMERA_CAMERA_ERROR_CODES_H

#include <string>

#include <common/utility/errors/include/error_codes.h>

namespace common::errors {

// 相机模块错误码（对外暴露，供业务侧获取/定义）
// 6 位错误码规则：
// - 前 3 位：模块（相机模块建议固定为 002）
// - 后 3 位：业务错误码（按百位映射错误级别：0xx=info, 1xx=warning, 2xx=error, 3xx=critical）
//
// 注意：在 C++ 的 `int` 表达中前导 0 会被省略，因此 `002200` 在代码中为 `2200`。
namespace camera_module {
inline constexpr int kModuleCamera = 2; // 002

// 9xx：未知/未分类（SeverityOfCode 会映射到 unknown）
inline constexpr int kUnknown = 2900; // 002900

// 2xx：error
inline constexpr int kDeviceNotFound = 2200; // 002200
inline constexpr int kOpenFailed = 2201; // 002201
inline constexpr int kNotOpened = 2202; // 002202
inline constexpr int kStartGrabbingFailed = 2203; // 002203
inline constexpr int kStopGrabbingFailed = 2204; // 002204
inline constexpr int kFrameTimeout = 2205; // 002205
inline constexpr int kSdkError = 2206; // 002206
inline constexpr int kAlreadyOpened = 2207; // 002207
inline constexpr int kAlreadyStarted = 2208; // 002208
inline constexpr int kInvalidConfig = 2209; // 002209

// 3xx：critical
inline constexpr int kHardwareFault = 2300; // 002300

// 注册相机模块的错误码描述（建议在某个 .cpp 中调用一次；相机库也可在加载时静态调用）
inline bool RegisterCameraCoders() {
    bool ok = true;
    ok = ok && ::common::errors::RegisterCoder(kUnknown, "camera unknown error", "https://docs.example/errors/camera/unknown");

    ok = ok && ::common::errors::RegisterCoder(kAlreadyOpened, "camera already opened", "https://docs.example/errors/camera/already-opened");
    ok = ok && ::common::errors::RegisterCoder(kAlreadyStarted, "camera already started", "https://docs.example/errors/camera/already-started");
    ok = ok && ::common::errors::RegisterCoder(kInvalidConfig, "camera invalid config", "https://docs.example/errors/camera/invalid-config");

    ok = ok && ::common::errors::RegisterCoder(kDeviceNotFound, "camera device not found", "https://docs.example/errors/camera/device-not-found");
    ok = ok && ::common::errors::RegisterCoder(kOpenFailed, "camera open failed", "https://docs.example/errors/camera/open-failed");
    ok = ok && ::common::errors::RegisterCoder(kNotOpened, "camera not opened", "https://docs.example/errors/camera/not-opened");
    ok = ok && ::common::errors::RegisterCoder(kStartGrabbingFailed, "camera start grabbing failed", "https://docs.example/errors/camera/start-grabbing-failed");
    ok = ok && ::common::errors::RegisterCoder(kStopGrabbingFailed, "camera stop grabbing failed", "https://docs.example/errors/camera/stop-grabbing-failed");
    ok = ok && ::common::errors::RegisterCoder(kFrameTimeout, "camera frame timeout", "https://docs.example/errors/camera/frame-timeout");
    ok = ok && ::common::errors::RegisterCoder(kSdkError, "camera sdk error", "https://docs.example/errors/camera/sdk-error");

    ok = ok && ::common::errors::RegisterCoder(kHardwareFault, "camera hardware fault", "https://docs.example/errors/camera/hardware-fault");
    return ok;
}
} // namespace camera_module

} // namespace common::errors

#endif // COMMON_CORE_DEVICE_CAMERA_CAMERA_ERROR_CODES_H
