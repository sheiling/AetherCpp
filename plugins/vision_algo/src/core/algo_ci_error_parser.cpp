#include "algo_ci_error_parser.h"

#include <ci_basic_error.h>
#include <ci_detection_modname.h>

#include <sstream>
#include <unordered_map>

namespace vision_algo {
namespace {

std::string basicErrorCodeName(uint32_t errorCode)
{
    switch (errorCode) {
    case 1:
        return "ALGO_INPUT_NULLPTR";
    case 2:
        return "ALGO_INPUT_EMPTY";
    case 3:
        return "ALGO_INPUT_INVALID";
    case 4:
        return "ALGO_INPUT_MISMATCH";
    case 10:
        return "ALGO_ERROR_FILE_VERSION";
    case 11:
        return "ALGO_ERROR_EXTNAME";
    case 12:
        return "ALGO_ERROR_FILE_ENDIANNESS";
    case 13:
        return "ALGO_ERROR_READ_FILE";
    case 14:
        return "ALGO_ERROR_WRITE_FILE";
    case 15:
        return "ALGO_MISMATCH_FILE_SIZE";
    case 16:
        return "ALGO_BUFFER_SIZE_INSUFFICIENT";
    case 17:
        return "ALGO_ERROR_MODCODE_MISMATCH";
    case 20:
        return "ALGO_EMPTY_DATA";
    case 21:
        return "ALGO_ERROR_HEADER";
    case 22:
        return "ALGO_INVALID_METHOD";
    default:
        break;
    }
    return {};
}

const std::unordered_map<uint32_t, const char *> &moduleNameMap()
{
    static const std::unordered_map<uint32_t, const char *> kMap = {
        {ALGOMOD_MODEL_PROCESSOR, "ALGOMOD_MODEL_PROCESSOR"},
        {ALGOMOD_CHIP_DETECTOR, "ALGOMOD_CHIP_DETECTOR"},
        {ALGOMOD_MARKER_DETECTOR, "ALGOMOD_MARKER_DETECTOR"},
        {ALGOMOD_DIE_BONDING_DEFECT_LOCATOR, "ALGOMOD_DIE_BONDING_DEFECT_LOCATOR"},
        {ALGOMOD_DL_DIE_BONDING_DETECTOR, "ALGOMOD_DL_DIE_BONDING_DETECTOR"},
    };
    return kMap;
}

} // namespace

std::string getModuleName(uint32_t moduleId)
{
    const auto &map = moduleNameMap();
    const auto  it  = map.find(moduleId);
    if (it != map.end())
        return it->second;

    std::ostringstream oss;
    oss << "ALGOMOD_UNKNOWN_" << moduleId;
    return oss.str();
}

std::string getErrorCodeName(uint32_t moduleId, uint32_t errorCode)
{
    (void)moduleId;
    if (const std::string basic = basicErrorCodeName(errorCode); !basic.empty())
        return basic;

    std::ostringstream oss;
    oss << "ERROR_CODE_" << errorCode;
    return oss.str();
}

AlgoErrorInfo parseAlgoError(AlgoError algoError)
{
    AlgoErrorInfo info;
    info.moduleId      = ERROR_MODULE_NAME(algoError);
    info.errorCode     = ERROR_CODE_NO_MODULE(algoError);
    info.moduleName    = getModuleName(info.moduleId);
    info.errorCodeName = getErrorCodeName(info.moduleId, info.errorCode);
    return info;
}

std::string getAlgoErrorString(AlgoError algoError)
{
    const AlgoErrorInfo info = parseAlgoError(algoError);
    return "[模块名:" + info.moduleName + "][错误码:" + info.errorCodeName + "]";
}

} // namespace vision_algo
