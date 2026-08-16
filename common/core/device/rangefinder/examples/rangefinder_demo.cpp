#include <stdio.h>
#include "../rangefinder_base.h"
#include "../rangefinder_by_tcp/rangefinder_by_tcp.h"
#include "../wonsor_rangefinder/wonsor_rangefinder.h"

int main() {
    json j;
    json serial;
    serial["rspTimeout_"] = 3000;
    serial["com_"] = "COM1";
    serial["baud_"] = 9600;
    serial["dataBit_"] = 8;
    serial["parity_"] = "None";
    serial["stopBit_"] = 1;
    j["serialInfo"] = serial;    
    j["devNO"] = 1;
    
    json config;
    config["devInfo"] = j;
    fprintf(stdout, "Demo start ... \n");
    common::device::WonsorRangefinder* rangefinder = new common::device::WonsorRangefinder();
    if (!rangefinder) {
        fprintf(stdout, "New Rangefinder failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New Rangefinder success.\n");
    rangefinder->reflectFromJson(config);

    if (!rangefinder->openRangeFinder()) {
        fprintf(stdout, "open range finder failed.\n");
        getchar();
        return -1;
    }
    while (true) {
        float value = 0;
        rangefinder->measureDistance(value);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return 0;
}

int main00() {

    json j;
    json connOpts;
    json serial;
    serial["rspTimeout_"] = 3000;
    serial["com_"] = "COM6";
    serial["dataBit_"] = 8;
    serial["parity_"] = "None";
    serial["stopBit_"] = 1;
    serial["baud_"] = 115200;
    serial["slaveID_"] = 1;
    connOpts["serialInfo"] = serial;
    connOpts["protocolType"] = "RTU";
    connOpts["devNO"] = 0;
    connOpts["ip"] = "127.0.0.1";
    connOpts["port"] = 5000;
    j["connOpts"] = connOpts;
    j["type"] = common::device::RangefinderDevType::RDN_UNKNOWN;

    fprintf(stdout, "Demo start ... \n");
    common::device::RangefinderBase* rangefinder = new common::device::RangefinderBase();
    if (!rangefinder) {
        fprintf(stdout, "New Rangefinder failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New Rangefinder success.\n");
    rangefinder->reflectFromJson(j);

    // 1 连接测距仪
    bool bret = rangefinder->Init();
    if (!bret) {
        fprintf(stdout, "Rangefinder Init failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Rangefinder Init success.\n");

    // 2 往0号寄存器写入一个字
    // uint16_t w_value = 99;
    // int ret = rangefinder->WriteRegister(0, w_value);
    // if (ret != 0) {
    //     fprintf(stdout, "Rangefinder WriteRegister failed.\n");
    //     getchar();
    //     return -1;
    // }
    // fprintf(stdout, "Rangefinder WriteRegister success.\n");

    // 3 从0号寄存器读取一个字
    float r_value = 0;
    int ret = rangefinder->ReadRegister(0x00, r_value);
    if (ret != 0) {
        fprintf(stdout, "Rangefinder ReadRegister failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Rangefinder ReadRegister success.\n");

    fprintf(stdout, "r_value=%f.\n", r_value);

    getchar();

    return 0;
}
