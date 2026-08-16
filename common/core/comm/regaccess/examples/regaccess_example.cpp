#include "../ireg_access.h"
#include "../keyence/client/kv_client.h"
#include "../melsec/client/melsec_client.h"
#include "../modbus/master/modbus_master.h"

#include <iostream>
#include <memory>

using namespace common::regaccess;

int main()
{
    std::cout << "IRegAccess unified API example." << std::endl;

    // ───────────────────── Keyence KV (TCP) 示例 ─────────────────────
    kv::KVClient kvClient;
    IRegAccess* kvAccess = &kvClient;

    ConnectOptions kvOpts;
    kvOpts.type            = ConnectType::Tcp;
    kvOpts.host            = "127.0.0.1";
    kvOpts.port            = 8501;          // KV 监听端口，根据实际设备修改
    kvOpts.tcpReconnEnable = true;

    if (!kvAccess->Connect(kvOpts)) {
        std::cerr << "KV connect failed: " << kvAccess->GetLastErrorMessage() << std::endl;
    } else {
        std::uint16_t kvVal = 0;
        kvAccess->WriteUInt16("DM100", 123);
        kvAccess->ReadUInt16("DM100", kvVal);
        std::cout << "KV DM100=" << kvVal << std::endl;
        kvAccess->Disconnect();
    }

    // ───────────────────── Melsec MC (TCP) 示例 ─────────────────────
    mc::MelsecClient& melsecClient = mc::MelsecClient::getInstance();
    IRegAccess* melsecAccess = &melsecClient;

    ConnectOptions mcOpts;
    mcOpts.type  = ConnectType::Tcp;
    mcOpts.host  = "127.0.0.1";
    mcOpts.port  = 5000;        // 三菱 MC 协议端口，根据实际设备修改

    if (!melsecAccess->Connect(mcOpts)) {
        std::cerr << "Melsec connect failed: " << melsecAccess->GetLastErrorMessage() << std::endl;
    } else {
        std::uint32_t dVal = 0;
        melsecAccess->WriteUInt32("D200", 456u);
        melsecAccess->ReadUInt32("D200", dVal);
        std::cout << "Melsec D200=" << dVal << std::endl;
        melsecAccess->Disconnect();
    }

    // ───────────────────── Modbus 示例（TCP/RTU 二选一） ─────────────────────
    modbus::ModbusMaster modbusMaster;
    IRegAccess* modbusAccess = &modbusMaster;

    // Modbus TCP
    ConnectOptions mbTcpOpts;
    mbTcpOpts.type    = ConnectType::Tcp;
    mbTcpOpts.host    = "127.0.0.1";
    mbTcpOpts.port    = 502;    // 标准 Modbus TCP 端口
    mbTcpOpts.slaveId = 1;

    if (!modbusAccess->Connect(mbTcpOpts)) {
        std::cerr << "Modbus TCP connect failed: " << modbusAccess->GetLastErrorMessage() << std::endl;
    } else {
        float fVal = 0.0f;
        modbusAccess->WriteFloat("300", 1.23f);
        modbusAccess->ReadFloat("300", fVal);
        std::cout << "Modbus TCP addr 300=" << fVal << std::endl;
        modbusAccess->Disconnect();
    }

    // Modbus RTU 串口（如不需要可注释掉）
    modbus::ModbusMaster modbusRtu;
    IRegAccess* modbusRtuAccess = &modbusRtu;

    ConnectOptions mbRtuOpts;
    mbRtuOpts.type           = ConnectType::SerialRtu;
    mbRtuOpts.serialDevice   = "COM3";  // Windows 串口名示例，根据实际修改
    mbRtuOpts.serialBaud     = 9600;
    mbRtuOpts.serialParity   = 'N';
    mbRtuOpts.serialDataBits = 8;
    mbRtuOpts.serialStopBits = 1;
    mbRtuOpts.slaveId        = 1;

    if (!modbusRtuAccess->Connect(mbRtuOpts)) {
        std::cerr << "Modbus RTU connect failed: " << modbusRtuAccess->GetLastErrorMessage() << std::endl;
    } else {
        std::vector<std::uint16_t> regsWrite {1, 2, 3};
        std::vector<std::uint16_t> regsRead;
        modbusRtuAccess->WriteRegisters("400", regsWrite);
        modbusRtuAccess->ReadRegisters("400", 3, regsRead);
        std::cout << "Modbus RTU addr 400 size=" << regsRead.size() << std::endl;
        modbusRtuAccess->Disconnect();
    }

    std::cout << "Example finished." << std::endl;
    return 0;
}

