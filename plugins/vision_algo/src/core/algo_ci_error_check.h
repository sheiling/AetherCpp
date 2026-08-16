#pragma once

/**
 * @file algo_ci_error_check.h
 * @brief CISDK / 业务错误 → common::errors::ErrorPtr（插件内部）
 */

#include "algo_ci_error_parser.h"
#include "vision_algo_error_codes.h"

#include <common/include/macros.h>
#include <common/utility/errors/include/error.h>

#include <ci_basic_error.h>

namespace vision_algo {

/** @return nullptr 表示 ALGO_PASS；否则封装为 algo_module::kSdkError（文案已翻译） */
common::errors::ErrorPtr checkAlgoError(const char *apiName, AlgoError err);

/** @param i18nKey VisionAlgo 翻译键（经 vision_algo::tr 翻译后写入 Message） */
common::errors::ErrorPtr algoErrorFromKey(int code, const char *i18nKey);

common::errors::ErrorPtr algoInvalidArgument(const char *i18nKey);
common::errors::ErrorPtr algoInvalidImage(const char *i18nKey);
common::errors::ErrorPtr algoUnitNotAvailable(const char *unitName);
common::errors::ErrorPtr algoServiceNotReady();

} // namespace vision_algo
