#include "../mv_camera.h"

#include <common/utility/string_tool/time_format.hpp>

#include <stdio.h>
#include <iostream>

uint64_t beginTm;

void offline(std::string msg) {
    printf("%s\n", msg.c_str());
}

void callback(int height, int width, unsigned char* dataBuffer, int dataLen,
    common::imgproc::ImageFormat imageFormat) {
    uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
    std::cout << "callback currentTm = " << currentTm << std::endl;
    std::cout << "currentTm - beginTm = " << currentTm - beginTm << std::endl;
    printf("callback dataLen = %d\n", dataLen);
    printf("callback height = %d\n", height);
    printf("callback width = %d\n", width);

    // 示例：业务侧可以在这里做图像处理（如 memcpy 保存 / 推理 / 写文件等）
    (void)dataBuffer;
    (void)imageFormat;
}

int main() {
    json j;
    json connOpts;
    connOpts["devNO"] = 0;
    connOpts["ip"] = "169.254.139.92";
    connOpts["port"] = 5000;
    connOpts["SN"] = "DA5089609";
    connOpts["index"] = 0;
    connOpts["connType"] = "CONN_BY_SN";

    // mv_camera 的设备类型目前不参与 DeviceManager 工厂（demo 只需能解析 JSON 并调用 reflectFromJson）
    j["type"] = common::device::CameraDevType::CDN_UNKNOWN;
    j["connOpts"] = connOpts;

    fprintf(stdout, "Demo start ... \n");
    common::device::MvCamera* camera = new common::device::MvCamera();
    if (!camera) {
        fprintf(stdout, "New Camera failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "New Camera success.\n");

    camera->reflectFromJson(j);

    camera->RegisterOfflineCB(offline);
    camera->RegisterGrabCB(callback);

    auto err = camera->Open();
    if (err) {
        fprintf(stdout, "camera open failed.\n");
        getchar();
        return -1;
    }
    fprintf(stdout, "camera open success.\n");

    common::device::CameraInfo cameraInfo = camera->GetCameraInfo();
    printf("cameraModelName = %s\n", cameraInfo.cameraModelName.c_str());
    printf("cameraID = %s\n", cameraInfo.cameraID.c_str());
    printf("pixelFormat = %s\n", cameraInfo.pixelFormat.c_str());
    printf("pixelFormatType = %s\n", common::device::pixelFormatTypeToString(cameraInfo.pixelFormatType));
    printf("imageWidth = %d\n", cameraInfo.imageWidth);
    printf("imageHeight = %d\n", cameraInfo.imageHeight);
    printf("bitDepth = %d\n", cameraInfo.bitDepth);
    printf("isSupportColor = %d\n", cameraInfo.isSupportColor);

    err = camera->SetExposureTime(20000);
    if (err) {
        fprintf(stdout, "camera SetExposureTime failed.\n");
        getchar();
        return -1;
    }

#if 1
    for (int i = 0; i < 5; i++) {
        std::string imageData;
        beginTm = common::utility::TimeFormat::GetCurrentMs();

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

        imageData.append(reinterpret_cast<char*>(dataBuffer), dataLen);

        fprintf(stdout, "camera SyncGrab success.\n");
        printf("dataLen = %d\n", dataLen);
        printf("height = %d\n", height);
        printf("width = %d\n", width);
        printf("length = %d\n", static_cast<int>(imageData.length()));

        uint64_t currentTm = common::utility::TimeFormat::GetCurrentMs();
        std::cout << "currentTm = " << currentTm << std::endl;
        std::cout << "currentTm - beginTm = " << currentTm - beginTm << std::endl;
    }
#else
    err = camera->AsyncGrab();
    if (err) {
        fprintf(stdout, "camera AsyncGrab failed.\n");
        getchar();
        return -1;
    }

    getchar();

    err = camera->StopGrab();
    if (err) {
        fprintf(stdout, "camera StopGrab failed.\n");
        getchar();
        return -1;
    }
#endif

    fprintf(stdout, "Demo end ... \n");

    getchar();
    camera->Close();
    delete camera;
    return 0;
}

