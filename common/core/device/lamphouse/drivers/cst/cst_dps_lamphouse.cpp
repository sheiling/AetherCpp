#include "cst_dps_lamphouse.h"

namespace common
{
namespace device
{

CstDpsLamphouse::CstDpsLamphouse()
{
    strSend_.resize(8, '0');
    strSend_[0] = '#';
}

const std::string& CstDpsLamphouse::open_chc(const uint8_t& /*ind*/)
{
    return strSend_;
}

const std::string& CstDpsLamphouse::close_chc(const uint8_t& /*ind*/)
{
    return strSend_;
}

const std::string& CstDpsLamphouse::set_brightness(const int& ind, const uint8_t& val)
{
    strSend_[0] = 'S';
    strSend_[1] = static_cast<char>('A' + (ind - 1));

    const uint8_t clamped = val > 255 ? 255 : val;
    strSend_[2] = '0';
    strSend_[3] = static_cast<char>('0' + clamped / 100);
    strSend_[4] = static_cast<char>('0' + (clamped / 10) % 10);
    strSend_[5] = static_cast<char>('0' + clamped % 10);
    strSend_[6] = '#';

    return strSend_;
}

const std::string& CstDpsLamphouse::open_all_chc()
{
    return strSend_;
}

const std::string& CstDpsLamphouse::close_all_chc()
{
    return strSend_;
}

} // namespace device
} // namespace common
