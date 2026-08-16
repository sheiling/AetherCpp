#ifndef COMMON_CORE_DEVICE_CAMERA_MV_CAMERA_HH
#define COMMON_CORE_DEVICE_CAMERA_MV_CAMERA_HH

#include "../camera_base.h"
#include "mv_camera_global.h"

#include <device/camera/mvs/Includes/MvCameraControl.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace common
{
namespace device
{

/**
 * @brief 海康 MVS SDK 相机驱动
 *
 * 采图接口自管触发（见 CameraBase）；MVS 通过 TriggerMode / TriggerSource 同步 SDK。
 *
 * ## 并发模型
 * - mtx_：设备与采图 API；isGrabbing_/isOpen_ 为 atomic。
 * - condmtx_ + cond_：SyncGrab 等帧；imageBufSize_ 由 ImageGrabProcess 写入、SyncGrab 读取（同 condmtx_ 保护唤醒）。
 * - SyncGrab 成功后 isStartGrabbing_ 可能仍为 true（SDK 流保持开启，回调由 isGrabbing_ 过滤）。
 */
class MV_CAMERA_EXPORT MvCamera : public CameraBase {
    using Base_ = CameraBase;
public:
    MvCamera();
    ~MvCamera() override;

    // --- 设备生命周期与状态 ---
    ErrorPtr Open() override;
    ErrorPtr Close() override;
    bool IsConnected() override;
    ErrorPtr ReConnect() override;

    // --- 回调注册 ---
    void RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB) override {
        imageGrabFuncCB_ = std::move(imageGrabFuncCB);
    }
    void RegisterOnlineCB(CameraOnlineFuncCB cameraOnlineFuncCB) override {
        cameraOnlineFuncCB_ = std::move(cameraOnlineFuncCB);
    }
    void RegisterOfflineCB(CameraOfflineFuncCB cameraOfflineFuncCB) override {
        cameraOfflineFuncCB_ = std::move(cameraOfflineFuncCB);
    }

    // --- 采图 ---
    ErrorPtr SyncGrab(int &height, int &width, unsigned char* &dataBuffer, int &dataLen) override;
    ErrorPtr AsyncGrab() override;
    ErrorPtr TriggerCapture(
        CameraTriggerSourceType source = CameraTriggerSourceType::SOFTWARE_TRIGGER) override;
    ErrorPtr StopGrab() override;

    // --- Feature / 配置 ---
    void reflectFromJson(const json& j) override;
    void reflectToJson(json& j) const override;

    // SDK 回调入口（须在 public，供 C 回调调用）
    void offlineProcess();
    void ImageGrabProcess(int height, int width, unsigned char* dataBuffer, int dataLen,
                          common::imgproc::ImageFormat imageFormat);

protected:
    void refreshCameraInfo() override;
    ErrorPtr setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg) override;
    ErrorPtr getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg) override;
    ErrorPtr onTriggerSwitchChanged() override;
    ErrorPtr onTriggerSourceChanged() override;

private:
    bool _open(MV_CC_DEVICE_INFO* pstMVDevInfo);
    ErrorPtr _close();
    /** 停止拉流；调用方已持有 mtx_。forceStop 为 true 时允许打断 SyncGrab（Close 用） */
    ErrorPtr stopGrabImpl(bool forceStop = false);
    ErrorPtr initCamera();

    std::string getSerialNumber(MV_CC_DEVICE_INFO* pstMVDevInfo);
    bool isColor(MvGvspPixelType enType) const;

private:
    CameraOnlineFuncCB cameraOnlineFuncCB_ = nullptr;
    CameraOfflineFuncCB cameraOfflineFuncCB_ = nullptr;
    ImageGrabFuncCB imageGrabFuncCB_ = nullptr;

    void* cam_ = nullptr; // 设备句柄
    std::atomic_bool isOpen_{false};

    std::atomic_bool isGrabbing_{false};
    bool isStartGrabbing_ = false;

    std::mutex mtx_;
    std::mutex condmtx_;
    std::condition_variable cond_;
    std::atomic<bool> asyncFlag_{false};
    /** SyncGrab 进行中；StopGrab 拒绝，Close 可 forceStop */
    std::atomic<bool> syncGrabInProgress_{false};

    // sync grab buffer
    unsigned char* imageBuffer_ = nullptr;
    uint64_t imageBufSize_ = 0;
    int imgWidth_ = 0;
    int imgHeight_ = 0;

    uint32_t imageNodeNum_ = 5;
    std::vector<unsigned char> asyncCopyBuffer_;

    // configuration from json
    bool reConnectAuto_ = true;
    bool reverseX_ = false;
    bool reverseY_ = false;
    std::string pixelFormat_ = "BayerRG8";
    std::map<std::string, MvGvspPixelType> pixelFormatMap_ = {
        {"Mono8", PixelType_Gvsp_Mono8},
        {"Mono10", PixelType_Gvsp_Mono10},
        {"Mono10Packed", PixelType_Gvsp_Mono10_Packed},
        {"Mono12", PixelType_Gvsp_Mono12},
        {"Mono12Packed", PixelType_Gvsp_Mono12_Packed},
        {"Mono16", PixelType_Gvsp_Mono16},
        {"RGB8Packed", PixelType_Gvsp_RGB8_Packed},
        {"BayerGR8", PixelType_Gvsp_BayerGR8},
        {"BayerRG8", PixelType_Gvsp_BayerRG8},
        {"BayerGB8", PixelType_Gvsp_BayerGB8},
        {"BayerBG8", PixelType_Gvsp_BayerBG8},
        {"BayerGB10", PixelType_Gvsp_BayerGB10},
        {"BayerBG10", PixelType_Gvsp_BayerBG10},
        {"BayerGR10", PixelType_Gvsp_BayerGR10},
        {"BayerRG10", PixelType_Gvsp_BayerRG10},
        {"BayerGB12", PixelType_Gvsp_BayerGB12},
        {"BayerBG12", PixelType_Gvsp_BayerBG12},
        {"BayerGR12", PixelType_Gvsp_BayerGR12},
        {"BayerRG12", PixelType_Gvsp_BayerRG12},
    };

    static bool isInit_;
    static std::atomic<int> refCnt_;
};

} // namespace device
} // namespace common

#endif
