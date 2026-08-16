#ifndef _LAMPHOUSE_XOR_FRAME_BASE_HH
#define _LAMPHOUSE_XOR_FRAME_BASE_HH

#include <string>
#include "lamphouse_base.h"

namespace common
{
namespace device
{

// 8 字节 XOR 校验帧协议基类（乐视 LTS、嘉励 JL、慕藤光 HDPS 等共用编解码）
class LAMPHOUSE_EXPORT XorFrameLamphouse : public LamphouseBase {
public:
    explicit XorFrameLamphouse(char frameHeader);
    ~XorFrameLamphouse() override = default;

protected:
    void setFrameHeader(char frameHeader);

    const std::string& encodeChannelCmd(char cmd, uint8_t channel,
                                        char data3 = '0', char data4 = '0', char data5 = '0');
    const std::string& encodeBrightnessCmd(uint8_t channel, uint8_t brightness);
    const std::string& encodeAllChannelsCmd(char cmd, char channelByte = '1');

    const std::string& open_chc(const uint8_t& ind) override;
    const std::string& close_chc(const uint8_t& ind) override;
    const std::string& set_brightness(const int& ind, const uint8_t& val) override;
    const std::string& open_all_chc() override;
    const std::string& close_all_chc() override;

private:
    void finalizeChecksum();

    static constexpr char kHexMap[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    std::string strSend_;
    char frameHeader_ = '#';
};

} // namespace device
} // namespace common

#endif
