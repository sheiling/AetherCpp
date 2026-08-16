#include "melsec_request.h"
#include "../utils/Buffer.h"

namespace common::regaccess {
namespace mc {
    namespace
    {
        constexpr uint8_t SUB_HEADER     = 0x50;
        constexpr uint8_t NETWORK_NO     = 0x00;
        constexpr uint8_t PLC_NO         = 0xFF;
        constexpr uint8_t MODULE_IO_LOW  = 0xFF;
        constexpr uint8_t MODULE_IO_HIGH = 0x03;
        constexpr uint8_t STATION_NO     = 0x00;

        // 超时时间为 0x0010 * 250ms = 4000ms
        constexpr uint8_t CPU_TIMER_LOW  = 0x10;
        constexpr uint8_t CPU_TIMER_HIGH = 0x00;

        // 批量读取/批量写入，字单位
        constexpr uint8_t CMD_MAIN_LO    = 0x01; 
        constexpr uint8_t CMD_SUB_LO     = 0x00;
        constexpr uint8_t CMD_SUB_HI     = 0x00;

        constexpr uint16_t BASE_REQ_LEN  = 12;

        enum class Instruction : uint8_t
        {
            Read  = 0x04,
            Write = 0x14
        };
    } // unnamed namespace

    std::vector<uint8_t> MelsecRequest::pack() const
    {
        // 1. 计算请求数据长度与指令码
        uint16_t reqLen = static_cast<uint16_t>(BASE_REQ_LEN + data_.size());
        auto ins = data_.size() > 0 ? Instruction::Write : Instruction::Read;

        Buffer buffer;

        // 2. 固定头部（副头部 + 网络号 + 可编程控制器编号 + 模块 IO + 站号）
        buffer << SUB_HEADER                    // 副头部
            << static_cast<uint8_t>(0x00)       // 保留 / 未使用
            << NETWORK_NO                       // 网络号
            << PLC_NO                           // 可编程控制器编号
            << MODULE_IO_LOW                    // 模块 IO
            << MODULE_IO_HIGH                   // 模块 IO
            << STATION_NO;                      // 站号
        
        // 3. 请求数据长度与监视定时器
        buffer << reqLen                        // 请求数据长度
            << CPU_TIMER_LOW                    // 监视定时器
            << CPU_TIMER_HIGH;                  // 监视定时器

        // 4. 指令与子指令
        buffer << CMD_MAIN_LO                   // 指令
            << static_cast<uint8_t>(ins)        // 指令
            << CMD_SUB_LO                       // 子指令
            << CMD_SUB_HI;                      // 子指令

        // 5. 地址与类型
        buffer << static_cast<uint8_t>(address_ % 256)                   // 地址
            << static_cast<uint8_t>(address_ >> 8)                       // 地址
            << static_cast<uint8_t>(address_ >> 16)                      // 地址
            << addressType_                                              // 地址类型
            << reqCnt_;                                                  // 软原件点数

        // 6. 写请求时追加数据
        if (data_.size() > 0) buffer << data_;

        return buffer.data();
    }
} // end of namespace mc
} // end of namespace common::regaccess