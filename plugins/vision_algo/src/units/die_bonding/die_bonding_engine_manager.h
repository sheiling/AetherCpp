#pragma once

/**
 * @file die_bonding_engine_manager.h
 * @brief 固晶检测引擎进程内单例（插件内部，不对外暴露）
 */

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include "die_bonding_detect_engine.h"

#include <string>

namespace vision_algo::die_bonding_internal {

bool acquireEngine(const DieBondingChipDetectParam &param, std::size_t poolSize,
                   int dlInferenceEngineOverride, std::string *errorOut, std::size_t *outPoolSize);

bool syncEngineParam(const DieBondingChipDetectParam &param, std::string *errorOut);

DieBondingDetectEngine *activeEngine();

bool engineReady();

std::size_t enginePoolSize();

} // namespace vision_algo::die_bonding_internal
