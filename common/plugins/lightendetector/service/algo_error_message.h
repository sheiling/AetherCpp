#pragma once

/**
 * @file algo_error_message.h
 * @brief CISDK AlgoError 模块名与错误码中文描述查询。
 */

#include <common/3rdparty/cisdk/measurement/include/ci_basic_error.h>

#include <cstdint>

namespace common {
namespace algorithm {

// 返回模块中文名称；未知模块时返回 "模块{id}"。
const char* GetAlgoModuleDescription(uint32_t moduleId);

// 返回错误码中文描述；未知错误时返回 "错误码{code}"。
const char* GetAlgoErrorDescription(uint32_t moduleId, uint32_t errorCode);

} // namespace algorithm
} // namespace common
