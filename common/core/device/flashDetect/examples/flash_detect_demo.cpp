#include "../flashDetect_by_serial/flash_detect_by_serial.h"

#include <cstdio>
#include <cstring>

namespace
{
int RunWithExplicitParameters()
{
    common::device::FlashDetectBySerial detector;
    common::device::ISerialPort::Parameters serialParams;
    serialParams.baudRate = 115200;
    serialParams.stopBits = common::device::ISerialPort::StopBits::One;
    serialParams.dataBits = common::device::ISerialPort::DataBits::Data8;
    serialParams.parity = common::device::ISerialPort::Parity::NoParity;
    serialParams.flowControl = common::device::ISerialPort::FlowControl::NoFlowControl;

    if (detector.OpenComm("COM1", serialParams) != 0) {
        std::fprintf(stderr, "OpenComm failed, please check serial settings.\n");
        return -1;
    }

    // 调用带参数接口：可按目标板卡地址执行检测。
    const int ret = detector.CheckFlash(1, 1, 1);
    detector.CloseComm();
    return ret;
}

int RunWithJsonConfiguration()
{
    common::device::FlashDetectBySerial detector;

    json cfg;
    cfg["connOpts"] = {
        {"portName", "COM1"},
        {"baudRate", 115200},
        {"stopBits", 0},
        {"dataBits", 8},
        {"parity", 0},
        {"flowControl", 0}
    };
    cfg["checkOpts"] = {
        {"destAddr", 1},
        {"portAddr", 1},
        {"boardAddr", 1},
        {"maxRetries", 100},
        {"retryIntervalMs", 200},
        {"expectedStatus", 0x00000005}
    };
    detector.reflectFromJson(cfg);

    if (detector.OpenComm() != 0) {
        std::fprintf(stderr, "OpenComm failed, please check COM port.\n");
        return -1;
    }

    // 调用无参接口：使用 reflectFromJson 注入的 checkOpts。
    const int ret = detector.CheckFlash();
    detector.CloseComm();
    return ret;
}
} // namespace

int main(int argc, char* argv[])
{
    const bool useJsonConfig = (argc > 1 && std::strcmp(argv[1], "--json") == 0);
    const int ret = useJsonConfig ? RunWithJsonConfiguration() : RunWithExplicitParameters();
    std::fprintf(stdout, "flash_detect_demo result=%d\n", ret);
    return ret;
}
