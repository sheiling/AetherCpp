#include "rangefinder_base.h"

namespace common
{
namespace device
{
bool RangefinderBase::Init()
{
    if (m_rangefinderDevInfo_.protocolType == "TCP")
        return InitByTCP();
    else if (m_rangefinderDevInfo_.protocolType == "RTU")
        return InitByRTU();
    else
        return false;
}

bool RangefinderBase::InitByTCP()
{
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::TCP;
    modbusMasterOpts.ipOrDevice_ = m_rangefinderDevInfo_.ip;
    modbusMasterOpts.port_ = m_rangefinderDevInfo_.port;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    int32_t iret = modbusMastes_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}
bool RangefinderBase::InitByRTU()
{
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::RTU;
    // Windows 串口路径需要前缀 "\\.\\"
    modbusMasterOpts.ipOrDevice_ = "\\\\.\\" + m_rangefinderDevInfo_.serialInfo.com_;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    modbusMasterOpts.baud_ = m_rangefinderDevInfo_.serialInfo.baud_;
    if (m_rangefinderDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Even) {
        modbusMasterOpts.parity_ = 'E';
    }
    else if (m_rangefinderDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Odd) {
        modbusMasterOpts.parity_ = 'O';
    }
    else {
        modbusMasterOpts.parity_ = 'N';
    }
    modbusMasterOpts.dataBit_ = m_rangefinderDevInfo_.serialInfo.dataBit_;
    modbusMasterOpts.stopBit_ = m_rangefinderDevInfo_.serialInfo.stopBit_;
    modbusMasterOpts.slaveID_ = m_rangefinderDevInfo_.serialInfo.slaveID_;
    int32_t iret = modbusMastes_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

int RangefinderBase::ChangeSlave(int slaveID)
{
    if (m_rangefinderDevInfo_.protocolType == "RTU")
        return (modbusMastes_.ChangeSlave(slaveID));
    else
        return 0;
}

int RangefinderBase::WriteBit(int address, uint8_t bitValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.WriteBit(addr, bitValue != 0);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadBit(int address, uint8_t &bitValue) {
    const std::string addr = std::to_string(address);
    bool b = false;
    bool ok = modbusMastes_.ReadBit(addr, b);
    if (ok) bitValue = b ? 1 : 0;
    return ok ? 0 : -1;
}

int RangefinderBase::WriteBits(int address, const std::vector<uint8_t> &bitValues) {
    const std::string addr = std::to_string(address);
    // 直接按字节序写入（底层按位写入）
    bool ok = modbusMastes_.WriteBytes(addr, bitValues);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadBits(int address, uint16_t bitCnt, std::vector<uint8_t> &bitValues) {
    const std::string addr = std::to_string(address);
    // 以寄存器数量估算读取长度（每寄存器2字节）；向上取整为至少覆盖 bitCnt 个元素
    uint16_t regNum = (bitCnt + 1) / 2;
    std::vector<uint8_t> tmp;
    bool ok = modbusMastes_.ReadBytes(addr, regNum, tmp);
    if (ok) {
        bitValues.assign(tmp.begin(), tmp.begin() + std::min<std::size_t>(bitCnt, tmp.size()));
    }
    return ok ? 0 : -1;
}

int RangefinderBase::WriteRegister(int address, uint16_t dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.WriteUInt16(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::WriteRegister(int address, uint32_t dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.WriteUInt32(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::WriteRegister(int address, float dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.WriteFloat(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadRegister(int address, uint16_t &dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.ReadUInt16(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadRegister(int address, uint32_t &dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.ReadUInt32(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadRegister(int address, float &dataValue) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.ReadFloat(addr, dataValue);
    return ok ? 0 : -1;
}

int RangefinderBase::WriteRegisters(int address, const std::vector<uint16_t> &dataValues) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.WriteRegisters(addr, dataValues);
    return ok ? 0 : -1;
}

int RangefinderBase::ReadRegisters(int address, uint16_t dataCnt, std::vector<uint16_t> &dataValues) {
    const std::string addr = std::to_string(address);
    bool ok = modbusMastes_.ReadRegisters(addr, dataCnt, dataValues);
    return ok ? 0 : -1;
}

}

}
