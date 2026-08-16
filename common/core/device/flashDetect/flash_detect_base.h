#ifndef _FLASH_DETECT_BASE_HH
#define _FLASH_DETECT_BASE_HH

#include <cstdint>
#include <common/utility/json_tool/reflector.h>
#include "flash_detect_global.h"

namespace common
{
namespace device
{

// Flash 检测设备型号
enum FlashDetectDevType {
    FD_BY_SERIAL = 0,   // 串口协议 Flash 检测
    FD_UNKNOWN          // 未知型号
};
NLOHMANN_JSON_SERIALIZE_ENUM(FlashDetectDevType, {
    { FlashDetectDevType::FD_BY_SERIAL, "FD_BY_SERIAL" },
    { FlashDetectDevType::FD_UNKNOWN,  "FD_UNKNOWN" }
});

class FLASH_DETECT_EXPORT FlashDetectBase
{
public:
    FlashDetectBase() = default;
    virtual ~FlashDetectBase() = default;

    void reflectFromJson(const json& j)
    {
        getFromJson(j, "type", m_typeName);
        reflectConfigFromJson(j);
    }
    void reflectToJson(json& j) const
    {
        j["type"] = m_typeName;
        reflectConfigToJson(j);
    }

    FlashDetectDevType type() const { return m_typeName; }

    virtual bool IsOpened() const = 0;
    virtual int OpenComm() = 0;
    virtual int CloseComm() = 0;
    virtual int CheckFlash() = 0;
    virtual int CheckFlash(uint8_t destAddr, uint8_t portAddr, uint16_t boardAddr) = 0;

protected:
    virtual void reflectConfigFromJson(const json& j) = 0;
    virtual void reflectConfigToJson(json& j) const = 0;

    FlashDetectDevType m_typeName = FlashDetectDevType::FD_UNKNOWN;
};

} // namespace device
} // namespace common

#endif
