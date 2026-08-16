#include <stdio.h>
#include "../pressurepickup_base.h"
#include "../pressurepickup_by_tcp/pressurepickup_by_tcp.h"
#include "../pressurepickup_by_serial/pressurepickup_by_serial.h"

int main() {

    json j;
    json connOpts;
    json serial;
    serial["rspTimeout_"] = 3000;
    serial["com_"] = "COM1";
    serial["dataBit_"] = 8;
    serial["parity_"] = "None";
    serial["stopBit_"] = 1;
    connOpts["serialInfo"] = serial;
    connOpts["protocolType"] = "TCP";
    connOpts["devNO"] = 0;
    connOpts["ip"] = "127.0.0.1";
    connOpts["port"] = 5000;
    j["connOpts"] = connOpts;
    j["type"] = common::device::PressurePickupDevType::PPDN_UNKNOWN;

    fprintf(stdout, "Demo start ... \n");
    common::device::PressurePickupBase* pressurePickup = new common::device::PressurePickupBase();

    fprintf(stdout, "Demo start ... \n");
    if (!pressurePickup) {
        fprintf(stdout, "New PressurePickup failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New PressurePickup success.\n");
    pressurePickup->reflectFromJson(j);

    // 1 连接压力传感器
    bool bret = pressurePickup->Init();
    if (!bret) {
        fprintf(stdout, "PressurePickup Init failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "PressurePickup Init success.\n");

    // 2 往0号寄存器写入一个字
    uint16_t w_value = 99;
    int ret = pressurePickup->WriteRegister(0, w_value);
    if (ret != 0) {
        fprintf(stdout, "PressurePickup WriteRegister failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "PressurePickup WriteRegister success.\n");

    // 3 从0号寄存器读取一个字
    uint16_t r_value = 0;
    ret = pressurePickup->ReadRegister(0, r_value);
    if (ret != 0) {
        fprintf(stdout, "PressurePickup ReadRegister failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "PressurePickup ReadRegister success.\n");

    getchar();

    return 0;
}
