#include "../haikang/hk_scanner.h"
#include "../cino/cino_scanner.h"

//#define SCANNER_CINO            1
#define SCANNER_HK               1

static void scannerRspMsg(const std::string &rspData) {
    fprintf(stdout, "rspData=%s\n", rspData.c_str());
}

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


#ifdef SCANNER_HK
    j["type"] = common::device::ScannerDevType::SSDN_HAIKANG;
    common::device::HKScanner* scanner = new common::device::HKScanner();
#endif
#ifdef SCANNER_CINO
    j["type"] = common::device::ScannerDevType::SSDN_CINO;
    common::device::CINOScanner* scanner = new common::device::CINOScanner();
#endif

    fprintf(stdout, "Demo start ... \n");
    if (!scanner) {
        fprintf(stdout, "New Scanner failed.\n");
        getchar(); 
        return -1;
    }
    fprintf(stdout, "New Scanner success.\n");
    scanner->reflectFromJson(j);

    // 1 建立与扫码器的连接
    bool bret = scanner->Init();
    if (!bret) {
        fprintf(stdout, "Scanner Init failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Scanner Init success.\n");

    // 2 注册扫码响应回调函数
    scanner->RegisterRspCB(scannerRspMsg);

    getchar();

    // 3 发起扫码请求
    bret = scanner->Request("start");
    if (!bret) {
        fprintf(stdout, "Scanner Request failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "Scanner Request success.\n");

    getchar();

    return 0;
}
