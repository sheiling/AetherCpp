#include "lamphouse_xor_frame_base.h"

namespace common
{
namespace device
{

XorFrameLamphouse::XorFrameLamphouse(char frameHeader)
{
    strSend_.resize(8, '0');
    frameHeader_ = frameHeader;
    strSend_[0] = frameHeader_;
}

void XorFrameLamphouse::setFrameHeader(char frameHeader)
{
    frameHeader_ = frameHeader;
    strSend_[0] = frameHeader_;
}

void XorFrameLamphouse::finalizeChecksum()
{
    const uint8_t checksum = static_cast<uint8_t>(
        strSend_[0] ^ strSend_[1] ^ strSend_[2] ^ strSend_[3] ^ strSend_[4] ^ strSend_[5]);
    strSend_[6] = kHexMap[checksum >> 4];
    strSend_[7] = kHexMap[checksum & 0x0f];
}

const std::string& XorFrameLamphouse::encodeChannelCmd(char cmd, uint8_t channel,
                                                     char data3, char data4, char data5)
{
    strSend_[0] = frameHeader_;
    strSend_[1] = cmd;
    strSend_[2] = static_cast<char>('0' + channel);
    strSend_[3] = data3;
    strSend_[4] = data4;
    strSend_[5] = data5;
    finalizeChecksum();
    return strSend_;
}

const std::string& XorFrameLamphouse::encodeBrightnessCmd(uint8_t channel, uint8_t brightness)
{
    const uint8_t clamped = brightness > 255 ? 255 : brightness;
    const uint8_t highNibble = clamped / 16;
    const uint8_t lowNibble = clamped - highNibble * 16;

    strSend_[0] = frameHeader_;
    strSend_[1] = '3';
    strSend_[2] = static_cast<char>('0' + channel);
    strSend_[3] = '0';
    strSend_[4] = kHexMap[highNibble];
    strSend_[5] = kHexMap[lowNibble];
    finalizeChecksum();
    return strSend_;
}

const std::string& XorFrameLamphouse::encodeAllChannelsCmd(char cmd, char channelByte)
{
    strSend_[0] = frameHeader_;
    strSend_[1] = cmd;
    strSend_[2] = channelByte;
    strSend_[3] = '0';
    strSend_[4] = '0';
    strSend_[5] = '0';
    finalizeChecksum();
    return strSend_;
}

const std::string& XorFrameLamphouse::open_chc(const uint8_t& ind)
{
    return encodeChannelCmd('1', ind);
}

const std::string& XorFrameLamphouse::close_chc(const uint8_t& ind)
{
    return encodeChannelCmd('2', ind);
}

const std::string& XorFrameLamphouse::set_brightness(const int& ind, const uint8_t& val)
{
    return encodeBrightnessCmd(static_cast<uint8_t>(ind), val);
}

const std::string& XorFrameLamphouse::open_all_chc()
{
    return strSend_;
}

const std::string& XorFrameLamphouse::close_all_chc()
{
    return strSend_;
}

} // namespace device
} // namespace common
