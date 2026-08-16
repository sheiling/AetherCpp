#ifndef MODBUS_MASTER_OPTIONS_HH
#define MODBUS_MASTER_OPTIONS_HH

#include <string>

namespace common::regaccess {
namespace modbus {
enum UseBackend {
    TCP,        // Ethernet in TCP IPv4
    TCP_PI,     // Ethernet in TCP IPv4 and IPv6
    RTU         // serial in RTU
};

class ModbusMasterOpts {
public:
    // tcp、tcp_pi、rtu 通用参数
    UseBackend useBackend_;
    std::string ipOrDevice_;    // TCP(127.0.0.1), TCP_PI(::1), RTU(linux:/dev/ttyUSB1, windows:\\.\COM10)
    uint32_t rspTimeout_;       // 响应超时时间(单位:微秒)[0,999999]
    // tcp 专属参数
    int port_ = 0;              // salve 节点 port
    // tcp_pi 专属参数
    std::string service_;       // slave 节点 service name/port
    // rtu 专属参数
    int baud_;                  // 波特率
    char parity_;               // 串口通信校验方式(N:none, E:even, O:odd)
    int dataBit_;               // 串口通信数据位(5/6/7/8)
    int stopBit_;               // 串口通信停止位(1/2)
    int slaveID_;               // 从节点ID
    
    ModbusMasterOpts() {
        useBackend_ = UseBackend::TCP;
        ipOrDevice_ = "127.0.0.1";
        rspTimeout_ = 2000 * 1000;

        port_ = 8686;

        service_ = "8686";

        baud_ = 115200;
        parity_ = 'N';
        dataBit_ = 8;
        stopBit_ = 1;
        slaveID_ = 0;
    }

};

} // end of namespace modbus
} // end of namespace common::regaccess

#endif
