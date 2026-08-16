#include "pressurepickup_base.h"

namespace common
{
namespace device
{

bool PressurePickupBase::Init()
{
    if (m_pressurePickupDevInfo_.protocolType == "TCP")
        return InitByTCP();
    else if (m_pressurePickupDevInfo_.protocolType == "RTU")
        return InitByRTU();
    else
        return false;
}

bool PressurePickupBase::InitByTCP()
{
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::TCP;
    modbusMasterOpts.ipOrDevice_ = m_pressurePickupDevInfo_.ip;
    modbusMasterOpts.port_ = m_pressurePickupDevInfo_.port;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    int32_t iret = modbusMaster_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}
bool PressurePickupBase::InitByRTU()
{
    common::regaccess::modbus::ModbusMasterOpts modbusMasterOpts;
    modbusMasterOpts.useBackend_ = common::regaccess::modbus::UseBackend::RTU;
    modbusMasterOpts.ipOrDevice_ = "\\.\\" + m_pressurePickupDevInfo_.serialInfo.com_;
    modbusMasterOpts.rspTimeout_ = 2000 * 1000;
    modbusMasterOpts.baud_ = m_pressurePickupDevInfo_.serialInfo.baud_;
    if (m_pressurePickupDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Even) {
        modbusMasterOpts.parity_ = 'E';
    }
    else if (m_pressurePickupDevInfo_.serialInfo.parity_ == common::serial::ParityMode::Odd) {
        modbusMasterOpts.parity_ = 'O';
    }
    else {
        modbusMasterOpts.parity_ = 'N';
    }
    modbusMasterOpts.dataBit_ = m_pressurePickupDevInfo_.serialInfo.dataBit_;
    modbusMasterOpts.stopBit_ = m_pressurePickupDevInfo_.serialInfo.stopBit_;
    int32_t iret = modbusMaster_.Start(modbusMasterOpts);
    return (iret == 0 ? true : false);
}

int PressurePickupBase::ChangeSlave(int slaveID)
{
    if (m_pressurePickupDevInfo_.protocolType == "RTU")
        return (modbusMaster_.ChangeSlave(slaveID));
    else
        return 0;
}

int PressurePickupBase::WriteBit(int address, uint8_t bitValue) {
    bool ok = modbusMaster_.WriteBit(std::to_string(address), bitValue != 0);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadBit(int address, uint8_t &bitValue) {
    bool value = false;
    bool ok = modbusMaster_.ReadBit(std::to_string(address), value);
    if (ok) bitValue = value ? 1 : 0;
    return ok ? 0 : -1;
}

int PressurePickupBase::WriteBits(int address, const std::vector<uint8_t> &bitValues) {
    // 将 uint8_t 数组转换为要写入的位序列（作为字节流写入保持兼容性）
    std::vector<uint8_t> bytes = bitValues;
    bool ok = modbusMaster_.WriteBytes(std::to_string(address), bytes);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadBits(int address, uint16_t bitCnt, std::vector<uint8_t> &bitValues) {
    std::vector<uint8_t> bytes;
    bool ok = modbusMaster_.ReadBytes(std::to_string(address), bitCnt, bytes);
    if (ok) bitValues = std::move(bytes);
    return ok ? 0 : -1;
}

int PressurePickupBase::WriteRegister(int address, uint16_t dataValue) {
    bool ok = modbusMaster_.WriteUInt16(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::WriteRegister(int address, uint32_t dataValue) {
    bool ok = modbusMaster_.WriteUInt32(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::WriteRegister(int address, float dataValue) {
    bool ok = modbusMaster_.WriteFloat(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadRegister(int address, uint16_t &dataValue) {
    bool ok = modbusMaster_.ReadUInt16(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadRegister(int address, uint32_t &dataValue) {
    bool ok = modbusMaster_.ReadUInt32(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadRegister(int address, float &dataValue) {
    bool ok = modbusMaster_.ReadFloat(std::to_string(address), dataValue);
    return ok ? 0 : -1;
}

int PressurePickupBase::WriteRegisters(int address, const std::vector<uint16_t> &dataValues) {
    bool ok = modbusMaster_.WriteRegisters(std::to_string(address), dataValues);
    return ok ? 0 : -1;
}

int PressurePickupBase::ReadRegisters(int address, uint16_t dataCnt, std::vector<uint16_t> &dataValues) {
    bool ok = modbusMaster_.ReadRegisters(std::to_string(address), dataCnt, dataValues);
    return ok ? 0 : -1;
}

}

}