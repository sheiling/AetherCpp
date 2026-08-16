#include <stdio.h>
#include <common/core/device/lamphouse/drivers/cst/cst_dps_lamphouse.h>
#include <common/core/device/lamphouse/drivers/hdps/hdps_2c_lamphouse.h>
#include <common/core/device/lamphouse/drivers/jl_dvc/jl_dvc_lamphouse.h>
#include <common/core/device/lamphouse/drivers/lts/lts_lamphouse.h>

//#define LAMPHOUSE_HDPS_2C               1
//#define LAMPHOUSE_JL_DVC                1
//#define LAMPHOUSE_LTS                   1
#define LAMPHOUSE_CST_DPS               1

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
    connOpts["channelNumber"] = 1;
    connOpts["devNO"] = 0;
    j["connOpts"] = connOpts;

    common::device::LamphouseBase* lamphouse = nullptr;

#ifdef LAMPHOUSE_HDPS_2C
    j["type"] = common::device::LamphouseDevType::LDN_HDPS_2C;
    lamphouse = new common::device::Hdps2cLamphouse();
#endif
#ifdef LAMPHOUSE_JL_DVC
    j["type"] = common::device::LamphouseDevType::LDN_JL_DVC;
    lamphouse = new common::device::JlDvcLamphouse();
#endif
#ifdef LAMPHOUSE_LTS
    j["type"] = common::device::LamphouseDevType::LDN_LTS;
    connOpts["productModel"] = common::device::LtsProductModel::LTS_3DPA24;
    j["connOpts"] = connOpts;
    lamphouse = new common::device::LtsLamphouse();
#endif
#ifdef LAMPHOUSE_CST_DPS
    j["type"] = common::device::LamphouseDevType::LDN_CST_DPS;
    lamphouse = new common::device::CstDpsLamphouse();
#endif

    fprintf(stdout, "Demo start ... \n");

    if (!lamphouse) {
        fprintf(stdout, "New Lamphouse failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New Lamphouse success.\n");
    lamphouse->reflectFromJson(j);

    int ret = lamphouse->Init();
    if (0 != ret) {
        fprintf(stdout, "Serial connect failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Serial connect success.\n");

    if (lamphouse->OpenChc(3)) {
        fprintf(stdout, "OpenChc failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "OpenChc success.\n");

    if (lamphouse->CloseChc(3)) {
        fprintf(stdout, "CloseChc failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "CloseChc success.\n");

    if (lamphouse->SetBrightness(3, 100)) {
        fprintf(stdout, "SetBrightness failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "SetBrightness success.\n");

    fprintf(stdout, "\n");

    getchar();

    return 0;
}
