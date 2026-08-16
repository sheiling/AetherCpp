#pragma once
#ifndef _JL_DVC_LAMPHOUSE_HH
#define _JL_DVC_LAMPHOUSE_HH

#include <common/core/device/lamphouse/lamphouse_xor_frame_base.h>

namespace common
{
namespace device
{

// 上海嘉励 JL-DVC 系列（XOR 帧协议，帧头固定 '#'，不支持全开/全关）
class LAMPHOUSE_EXPORT JlDvcLamphouse : public XorFrameLamphouse {
public:
    JlDvcLamphouse();
    ~JlDvcLamphouse() override = default;
};

} // namespace device
} // namespace common

#endif
