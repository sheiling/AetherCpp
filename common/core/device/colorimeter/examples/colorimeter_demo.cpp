#include "../konicaminolta/konica_colorimeter.h"

#define TEST_KONICA_MINOLTA               1

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
    connOpts["devNO"] = 0;
    connOpts["ip"] = "169.254.64.147";
    connOpts["port"] = 2001;
    j["connOpts"] = connOpts;


#ifdef TEST_KONICA_MINOLTA
    j["type"] = common::device::ColorimeterDevType::KONICA_MINOLTA;
    common::device::KonicaColorimeter* colorimeter = new common::device::KonicaColorimeter();
#endif


    fprintf(stdout, "Demo start ... \n");
    if (!colorimeter) {
        fprintf(stdout, "New Colorimeter failed.\n");
        getchar(); 
        return -1;
    }
    fprintf(stdout, "New Colorimeter success.\n");
    colorimeter->reflectFromJson(j);

    // 初始化
    bool bret = colorimeter->init();
    if (!bret) {
        fprintf(stdout, "Colorimeter Init failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Colorimeter Init success.\n");

    bret = colorimeter->connect();
    if (!bret) {
        fprintf(stdout, "Colorimeter connect failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Colorimeter connect success.\n");

    getchar();

    double cx, cy, lum;
    bret = colorimeter->measureData(cx, cy, lum);
    if (!bret) {
        fprintf(stdout, "Colorimeter measureData failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Colorimeter measureData success.\n");

    getchar();

    return 0;
}
