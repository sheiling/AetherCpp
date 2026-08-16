#pragma once

/**
 * @file vision_algo_error_codes.h
 * @brief VisionAlgo 插件业务错误码（common::errors::ErrorPtr）
 */

#include <common/utility/errors/include/error_codes.h>

namespace common::errors {

namespace algo_module {

inline constexpr int kModuleAlgo = 5; // 005

inline constexpr int kUnknown        = 5900; // 005900
inline constexpr int kSdkError         = 5200; // 005200 CISDK AlgoError 失败
inline constexpr int kInvalidHandle    = 5201; // 005201 句柄为空或无效
inline constexpr int kModelLoadFailed  = 5202; // 005202 模板/模型加载失败
inline constexpr int kInvalidImage     = 5203; // 005203 送检图像无效
inline constexpr int kTempImageFailed  = 5204; // 005204 临时图像写入失败
inline constexpr int kInvalidArgument  = 5205; // 005205 入参无效
inline constexpr int kUnitNotAvailable = 5206; // 005206 算法单元未注册/不可用
inline constexpr int kServiceNotReady  = 5207; // 005207 服务未就绪

inline bool RegisterVisionAlgoCoders()
{
    bool ok = true;
    ok      = ok && RegisterCoder(kUnknown, "vision algo unknown error",
                                    "https://docs.example/errors/vision-algo/unknown");
    ok      = ok && RegisterCoder(kSdkError, "vision algo cisdk sdk error",
                                    "https://docs.example/errors/vision-algo/sdk-error");
    ok      = ok && RegisterCoder(kInvalidHandle, "vision algo invalid handle",
                                    "https://docs.example/errors/vision-algo/invalid-handle");
    ok      = ok && RegisterCoder(kModelLoadFailed, "vision algo model load failed",
                                    "https://docs.example/errors/vision-algo/model-load-failed");
    ok      = ok && RegisterCoder(kInvalidImage, "vision algo invalid image",
                                    "https://docs.example/errors/vision-algo/invalid-image");
    ok      = ok && RegisterCoder(kTempImageFailed, "vision algo temp image write failed",
                                    "https://docs.example/errors/vision-algo/temp-image-failed");
    ok      = ok && RegisterCoder(kInvalidArgument, "vision algo invalid argument",
                                    "https://docs.example/errors/vision-algo/invalid-argument");
    ok      = ok && RegisterCoder(kUnitNotAvailable, "vision algo unit not available",
                                    "https://docs.example/errors/vision-algo/unit-not-available");
    ok      = ok && RegisterCoder(kServiceNotReady, "vision algo service not ready",
                                    "https://docs.example/errors/vision-algo/service-not-ready");
    return ok;
}

} // namespace algo_module

} // namespace common::errors
