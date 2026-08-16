#include "hdps_2c_lamphouse.h"

namespace common
{
namespace device
{

Hdps2cLamphouse::Hdps2cLamphouse()
    : XorFrameLamphouse('#')
{
}

const std::string& Hdps2cLamphouse::open_chc(const uint8_t& ind)
{
    return encodeChannelCmd('1', ind, '0', '6', '4');
}

} // namespace device
} // namespace common
