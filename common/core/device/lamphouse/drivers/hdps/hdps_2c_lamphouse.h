#pragma once
#ifndef _HDPS_2C_LAMPHOUSE_HH
#define _HDPS_2C_LAMPHOUSE_HH

#include <common/core/device/lamphouse/lamphouse_xor_frame_base.h>

namespace common
{
namespace device
{

// 慕藤光 HDPS-2C（XOR 帧协议；开通道时 data4/data5 为固定 0x64）
class LAMPHOUSE_EXPORT Hdps2cLamphouse : public XorFrameLamphouse {
public:
    Hdps2cLamphouse();
    ~Hdps2cLamphouse() override = default;

    const std::string& open_chc(const uint8_t& ind) override;
};

} // namespace device
} // namespace common

#endif
