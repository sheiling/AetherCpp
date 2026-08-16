#pragma once

/**
 * @file die_bonding_detect_engine.h
 * @brief 固晶检测引擎实现（构造时固定 poolSize，运行期仅 syncParam）
 */

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include "die_handle_pool.h"

namespace vision_algo {

class DieBondingDetectEngine
{
public:
    DieBondingDetectEngine(DieBondingChipDetectParam param, std::size_t poolSize, bool isDeepLearning);
    ~DieBondingDetectEngine() = default;

    bool isReady() const { return pool_ != nullptr; }

    std::size_t poolSize() const { return pool_ ? pool_->initialPoolSize() : 0; }

    bool syncParam(const DieBondingChipDetectParam &param);

    common::errors::ErrorPtr runDetect(const DieBondingChipDetectInput::DetectFields &in,
                                       DieBondingChipDetectOutput::DetectResult &out);

private:
    DieBondingChipDetectParam   param_;
    std::unique_ptr<HandlePool> pool_;
};

using DieBondingDetectEnginePtr = std::shared_ptr<DieBondingDetectEngine>;

} // namespace vision_algo
