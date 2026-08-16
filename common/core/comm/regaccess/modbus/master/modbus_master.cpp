#include "modbus_master.h"

namespace common::regaccess {
namespace modbus {

ModbusMaster::~ModbusMaster() {
    Stop();
}

bool ModbusMaster::Connect(const ConnectOptions &options) {
    // 将统一的 ConnectOptions 映射为 Modbus 专用配置：
    // - 当 type == Tcp 时，按 TCP 处理（ipOrDevice_ 为 IP，port_ 为端口）
    // - 当 type == SerialRtu 时，按 RTU 串口处理（ipOrDevice_ 为串口设备名，如 "COM3" 或 "/dev/ttyUSB0"）
    ModbusMasterOpts opts;

    if (options.type == ConnectType::Tcp) {
        opts.useBackend_ = UseBackend::TCP;
        opts.ipOrDevice_ = options.host;
        opts.port_       = options.port;
    } else {
        opts.useBackend_ = UseBackend::RTU;
        opts.ipOrDevice_ = options.serialDevice.empty() ? options.host : options.serialDevice; // 串口设备名
        opts.baud_       = options.serialBaud;
        opts.parity_     = options.serialParity;
        opts.dataBit_    = options.serialDataBits;
        opts.stopBit_    = options.serialStopBits;
    }

    opts.rspTimeout_ = options.timeoutMs * 1000; // 接口使用毫秒，这里转换为微秒
    opts.slaveID_    = options.slaveId;

    int ret = Start(opts);
    if (ret != 0) {
        // Start 内部已设置详细错误信息，这里仅返回 false
        return false;
    }
    setError(RegErrorCode::Ok, "");
    return true;
}

int ModbusMaster::Start(const ModbusMasterOpts &masterOPts) {
    useBackend_ = masterOPts.useBackend_;

    switch (masterOPts.useBackend_) {
        case UseBackend::TCP: {
            ctx_ = modbus_new_tcp(masterOPts.ipOrDevice_.c_str(), masterOPts.port_);
            break;
        }
        case UseBackend::TCP_PI: {
            ctx_ = modbus_new_tcp_pi(masterOPts.ipOrDevice_.c_str(), masterOPts.service_.c_str());
            break;
        }
        case UseBackend::RTU: {
            ctx_ = modbus_new_rtu(masterOPts.ipOrDevice_.c_str(), masterOPts.baud_, masterOPts.parity_, masterOPts.dataBit_, masterOPts.stopBit_);
            break;
        }
        default: {
            break;
        }
    }

    if (ctx_ == nullptr) {
        setError(RegErrorCode::TransportError, "Unable to allocate libmodbus context");
        return -1;
    }

    // 开启Debug模式
    // modbus_set_debug(ctx_, TRUE);

    // RTU 设置从节点ID
    // if (useBackend_ == UseBackend::RTU) {
    //     modbus_set_slave(ctx_, masterOPts.slaveID_);
    // }

    modbus_set_slave(ctx_, masterOPts.slaveID_);

    // 设置响应超时时间
    modbus_set_response_timeout(ctx_, 0, masterOPts.rspTimeout_);

    // 连接modbus slave节点
    if (modbus_connect(ctx_) == -1) {
        setError(RegErrorCode::TransportError,
                 std::string("modbus connection failed: ") + modbus_strerror(errno));
        modbus_free(ctx_);
        ctx_ = nullptr;
        return -1;
    }

    return 0;
}

void ModbusMaster::Stop() {
	if (ctx_) {
		modbus_close(ctx_);
		modbus_free(ctx_);
        ctx_ = nullptr;
        setError(RegErrorCode::Ok, "");
	}
}

int ModbusMaster::ChangeSlave(int slaveID) {
    if (!ctx_) {
        return -1;
    }

    if (useBackend_ == UseBackend::RTU) {
        modbus_set_slave(ctx_, slaveID);
    }
    return 0;
}

// IRegAccess 接口实现
namespace {
    inline bool parseAddress(const std::string &addr, int &out)
    {
        try {
            out = std::stoi(addr);
            return true;
        } catch (...) {
            return false;
        }
    }
}

bool ModbusMaster::WriteUInt16(const std::string &address, std::uint16_t value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteRegister(uint16_t)");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    int iret = modbus_write_register(ctx_, addr, value);
    if (iret != 1) {
        std::string errmsg = "modbus_write_register failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    return true;
}

bool ModbusMaster::ReadUInt16(const std::string &address, std::uint16_t &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadUInt16");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t *tab_rp_registers = (uint16_t*)malloc(1 * sizeof(uint16_t));
    memset(tab_rp_registers, 0, sizeof(uint16_t));
    int iret = modbus_read_registers(ctx_, addr, 1, tab_rp_registers);
    if (iret != 1) {
        free(tab_rp_registers);
        tab_rp_registers = nullptr;
        std::string errmsg = "modbus_read_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }
    value = tab_rp_registers[0];
    free(tab_rp_registers);
    tab_rp_registers = nullptr;
    return true;
}

bool ModbusMaster::WriteUInt32(const std::string &address, std::uint32_t value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteUInt32");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t *tab_registers = (uint16_t*)malloc(sizeof(uint32_t));
    memset(tab_registers, 0, sizeof(uint32_t));

    uint16_t h_value = (value >> 16) & 0xFFFF;
    uint16_t l_value = (value & 0xFFFF);

    memcpy(tab_registers, &h_value, sizeof(uint16_t));
    memcpy(tab_registers + 1, &l_value, sizeof(uint16_t));

    int iret = modbus_write_registers(ctx_, addr, 2, tab_registers);
    if (iret != 2) {
        free(tab_registers);
        tab_registers = nullptr;
        std::string errmsg = "modbus_write_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    free(tab_registers);
    tab_registers = nullptr;

    return true;
}

bool ModbusMaster::ReadUInt32(const std::string &address, std::uint32_t &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadUInt32");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t *tab_rp_registers = (uint16_t*)malloc(sizeof(uint32_t));
    memset(tab_rp_registers, 0, sizeof(uint32_t));

    int iret = modbus_read_registers(ctx_, addr, 2, tab_rp_registers);
    if (iret != 2) {
        free(tab_rp_registers);
        tab_rp_registers = nullptr;
        std::string errmsg = "modbus_read_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }

    value = ((tab_rp_registers[0] & 0xFFFF) << 16) + (tab_rp_registers[1] & 0xFFFF);

    free(tab_rp_registers);
    tab_rp_registers = nullptr;
    return true;
}

bool ModbusMaster::WriteFloat(const std::string &address, float value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteFloat");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t dataCnt = sizeof(float) / sizeof(uint16_t);

    uint16_t *tab_registers = (uint16_t*)malloc(sizeof(float));
    memset(tab_registers, 0, sizeof(float));

    modbus_set_float(value, tab_registers);

    int iret = modbus_write_registers(ctx_, addr, dataCnt, tab_registers);
    if (iret != dataCnt) {
        free(tab_registers);
        tab_registers = nullptr;
        std::string errmsg = "modbus_write_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }
    free(tab_registers);
    tab_registers = nullptr;

    return true;
}

bool ModbusMaster::ReadFloat(const std::string &address, float &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadFloat");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t *tab_rp_registers = (uint16_t*)malloc(sizeof(float));
    memset(tab_rp_registers, 0, sizeof(float));

    int iret = modbus_read_registers(ctx_, addr, 2, tab_rp_registers);
    if (iret != 2) {
        free(tab_rp_registers);
        tab_rp_registers = nullptr;
        std::string errmsg = "modbus_read_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }

    value = modbus_get_float(tab_rp_registers);

    free(tab_rp_registers);
    tab_rp_registers = nullptr;
    return true;
}

bool ModbusMaster::WriteString(const std::string &address,
                               const std::string &value,
                               std::uint16_t regNum)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteString");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    std::vector<uint8_t> dataValues(regNum*2, 0);
    for (int i = 0; i < value.size(); i++)
    {
        dataValues[i] = static_cast<uint8_t>(value[i]);
    }

    // 将8位数据转换为16位寄存器值（大端序）
    std::vector<uint16_t> registers(dataValues.size() / 2);
    for (size_t i = 0; i < registers.size(); ++i) {
        registers[i] = (dataValues[2*i] << 8) | dataValues[2*i + 1];
    }

    // 写入寄存器
    if (modbus_write_registers(ctx_, addr, registers.size(), registers.data()) == -1) {
        std::string errmsg = "modbus_write_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    return true;
}

bool ModbusMaster::ReadString(const std::string &address,
                              std::uint16_t regNum,
                              std::string &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadString");
        return false;
    }   

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    std::vector<uint16_t> regs(regNum);
    
    // 读取寄存器数据
    if (modbus_read_registers(ctx_, addr, regNum, regs.data()) == -1) {
        std::string errmsg = "modbus_read_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    
    // 将16位数据转换为8位vector
    std::vector<uint8_t> dataValues;
    dataValues.reserve(regNum * 2);
    
    for (uint16_t val : regs) {
        // 默认按大端序存储 (高字节在前)
        dataValues.push_back(static_cast<uint8_t>(val >> 8));
        dataValues.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    std::string str(dataValues.begin(), dataValues.end());
    auto newEnd = std::remove(str.begin(), str.end(), '\0');
    str.erase(newEnd, str.end());
    value = str;
    return true;
}

bool ModbusMaster::WriteBytes(const std::string &address,
                              const std::vector<std::uint8_t> &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteBytes");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t bitCnt = value.size();
    uint8_t *tab_value = (uint8_t*)malloc(bitCnt * sizeof(uint8_t));
    memset(tab_value, 0, bitCnt * sizeof(uint8_t));
    for (uint16_t i = 0; i < bitCnt; ++i) {
        tab_value[i] = value[i];
    }
    int iret = modbus_write_bits(ctx_, addr, bitCnt, tab_value);
    if (iret != bitCnt) {
        free(tab_value);
        tab_value = nullptr;
        std::string errmsg = "modbus_write_bits failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    free(tab_value);
    tab_value = nullptr;
    return true;
}

bool ModbusMaster::ReadBytes(const std::string &address,
                             std::uint16_t regNum,
                             std::vector<std::uint8_t> &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadBytes");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t bitCnt = regNum * 2;
    uint8_t *tab_rp_bits = (uint8_t*)malloc(bitCnt * sizeof(uint8_t));
    memset(tab_rp_bits, 0, bitCnt * sizeof(uint8_t));
    int iret = modbus_read_bits(ctx_, addr, bitCnt, tab_rp_bits);
    if (iret != bitCnt) {
        free(tab_rp_bits);
        tab_rp_bits = nullptr;
        std::string errmsg = "modbus_read_bits failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }

    for (uint16_t i = 0; i < bitCnt; ++i) {
        value.emplace_back(tab_rp_bits[i]);
    }

    free(tab_rp_bits);
    tab_rp_bits = nullptr;
    return true;
}

bool ModbusMaster::WriteBit(const std::string &address, bool value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteBit");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint8_t bit = value ? 1u : 0u;
    int iret = modbus_write_bit(ctx_, addr, bit);
    if (iret != 1) {
        std::string errmsg = "modbus_write_bit failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;
    }
    return true;
}

bool ModbusMaster::ReadBit(const std::string &address, bool &value)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadBit");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint8_t *tab_rp_bits = (uint8_t*)malloc(1 * sizeof(uint8_t));
    memset(tab_rp_bits, 0, sizeof(uint8_t));
    int iret = modbus_read_bits(ctx_, addr, 1, tab_rp_bits);
    if (iret != 1) {
        free(tab_rp_bits);
        tab_rp_bits = nullptr;
        std::string errmsg = "modbus_read_bits failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }
    value = tab_rp_bits[0];
    free(tab_rp_bits);
    tab_rp_bits = nullptr;
    return true;
}

bool ModbusMaster::WriteRegisters(const std::string &address,
                                  const std::vector<std::uint16_t> &values)
{   
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in WriteRegisters");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t dataCnt = values.size();

    uint16_t *tab_registers = (uint16_t*)malloc(dataCnt * sizeof(uint16_t));
    memset(tab_registers, 0, dataCnt * sizeof(uint16_t));

    for (uint32_t i = 0; i < dataCnt; ++i) {
        memcpy(tab_registers + i, &(values[i]), sizeof(uint16_t));
    }
    
    int iret = modbus_write_registers(ctx_, addr, dataCnt, tab_registers);
    if (iret != dataCnt) {
        free(tab_registers);
        tab_registers = nullptr;
        std::string errmsg = "modbus_write_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }
    free(tab_registers);
    tab_registers = nullptr;
    return true;
}

bool ModbusMaster::ReadRegisters(const std::string &address,
                                 std::uint16_t regCnt,
                                 std::vector<std::uint16_t> &values)
{
    if (!ctx_) {
        setError(RegErrorCode::NotConnected, "modbus context is null in ReadRegisters");
        return false;
    }

    int addr = 0;
    if (!parseAddress(address, addr)) {
        setError(RegErrorCode::InvalidAddress, "Invalid address: " + address);
        return false;
    }

    FairLock::ScopedLock lck(lock_);

    uint16_t *tab_rp_registers = (uint16_t*)malloc(regCnt * sizeof(uint16_t));
    memset(tab_rp_registers, 0, regCnt * sizeof(uint16_t));
    int iret = modbus_read_registers(ctx_, addr, regCnt, tab_rp_registers);
    if (iret != regCnt) {
        free(tab_rp_registers);
        tab_rp_registers = nullptr;
        std::string errmsg = "modbus_read_registers failed: " + std::string(modbus_strerror(errno));
        setError(RegErrorCode::ProtocolError, errmsg);
        return false;  
    }

    for (uint16_t i = 0; i < regCnt; ++i) {
        values.emplace_back(tab_rp_registers[i]);
    }


    free(tab_rp_registers);
    tab_rp_registers = nullptr;
    return true;
}

} // end of namespace modbus
} // end of namespace common::regaccess