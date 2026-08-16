#ifndef COMMON_BASE_IMGPROC_IMGPROC_ERROR_CODES_H
#define COMMON_BASE_IMGPROC_IMGPROC_ERROR_CODES_H

#include <common/utility/errors/include/error_codes.h>

namespace common::errors {

// 图像处理模块错误码（对外暴露，供业务侧获取/定义）
// 6 位错误码规则：
// - 前 3 位：模块（图像处理模块建议固定为 004）
// - 后 3 位：业务错误码（按百位映射错误级别：0xx=info, 1xx=warning, 2xx=error, 3xx=critical）
//
// 注意：在 C++ 的 `int` 表达中前导 0 会被省略，因此 `004200` 在代码中为 `4200`。
namespace imgproc_module {
inline constexpr int kModuleImgproc = 4; // 004

// 9xx：未知/未分类（SeverityOfCode 会映射到 unknown）
inline constexpr int kUnknown = 4900; // 004900

// 2xx：error
inline constexpr int kInvalidArgument = 4200; // 004200
inline constexpr int kInvalidImageBuffer = 4201; // 004201
inline constexpr int kUnsupportedBayerFormat = 4202; // 004202
inline constexpr int kUnsupportedImageFormat = 4203; // 004203
inline constexpr int kEncodeFailed = 4204; // 004204
inline constexpr int kJpegOnlySupports8Bit = 4205; // 004205
inline constexpr int kConvertSourceTypeMismatch = 4206; // 004206
inline constexpr int kRotateCodeUnsupported = 4207; // 004207
inline constexpr int kBuildOutputFailed = 4208; // 004208

inline bool RegisterImgprocCoders() {
    bool ok = true;
    ok = ok && ::common::errors::RegisterCoder(kUnknown, "imgproc unknown error", "https://docs.example/errors/imgproc/unknown");
    ok = ok && ::common::errors::RegisterCoder(kInvalidArgument, "imgproc invalid argument", "https://docs.example/errors/imgproc/invalid-argument");
    ok = ok && ::common::errors::RegisterCoder(kInvalidImageBuffer, "imgproc invalid image buffer", "https://docs.example/errors/imgproc/invalid-image-buffer");
    ok = ok && ::common::errors::RegisterCoder(kUnsupportedBayerFormat, "imgproc unsupported bayer format", "https://docs.example/errors/imgproc/unsupported-bayer-format");
    ok = ok && ::common::errors::RegisterCoder(kUnsupportedImageFormat, "imgproc unsupported image format", "https://docs.example/errors/imgproc/unsupported-image-format");
    ok = ok && ::common::errors::RegisterCoder(kEncodeFailed, "imgproc encode failed", "https://docs.example/errors/imgproc/encode-failed");
    ok = ok && ::common::errors::RegisterCoder(kJpegOnlySupports8Bit, "imgproc jpeg only supports 8-bit image", "https://docs.example/errors/imgproc/jpeg-only-supports-8bit");
    ok = ok && ::common::errors::RegisterCoder(kConvertSourceTypeMismatch, "imgproc source type mismatch", "https://docs.example/errors/imgproc/source-type-mismatch");
    ok = ok && ::common::errors::RegisterCoder(kRotateCodeUnsupported, "imgproc rotate code unsupported", "https://docs.example/errors/imgproc/rotate-code-unsupported");
    ok = ok && ::common::errors::RegisterCoder(kBuildOutputFailed, "imgproc build output failed", "https://docs.example/errors/imgproc/build-output-failed");
    return ok;
}
} // namespace imgproc_module

} // namespace common::errors

#endif // COMMON_BASE_IMGPROC_IMGPROC_ERROR_CODES_H
