#pragma once

/**
 * @file algo_ci_error_parser.h
 * @brief CISDK AlgoError 解析（仅 VisionAlgo 插件内部）
 *
 * 对应 CISDK ci_basic_error.h：高 16 位模块号、低 16 位错误码。
 */

#include <ci_define.h>

#include <cstdint>
#include <string>

namespace vision_algo {

/** CISDK AlgoError 解析结果 */
struct AlgoErrorInfo
{
    uint32_t    moduleId;     ///< 模块号（ERROR_MODULE_NAME）
    uint32_t    errorCode;    ///< 错误码（ERROR_CODE_NO_MODULE）
    std::string moduleName;   ///< 模块名字符串
    std::string errorCodeName; ///< 错误码字符串
};

AlgoErrorInfo parseAlgoError(AlgoError algoError);

/** 格式化错误描述：[模块名:xxx][错误码:yyy] */
std::string getAlgoErrorString(AlgoError algoError);

std::string getModuleName(uint32_t moduleId);

std::string getErrorCodeName(uint32_t moduleId, uint32_t errorCode);

} // namespace vision_algo
