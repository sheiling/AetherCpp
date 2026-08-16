#pragma once

/**
 * @file algo_sdk_guard.h
 * @brief CISDK 算法资源 RAII 封装与错误转换工具。
 *
 * 提供 MeterInspectGuard、AlgoMatGuard 及 FailAlgo，供各检测算法与 ShotMeter 复用。
 */

#include "algo_error_message.h"

#include <common/3rdparty/cisdk/detection/include/led_meter_inspector.h>
#include <common/3rdparty/cisdk/measurement/include/algo_mat.h>
#include <common/3rdparty/cisdk/measurement/include/ci_basic_error.h>
#include <common/utility/errors/include/error.h>

namespace common {
namespace algorithm {

// RAII 封装 MeterInspect 句柄，析构时自动销毁。
class MeterInspectGuard {
public:
    ~MeterInspectGuard() { reset(); }

    AlgoHandle* out() { return &handle_; }
    AlgoHandle get() const { return handle_; }

    void reset()
    {
        if (handle_ != nullptr) {
            MeterInspectDestroyHandle(handle_);
            handle_ = nullptr;
        }
    }

private:
    AlgoHandle handle_ = nullptr;
};

// RAII 封装 AlgoMat，析构或 reset 时自动释放。
class AlgoMatGuard {
public:
    ~AlgoMatGuard() { reset(nullptr); }

    AlgoMat* get() const { return mat_; }

    void reset(AlgoMat* mat)
    {
        if (mat_ != nullptr) {
            AlgoMatDelete(mat_);
        }
        mat_ = mat;
    }

private:
    AlgoMat* mat_ = nullptr;
};

// 将 CISDK AlgoError 转换为项目统一错误对象；ALGO_PASS 时返回 nullptr。
inline common::errors::ErrorPtr FailAlgo(const char* ctx, AlgoError err)
{
    if (err == ALGO_PASS) {
        return nullptr;
    }
    const uint32_t moduleId = ERROR_MODULE_NAME(err);
    const uint32_t errorCode = ERROR_CODE_NO_MODULE(err);
    const char* moduleDesc = GetAlgoModuleDescription(moduleId);
    const char* errorDesc = GetAlgoErrorDescription(moduleId, errorCode);
    return common::errors::Newf(
        errorCode,
        "%s (%s：%s)",
        ctx,
        moduleDesc,
        errorDesc);
}

} // namespace algorithm
} // namespace common
