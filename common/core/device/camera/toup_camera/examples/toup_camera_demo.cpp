#include "../toup_camera.h"
#include <stdio.h>

void offline(std::string msg)
{
    printf("%s\n", msg.c_str());
}

void callback(int height, int width, unsigned char *dataBuffer, int dataLen,
    common::imgproc::ImageFormat imageFormat)
{
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
    connOpts["ip"] = "127.0.0.1";
    connOpts["port"] = 5000;
    connOpts["SN"] = "00001";
    connOpts["index"] = 0;
    connOpts["connType"] = "CONN_BY_INDEX";

    j["type"] = common::device::CameraDevType::CDN_TOUP;
    j["connOpts"] = connOpts;


    fprintf(stdout, "Demo start ... \n");
    common::device::ToupCamera* camera = new common::device::ToupCamera();
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
    err = camera->SetExposureTime(300000);
    if (err) {
        fprintf(stdout, "camera SetExposureTime failed.\n");
    }
    // 设置滤片，1-无滤片，2-X滤片、3-Y滤片、4-Z滤片
    err = camera->SetCameraFeature(common::device::CameraFeatureType::CAMERA_FILTER_WHEEL, 2);
    if (err) {
        fprintf(stdout, "camera SetCameraFeature CAMERA_FILTER_WHEEL failed.\n");
    }


    #if 1
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
    #else
    // 回调采集
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

    return 0;
}
