#ifndef _MODBUS_MASTER_HH
#define _MODBUS_MASTER_HH

#include <vector>
#include <string>
#include <modbus.h>
#include "modbus_master_options.h"
#include "../../ireg_access.h"
#include "../../fairlock.h"

namespace common::regaccess {
namespace modbus {
class REGACCESS_EXPORT ModbusMaster : public IRegAccess {
public:
    ModbusMaster() = default;
    ~ModbusMaster();
public:
    int Start(const ModbusMasterOpts &masterOPts);
    void Stop();
    int ChangeSlave(int slaveID); // 只有Serial RTU才有切换salve节点ID，TCP是点对点的所以不需要

    // IRegAccess 接口实现：Modbus 使用十进制寄存器地址
    // 通过统一的 ConnectOptions 建立连接（当前实现主要面向 TCP，RTU 可在需要时扩展）
    bool Connect(const ConnectOptions &options) override;
    void Disconnect() override { Stop(); }

    bool WriteBit(const std::string &address, bool value) override;
    bool ReadBit(const std::string &address, bool &value) override;

    bool WriteUInt16(const std::string &address, std::uint16_t value) override;
    bool ReadUInt16(const std::string &address, std::uint16_t &value) override;

    bool WriteUInt32(const std::string &address, std::uint32_t value) override;
    bool ReadUInt32(const std::string &address, std::uint32_t &value) override;

    bool WriteFloat(const std::string &address, float value) override;
    bool ReadFloat(const std::string &address, float &value) override;

    bool WriteRegisters(const std::string &address,
                        const std::vector<std::uint16_t> &values) override;
    bool ReadRegisters(const std::string &address,
                       std::uint16_t regCnt,
                       std::vector<std::uint16_t> &values) override;

    bool WriteString(const std::string &address,
                     const std::string &value,
                     std::uint16_t regNum) override;
    bool ReadString(const std::string &address,
                    std::uint16_t regNum,
                    std::string &value) override;

    bool WriteBytes(const std::string &address,
                    const std::vector<std::uint8_t> &value) override;
    bool ReadBytes(const std::string &address,
                   std::uint16_t regNum,
                   std::vector<std::uint8_t> &value) override;

public:
    RegErrorCode GetLastErrorCode() const override { return lastErrorCode_; }
    std::string GetLastErrorMessage() const override { return lastErrorMsg_; }

private:
    RegErrorCode lastErrorCode_ = RegErrorCode::Ok;
    std::string  lastErrorMsg_;

    void setError(RegErrorCode code, const std::string &msg) {
        lastErrorCode_ = code;
        lastErrorMsg_  = msg;
    }

    UseBackend useBackend_ = UseBackend::TCP;
    modbus_t *ctx_ = nullptr;
    FairLock lock_;
};
} // namespace modbus
} // namespace common::regaccess

#endif
