#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_ERROR_CODES_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_ERROR_CODES_H

#include <common/utility/errors/include/error_codes.h>

namespace common::errors {

// 运动控制模块错误码（对外暴露，供业务侧获取/定义）
// 6 位错误码规则（业务侧 ErrorPtr，与 motion_sdk_err_map.h 厂商 SDK 字符串表无关）
// - 前 3 位：模块（运动控制模块建议固定为 003）
// - 后 3 位：业务错误码（按百位映射错误级别：0xx=info, 1xx=warning, 2xx=error, 3xx=critical）
//
// 注意：在 C++ 的 `int` 表达中前导 0 会被省略，因此 `003200` 在代码中为 `3200`。
namespace motion_module {
inline constexpr int kModuleMotion = 3; // 003

// 9xx：未知/未分类（SeverityOfCode 会映射到 unknown）
inline constexpr int kUnknown = 3900; // 003900

// 2xx：error
inline constexpr int kAxisCardNotMapped = 3200; // 003200
inline constexpr int kIoCardNotMapped = 3201; // 003201
inline constexpr int kAdCardNotMapped = 3202; // 003202
inline constexpr int kInvalidAxisCount = 3203; // 003203
inline constexpr int kInvalidCardCount = 3204; // 003204
inline constexpr int kAxisNameEmpty = 3205; // 003205
inline constexpr int kCardAlreadyExists = 3206; // 003206
inline constexpr int kResourceAlreadyExists = 3207; // 003207
inline constexpr int kUnsupportedCardType = 3208; // 003208
inline constexpr int kSdkError = 3209; // 003209
inline constexpr int kAxisNotFound = 3210; // 003210
inline constexpr int kMotionTimeout = 3211; // 003211
inline constexpr int kAxisAlarm = 3212; // 003212
inline constexpr int kAxisResetTimeout = 3213; // 003213
inline constexpr int kAxisDisabled = 3214; // 003214
inline constexpr int kNotSupported = 3215; // 003215
inline constexpr int kInvalidComparePoints = 3216; // 003216

// 3xx：critical
inline constexpr int kHardwareFault = 3300; // 003300

inline bool RegisterMotionCoders() {
    bool ok = true;
    ok = ok && ::common::errors::RegisterCoder(kUnknown, "motion unknown error", "https://docs.example/errors/motion/unknown");
    ok = ok && ::common::errors::RegisterCoder(kAxisCardNotMapped, "motion axis not mapped to card", "https://docs.example/errors/motion/axis-card-not-mapped");
    ok = ok && ::common::errors::RegisterCoder(kIoCardNotMapped, "motion io not mapped to card", "https://docs.example/errors/motion/io-card-not-mapped");
    ok = ok && ::common::errors::RegisterCoder(kAdCardNotMapped, "motion ad not mapped to card", "https://docs.example/errors/motion/ad-card-not-mapped");
    ok = ok && ::common::errors::RegisterCoder(kInvalidAxisCount, "motion invalid axis count", "https://docs.example/errors/motion/invalid-axis-count");
    ok = ok && ::common::errors::RegisterCoder(kInvalidCardCount, "motion invalid card count", "https://docs.example/errors/motion/invalid-card-count");
    ok = ok && ::common::errors::RegisterCoder(kAxisNameEmpty, "motion axis name empty", "https://docs.example/errors/motion/axis-name-empty");
    ok = ok && ::common::errors::RegisterCoder(kCardAlreadyExists, "motion card already exists", "https://docs.example/errors/motion/card-already-exists");
    ok = ok && ::common::errors::RegisterCoder(kResourceAlreadyExists, "motion resource already exists", "https://docs.example/errors/motion/resource-already-exists");
    ok = ok && ::common::errors::RegisterCoder(kUnsupportedCardType, "motion unsupported card type", "https://docs.example/errors/motion/unsupported-card-type");
    ok = ok && ::common::errors::RegisterCoder(kSdkError, "motion sdk error", "https://docs.example/errors/motion/sdk-error");
    ok = ok && ::common::errors::RegisterCoder(kAxisNotFound, "motion axis not found", "https://docs.example/errors/motion/axis-not-found");
    ok = ok && ::common::errors::RegisterCoder(kMotionTimeout, "motion timeout", "https://docs.example/errors/motion/timeout");
    ok = ok && ::common::errors::RegisterCoder(kAxisAlarm, "motion axis alarm", "https://docs.example/errors/motion/axis-alarm");
    ok = ok && ::common::errors::RegisterCoder(kAxisResetTimeout, "motion axis reset timeout", "https://docs.example/errors/motion/axis-reset-timeout");
    ok = ok && ::common::errors::RegisterCoder(kAxisDisabled, "motion axis disabled", "https://docs.example/errors/motion/axis-disabled");
    ok = ok && ::common::errors::RegisterCoder(kNotSupported, "motion capability not supported", "https://docs.example/errors/motion/not-supported");
    ok = ok && ::common::errors::RegisterCoder(kInvalidComparePoints, "motion invalid position compare points", "https://docs.example/errors/motion/invalid-compare-points");
    ok = ok && ::common::errors::RegisterCoder(kHardwareFault, "motion hardware fault", "https://docs.example/errors/motion/hardware-fault");
    return ok;
}

} // namespace motion_module

} // namespace common::errors

#endif // COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_ERROR_CODES_H
