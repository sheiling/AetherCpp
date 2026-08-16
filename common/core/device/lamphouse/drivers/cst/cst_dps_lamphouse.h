#pragma once
#ifndef _CST_DPS_LAMPHOUSE_HH
#define _CST_DPS_LAMPHOUSE_HH

#include <string>
#include <common/core/device/lamphouse/lamphouse_base.h>

namespace common
{
namespace device
{

// 康视达 CST-DPS（SA 前缀十进制亮度协议，与 XOR 帧系列不同）
class LAMPHOUSE_EXPORT CstDpsLamphouse : public LamphouseBase {
public:
    CstDpsLamphouse();
    ~CstDpsLamphouse() override = default;

    const std::string& open_chc(const uint8_t& ind) override;
    const std::string& close_chc(const uint8_t& ind) override;
    const std::string& set_brightness(const int& ind, const uint8_t& val) override;
    const std::string& open_all_chc() override;
    const std::string& close_all_chc() override;

private:
    std::string strSend_;
};

} // namespace device
} // namespace common

#endif
