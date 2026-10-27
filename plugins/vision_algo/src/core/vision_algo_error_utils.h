#pragma once

/**
 * @file vision_algo_error_utils.h
 * @brief VisionAlgo 业务错误 → common::errors::ErrorPtr（插件内部）
 */

#include "vision_algo_error_codes.h"

#include <common/utility/errors/include/error.h>

namespace vision_algo {

/** @param i18nKey VisionAlgo 翻译键（经 vision_algo::tr 翻译后写入 Message） */
common::errors::ErrorPtr algoErrorFromKey(int code, const char *i18nKey);

common::errors::ErrorPtr algoInvalidArgument(const char *i18nKey);
common::errors::ErrorPtr algoInvalidImage(const char *i18nKey);
common::errors::ErrorPtr algoUnitNotAvailable(const char *unitName);
common::errors::ErrorPtr algoServiceNotReady();

} // namespace vision_algo
