#include "../galaxy_camera.h"
#include <common/utility/string_tool/time_format.hpp>
#include <stdio.h>
#include <iostream>

uint64_t beginTm;

void offline(std::string msg)
{
    printf("%s\n", msg.c_str());
}

void callback(int height, int width, unsigned char *dataBuffer, int dataLen,
    common::imgproc::ImageFormat imageFormat)
{
    uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
    std::cout << "callback currentTm = " << currentTm << std::endl;
    std::cout << "currentTm - beginTm = " << currentTm -  beginTm << std::endl;
    printf("callback dataLen = %d\n", dataLen);
    printf("callback height = %d\n", height);
    printf("callback width = %d\n", width);
    (void)dataBuffer;
    (void)imageFormat;
}

int main() {
    json j;
    json connOpts;
    connOpts["devNO"] = 0;
    connOpts["ip"] = "192.168.100.11";
    connOpts["port"] = 5000;
    connOpts["SN"] = "LS0210110778";
    connOpts["index"] = 0;
    connOpts["connType"] = "CONN_BY_SN";

    j["type"] = common::device::CameraDevType::CDN_GALAXY;
    j["connOpts"] = connOpts;

    fprintf(stdout, "Demo start ... \n");
    common::device::GalaxyCamera* camera = new common::device::GalaxyCamera();
    if (!camera) {
        fprintf(stdout, "New CameraBase failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New CameraBase success.\n");

    camera->reflectFromJson(j);

    // 注册回调函数
    camera->RegisterOfflineCB(offline);
    camera->RegisterGrabCB(callback);

    // 打开相机
    auto err = camera->Open();
    if (err) {
        fprintf(stdout, "camera open failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "camera open success.\n");

    
    // 获取相机信息
    common::device::CameraInfo cameraInfo = camera->GetCameraInfo();
    printf("cameraModelName = %s\n", cameraInfo.cameraModelName.c_str());
    printf("cameraID = %s\n", cameraInfo.cameraID.c_str());
    printf("pixelFormat = %s\n", cameraInfo.pixelFormat.c_str());
    printf("pixelFormatType = %s\n", common::device::pixelFormatTypeToString(cameraInfo.pixelFormatType));
    printf("imageWidth = %d\n", cameraInfo.imageWidth);
    printf("imageHeight = %d\n", cameraInfo.imageHeight);
    printf("bitDepth = %d\n", cameraInfo.bitDepth);
    printf("isSupportColor = %d\n", cameraInfo.isSupportColor);

    // 设置曝光时间，单位（us）
    // camera->SetExposureTime(20000);
    
    #if 1
    for (int i = 0; i < 5; i++)
    {
        beginTm = common::utility::TimeFormat::GetCurrentMs();
        std::cout << "beginTm = " << beginTm << std::endl;
        // 单帧采集
        unsigned char* dataBuffer = nullptr;
        int dataLen = 0;
        int height = 0;
        int width = 0;
        err = camera->SyncGrab(height, width, dataBuffer, dataLen);
        if (err) {
            fprintf(stdout, "camera SyncGrab failed.\n");
            getchar();
            return -1;
        }
        fprintf(stdout, "camera SyncGrab success.\n");
        printf("dataLen = %d\n", dataLen);
        printf("height = %d\n", height);
        printf("width = %d\n", width);
        uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
        std::cout << "currentTm = " << currentTm << std::endl;
        std::cout << "currentTm - beginTm = " << currentTm -  beginTm << std::endl;
    }
    
    
    #else
    // 回调采集
    // bret = camera->AsyncGrab();
    // if (!bret) {
    //     fprintf(stdout, "camera AsyncGrab failed.\n");
    //     getchar();
    //     return -1;
    // }

    // getchar();

    // bret = camera->StopGrab();
    // if (!bret) {
    //     fprintf(stdout, "camera StopGrab failed.\n");
    //     getchar();
    //     return -1;
    // }
    err = camera->TriggerCapture();
    if (err) {
        fprintf(stdout, "camera TriggerCapture failed.\n");
        getchar();
        return -1;
    }
    #endif
    
    
    fprintf(stdout, "Demo end ... \n");
    fprintf(stdout, "Demo end ... \n");

    getchar();

    camera->Close();

    return 0;
}
