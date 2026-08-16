#ifndef _GALAXY_CAMERA_HH
#define _GALAXY_CAMERA_HH

#include "../camera_base.h"
#include "galaxy_camera_global.h"
#include <common/3rdparty/device/camera/galaxy/include/GalaxyIncludes.h>
#include <stdint.h>
#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace common
{
namespace device
{

/**
 * @brief 大恒 Galaxy SDK 相机驱动
 *
 * 采图接口自管触发（见 CameraBase）；Galaxy 通过 TriggerMode / TriggerSource 同步 SDK。
 *
 * ## 并发模型
 * - mtx_：Open/Close/采图 API/setCameraFeatureImpl；SyncGrab 等帧时短暂释放，以便掉线回调 cameraOffline 加锁。
 * - CaptureEventHandler::mtx_ + cond_：SDK 帧回调与 SyncGrab 同步；asyncFlag_/syncCB_/syncCBed_ 为 atomic。
 * - syncGrabInProgress_：阻止 StopGrab 打断 SyncGrab；Close/stopGrabImpl(true) 可强制停流并 notify。
 */
class GALAXY_CAMERA_EXPORT GalaxyCamera : public CameraBase {
    friend class DeviceOfflineEventHandler;
    friend class CaptureEventHandler;
    using Base_ = CameraBase;
public:
    GalaxyCamera();
    ~GalaxyCamera();

    // --- 设备生命周期与状态 ---
    ErrorPtr Open() override;
    ErrorPtr Close() override;
    bool IsConnected() override;
    ErrorPtr ReConnect() override;

    // --- 回调注册 ---
    void RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB) override;
    void RegisterOfflineCB(CameraOfflineFuncCB cameraOfflineFuncCB) override {
        cameraOfflineFuncCB_ = cameraOfflineFuncCB;
    }

    // --- 采图 ---
    ErrorPtr SyncGrab(int &height, int &width, unsigned char* &dataBuffer, int &dataLen) override;
    ErrorPtr AsyncGrab() override;
    ErrorPtr TriggerCapture(
        CameraTriggerSourceType source = CameraTriggerSourceType::SOFTWARE_TRIGGER) override;
    ErrorPtr StopGrab() override;

    // --- Feature / 配置 ---
    ErrorPtr SetAutoWhiteBalance(bool enable);
    void reflectFromJson(const json& j) override;
    void reflectToJson(json& j) const override;

    void cameraOffline();

protected:
    void refreshCameraInfo() override;
    ErrorPtr setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg) override;
    ErrorPtr getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg) override;
    ErrorPtr onTriggerSwitchChanged() override;
    ErrorPtr onTriggerSourceChanged() override;

private:
    ErrorPtr init();
    void uninit();
    ErrorPtr initDevicePointer();
    void initCamera();
    void applyTriggerSourceToDevice();
    ErrorPtr resetCamera(GX_RESET_DEVICE_MODE resetMode = GX_MANUFACTURER_SPECIFIC_RECONNECT);
    void initImageParams();
    bool checkCamera();
    bool isSettableFeature(std::string feature);

    ErrorPtr OpenBySN(const std::string& sn);
    ErrorPtr OpenByIP(const std::string& ip, uint16_t port);
    /** 停止拉流；调用方已持有 mtx_。forceStop 为 true 时允许打断 SyncGrab（Close 用） */
    ErrorPtr stopGrabImpl(bool forceStop = false);

private:
    CGXDevicePointer devicePtr_;
    CGXStreamPointer streamPtr_;
    CGXFeatureControlPointer featureControlPtr_;
    CameraOfflineFuncCB cameraOfflineFuncCB_ = nullptr;
    DeviceOfflineEventHandler *deviceOfflineEventHandler_ = nullptr;
    CaptureEventHandler *captureEventHandler_ = nullptr;
    GX_DEVICE_OFFLINE_CALLBACK_HANDLE deviceOfflineCallbackHandle_ = nullptr;
    CImageProcessConfigPointer m_cameraProcessConfig;
    GxIAPICPP::gxstring mac_;
    bool isGrabbing_ = false;
    /** SyncGrab 进行中；StopGrab 拒绝，Close 可 forceStop */
    std::atomic<bool> syncGrabInProgress_{false};
    std::mutex mtx_;
    static bool isInit_;
    static std::atomic<int> refCnt_;

    // 参数  
    bool m_autoCalibrationBadPoint = false;
    double m_gammaValue = 0.1;                  //gamma值
    int m_contrastValue = 20;                   //对比度值 
    int m_saturationValue = 20;                 //饱和度值
    int m_lightnessValue = 20;                  //亮度值
    double m_sharpenValue = 20;                 //锐化值
    bool m_enableGamma;                         //Gamma使能开关
    bool m_enableSharpness;                     //锐化使能开关
    bool m_enableStaturation;                   //饱和度使能开关
    bool m_enableCheckDenoise;                  //降噪使能开关
    std::string m_pixelFormat = "BayerRG8";     //像素格式
    bool m_reverseX = false;
    bool m_reverseY = false;
    bool m_extraParamEnabled = false;           //初始化额外参数
};

} // namespace device
} // namespace common

#endif
