#include "die_bonding_detect_engine.h"

#include "algo_ci_error_check.h"
#include "die_bonding_chip_detect.h"

#include <Logger.h>

namespace vision_algo {

DieBondingDetectEngine::DieBondingDetectEngine(DieBondingChipDetectParam param, std::size_t poolSize,
                                               bool isDeepLearning)
    : param_(std::move(param))
{
    pool_ = std::make_unique<HandlePool>(poolSize, isDeepLearning, param_);
}

bool DieBondingDetectEngine::syncParam(const DieBondingChipDetectParam &param)
{
    if (!pool_)
        return false;
    if (const auto err = pool_->SetParam(param); err) {
        LOG_ERROR("[固晶引擎] syncParam 失败：{}", err->Message());
        return false;
    }
    param_ = param;
    return true;
}

common::errors::ErrorPtr DieBondingDetectEngine::runDetect(
    const DieBondingChipDetectInput::DetectFields &in,
    DieBondingChipDetectOutput::DetectResult &out)
{
    if (!pool_)
        return algoInvalidArgument("vision_algo.error.engine_pool_null");

    void *handle = pool_->GetHandle();
    if (!handle)
        return algoInvalidArgument("vision_algo.error.engine_handle_unavailable");

    return DieBondingChipDetect::runDetect(in, out, handle);
}

} // namespace vision_algo
