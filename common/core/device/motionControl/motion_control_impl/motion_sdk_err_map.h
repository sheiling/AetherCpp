#ifndef COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_SDK_ERR_MAP_H
#define COMMON_CORE_DEVICE_MOTION_CONTROL_MOTION_SDK_ERR_MAP_H

#include <cstdint>
#include <map>
#include <string>

/** 厂商 SDK 返回值 → 可读字符串（与 motion_error_codes.h 业务 ErrorPtr 码无关） */
extern std::map<uint16_t, std::string> LTSMC_ERR_CODE_MAP;
extern std::map<uint16_t, std::string> SOLID_ERR_CODE_MAP;

#endif
