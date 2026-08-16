#include "melsec_response.h"
#include "../utils/Buffer.h"

namespace common::regaccess {
namespace mc {
    namespace
    {
        // 协议相关常量（根据当前实现推导，仅用于增强语义）
        constexpr std::size_t kFixedHeaderSize = 9;          // deputyHead_ + reserved + network + controller + targetIO + module + respLen(2)
        constexpr std::size_t kErrorCodeSize   = 2;          // 结束代码长度
        constexpr uint16_t    kErrorCodeOk     = 0;          // 正常结束代码
    } // unnamed namespace

    bool MelsecResponse::expand(const std::vector<uint8_t>& data)
    {
        // 1. 基本长度检查：至少要有固定头 + 结束代码
        if (data.size() < (kFixedHeaderSize + kErrorCodeSize))
        {
            // 数据长度异常，重置为安全状态
            deputyHead_ = 0;
            dataLen_    = 0;
            errorCode_  = 0xFFFF;    // 非协议定义的错误码，用于标识本地解析错误
            data_.clear();
            return false;
        }

        uint8_t  reserved         = 0;
        uint8_t  networkNumber    = 0;
        uint8_t  controllerNumber = 0;
        uint16_t targetIONumber   = 0;
        uint8_t  moduleNumber     = 0;
        uint16_t respLen          = 0;

        Buffer buffer(data);

        // 2. 解析固定头部与结束代码
        buffer >> deputyHead_               // 响应副头部
               >> reserved
               >> networkNumber             // 网络编号
               >> controllerNumber          // 可编程控制器编号
               >> targetIONumber            // 目标模块IO编号
               >> moduleNumber              // 请求目标模块站号
               >> respLen                   // 响应数据长度
               >> errorCode_;               // 结束代码

        // 3. 再次根据 respLen 做安全检查
        if (respLen < kErrorCodeSize)
        {
            // respLen 不可能小于结束代码长度，认为是异常
            dataLen_   = 0;
            errorCode_ = 0xFFFF;
            data_.clear();
            return false;
        }

        // 固定头部之后应该至少有 respLen 字节的数据（其中包含结束代码 + 可选数据区）
        const std::size_t expectedTotalSize = kFixedHeaderSize + respLen;
        if (data.size() < expectedTotalSize)
        {
            // 实际数据长度不足，认为是截断或异常
            dataLen_   = 0;
            errorCode_ = 0xFFFF;
            data_.clear();
            return false;
        }

        // 4. 正常响应，获取数据（errorCode_ == kErrorCodeOk）
        if (errorCode_ == kErrorCodeOk)
        {
            dataLen_ = static_cast<uint16_t>(respLen - kErrorCodeSize);
            data_.resize(dataLen_);
            buffer >> data_;
            return true;
        }
        else
        {
            // 有错误码时，清空数据区，仅保留错误信息
            dataLen_ = 0;
            data_.clear();
            return false;
        }  
    }
} // end of namespace mc
} // end of namespace common::regaccess