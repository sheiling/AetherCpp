#include "../device_manager.h"
#include <stdio.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <common/core/device/deviceManager/device_manager.h>

namespace fs = std::filesystem;

#define LAMPHOUSE_DEBUG         1
#define SCANNER_DEBUG           1
#define RANGEFINDER_DEBUG       1
#define PRESSUREPICKUP_DEBUG    1
#define SOFTWARE_SCREEN_DEBUG   1
#define CAMERA_DEBUG            1


#define LAMPHOUSE_DEVICE_KEY            "LDN_HDPS_2C"
#define SCANNER_DEVICE_KEY              "SSDN_HAIKANG"
#define RANGEFINDER_DEVICE_KEY          "RangefinderByTCP"
#define PRESSUREPICKUP_DEVICE_KEY       "PressurePickupByTCP"
#define SOFTWARE_SCREEN_DEVICE_KEY      "SWSDN_WIN32"
#define CAMERA_DEVICE_KEY               "Toup_Camera"


static void ScanSNRsp(const std::string &rspData) {
    fprintf(stdout, "SN=%s\n", rspData.c_str());
}

static void cameraOffline(std::string msg)
{
    printf("%s\n", msg.c_str());
}

static void cameraImageGrab(int height, int width, unsigned char *dataBuffer, int dataLen,
    common::imgproc::ImageFormat imageFormat)
{
    printf("callback dataLen = %d\n", dataLen);
    printf("callback height = %d\n", height);
    printf("callback width = %d\n", width);
    (void)dataBuffer;
    (void)imageFormat;
}


int main() {

#if _WIN32
    setlocale(LC_ALL, ".utf-8");    //设置标准库调用系统API时的默认编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);    //设置控制台输出编码为UTF-8
#endif

    std::shared_ptr<common::device::DeviceManager> device = std::make_shared<common::device::DeviceManager>();

    // 加载示例配置（用于演示编译通过；运行时如果找不到文件则跳过反序列化）
    try {
        json jCfg;
        auto configPath = fs::path(__FILE__).parent_path() / "device_config.json";
        std::ifstream ifs(configPath.string());
        if (ifs) {
            ifs >> jCfg;
            if (jCfg.contains("DeviceManager")) {
                device->reflectFromJson(jCfg["DeviceManager"]);
            }
        }
    } catch (...) {
        // ignore (compile-time demo)
    }

    device->Init();
    auto deviceManager = device;
    fprintf(stdout, "Demo start ... \n");

    bool bret = true;
    int ret = 0;
    int address = 0;
    uint16_t w_value = 99;
    uint16_t r_value = 0;

#ifdef LAMPHOUSE_DEBUG
    //////////////////////// 2 光源设备操作 //////////////////////////////////////
    // 2.0 从光源设备连接池获取光源设备操作句柄
    auto lamphouse = deviceManager->LamphouseHandler(LAMPHOUSE_DEVICE_KEY);
    if (!lamphouse) {
        fprintf(stdout, "Lamphouse device is invalid, key=%s\n", LAMPHOUSE_DEVICE_KEY);
        getchar();
        return -1;
    }
    // 2.1 打开当前光源的0号通道（API 参数为 uint8_t 通道号）
    const uint8_t chcNO = 0;
    bret = lamphouse->OpenChc(chcNO);
    if (!bret) {
        fprintf(stdout, "Lamphouse OpenChc failed, key=%s, chcNO=%u\n", LAMPHOUSE_DEVICE_KEY, chcNO);
        getchar();
        return -1;
    }
    // 2.2 设置当前光源的0号通道的亮度为100
    bret = lamphouse->SetBrightness(chcNO, static_cast<uint8_t>(100));
    if (!bret) {
        fprintf(stdout, "Lamphouse SetBrightness failed, key=%s, chcNO=%u\n", LAMPHOUSE_DEVICE_KEY, chcNO);
        getchar();
        return -1;
    }
    // 2.3 关闭当前光源的0号通道
    bret = lamphouse->CloseChc(chcNO);
    if (!bret) {
        fprintf(stdout, "Lamphouse CloseChc failed, key=%s, chcNO=%u\n", LAMPHOUSE_DEVICE_KEY, chcNO);
        getchar();
        return -1;
    }
    // 2.4 打开当前光源的所有通道
    bret = lamphouse->OpenAllChc();
    if (!bret) {
        fprintf(stdout, "Lamphouse OpenAllChc failed, key=%s\n", LAMPHOUSE_DEVICE_KEY);
        getchar();
        return -1;
    }
    // 2.5 关闭当前光源的所有通道
    bret = lamphouse->CloseAllChc();
    if (!bret) {
        fprintf(stdout, "Lamphouse CloseAllChc failed, key=%s\n", LAMPHOUSE_DEVICE_KEY);
        getchar();
        return -1;
    }
#endif

#ifdef SCANNER_DEBUG
    //////////////////////// 3 扫码器设备操作 //////////////////////////////////////
    // 3.0 从扫码器设备连接池获取扫码器设备操作句柄
    auto scanner = deviceManager->ScannerHandler(SCANNER_DEVICE_KEY);
    if (!scanner) {
        fprintf(stdout, "Scanner device is invalid, key=%s\n", SCANNER_DEVICE_KEY);
        getchar();
        return -1;
    }
    if (!scanner->Init()){
        fprintf(stdout, "Scanner device Init failed, key=%s\n", SCANNER_DEVICE_KEY);
        getchar();
        return -1;
    }

    // 3.1 注册扫码响应回调函数
    scanner->RegisterRspCB(ScanSNRsp);
    
    fprintf(stdout, "Press any to request SN:\n");
    getchar();


    // 3.2 请求扫码
    bret = scanner->Request("start");
    if (!bret) {
        fprintf(stdout, "Scanner Request failed, key=%s\n", SCANNER_DEVICE_KEY);
        getchar();
        return -1;
    }
#endif

#ifdef RANGEFINDER_DEBUG
    //////////////////////// 4 测距仪设备操作 //////////////////////////////////////
    // 4.0 从测距仪设备连接池获取测距仪设备操作句柄
    auto rangefinder = deviceManager->RangeFinderHandler(RANGEFINDER_DEVICE_KEY);
    if (!rangefinder) {
        fprintf(stdout, "Rangefinder device is invalid, key=%s\n", RANGEFINDER_DEVICE_KEY);
        getchar();
        return -1;
    }
    address = 0;
    w_value = 99; 
    // 4.1 往0号寄存器写入一个字的数据
    ret = rangefinder->WriteRegister(address, w_value);
    if (ret != 0) {
        fprintf(stdout, "Rangefinder write failed, address=%d, w_value=%u\n", address, w_value);
        getchar();
        return -1;
    }
    // 4.2 从0号寄存器读取一个字的数据
    r_value = 0;
    ret = rangefinder->ReadRegister(address, r_value);
    if (ret != 0) {
        fprintf(stdout, "Rangefinder read failed, address=%d\n", address);
        getchar();
        return -1;
    }
#endif

#ifdef PRESSUREPICKUP_DEBUG
    //////////////////////// 5 压力传感器设备操作 //////////////////////////////////////
    // 5.0 从压力传感器设备连接池获取压力传感器设备操作句柄
    auto pressurePickup = deviceManager->PressurePickupHandler(PRESSUREPICKUP_DEVICE_KEY);
    if (!pressurePickup) {
        fprintf(stdout, "PressurePickup device is invalid, key=%s\n", PRESSUREPICKUP_DEVICE_KEY);
        getchar();
        return -1;
    }
    address = 0;
    w_value = 99; 
    // 4.1 往0号寄存器写入一个字的数据
    ret = pressurePickup->WriteRegister(address, w_value);
    if (ret != 0) {
        fprintf(stdout, "PressurePickup write failed, address=%d, w_value=%u\n", address, w_value);
        getchar();
        return -1;
    }
    // 4.2 从0号寄存器读取一个字的数据
    r_value = 0;
    ret = pressurePickup->ReadRegister(address, r_value);
    if (ret != 0) {
        fprintf(stdout, "PressurePickup read failed, address=%d\n", address);
        getchar();
        return -1;
    }
#endif

#ifdef SOFTWARE_SCREEN_DEBUG
    //////////////////////// 7 软件打屏设备操作 //////////////////////////////////////
    // 7.0 从软件打屏设备连接池获取设软件打屏设备操作句柄
    auto softwareScreen = deviceManager->SoftScreenHandler(SOFTWARE_SCREEN_DEVICE_KEY);
    if (!softwareScreen) {
        fprintf(stdout, "SoftwareScreen device is invalid, key=%s\n", SOFTWARE_SCREEN_DEVICE_KEY);
        getchar();
        return -1;
    }

    // 7.1 构造软件打屏消息结构
    common::device::SoftScreenParam softScreenParams;
    softScreenParams.width = 500;
    softScreenParams.height = 500;
    softScreenParams.brightness = 255;
    softScreenParams.color = common::device::SoftScreenColor::COLOR_RED;
    softScreenParams.mode = common::device::SoftScreenMode::FULL_MODE;
    softScreenParams.interval = 1;

    // 7.2 发起软件打屏请求
    softwareScreen->Display(softScreenParams);
    
#endif

#ifdef CAMERA_DEBUG
    //////////////////////// 8 相机设备操作 //////////////////////////////////////
    // 8.0 从硬件相机设备连接池获取相机设备操作句柄
    auto camera = deviceManager->CameraHandler(CAMERA_DEVICE_KEY);
    if (!camera) {
        fprintf(stdout, "camera device is invalid, key=%s\n", CAMERA_DEVICE_KEY);
        getchar();
        return -1;
    }

    // 8.1 注册回调函数
    camera->RegisterOfflineCB(cameraOffline);
    camera->RegisterGrabCB(cameraImageGrab);


    // 8.2 获取相机信息
    common::device::CameraInfo cameraInfo = camera->GetCameraInfo();
    printf("cameraModelName = %s\n", cameraInfo.cameraModelName.c_str());
    printf("cameraID = %s\n", cameraInfo.cameraID.c_str());
    printf("pixelFormat = %s\n", cameraInfo.pixelFormat.c_str());
    printf("pixelFormatType = %s\n", common::device::pixelFormatTypeToString(cameraInfo.pixelFormatType));
    printf("imageWidth = %d\n", cameraInfo.imageWidth);
    printf("imageHeight = %d\n", cameraInfo.imageHeight);
    printf("bitDepth = %d\n", cameraInfo.bitDepth);
    printf("isSupportColor = %d\n", cameraInfo.isSupportColor);

    // 8.3 设置曝光时间，单位（us）
    auto expErr = camera->SetExposureTime(300000);
    if (expErr) {
        fprintf(stdout, "camera SetExposureTime failed.\n");
    }
    
    // 8.4 图像采集
    #if 1
    // 单帧采集
    unsigned char* dataBuffer = nullptr;
    int dataLen = 0;
    int height = 0;
    int width = 0;
    auto grabErr = camera->SyncGrab(height, width, dataBuffer, dataLen);
    if (grabErr) {
        fprintf(stdout, "camera SyncGrab failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "camera SyncGrab success.\n");
    printf("dataLen = %d\n", dataLen);
    printf("height = %d\n", height);
    printf("width = %d\n", width);
    #else
    // 回调采集
    auto asyncErr = camera->AsyncGrab();
    if (asyncErr) {
        fprintf(stdout, "camera AsyncGrab failed.\n");
        getchar();
        return -1;
    }

    getchar();

    auto stopErr = camera->StopGrab();
    if (stopErr) {
        fprintf(stdout, "camera StopGrab failed.\n");
        getchar();
        return -1;
    }
    #endif

    fprintf(stdout, "Demo end ... \n");

    getchar();

    camera->Close();
    
#endif

    getchar();

    return 0;
}


