#ifndef _TOUP_CAMERA_HH
#define _TOUP_CAMERA_HH

#include "../camera_base.h"
#include "toup_camera_global.h"
#include <common/3rdparty/device/camera/toupcam/include/toupcam.h>
#include <stdint.h>
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace common
{
namespace device
{

/**
 * @brief 图谱 ToupCam SDK 相机驱动
 *
 * SDK 层固定软件触发；ensureFreeRunMode/ensureTriggerMode 仅维护逻辑 m_triggerSwitch_ 供路由。
 * 实际采图区分：Trigger(1) 单帧 / Trigger(0xffff) 连续 / Trigger(0) 停止。
 *
 * ## 并发模型
 * - mtx_：Open/Close/采图/setCameraFeatureImpl；SyncGrab 等帧时释放 mtx_。
 * - ERROR 事件：Close 投递 detached 线程，避免 SDK 回调线程死锁。
 * - 异步 GrabCB 经 asyncCopyBuffer_ 拷贝，避免下一帧覆盖。
 */
class TOUP_CAMERA_EXPORT ToupCamera : public CameraBase {
    using Base_ = CameraBase;
public:
    ToupCamera() = default;
    ~ToupCamera() override;

    // --- 设备生命周期与状态 ---
    ErrorPtr Open() override;
    ErrorPtr Close() override;
    bool IsConnected() override;
    ErrorPtr ReConnect() override;

    // --- 回调注册 ---
    void RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB) override {
        imageGrabFuncCB_ = imageGrabFuncCB;
    }
    void RegisterOfflineCB(CameraOfflineFuncCB cameraOfflineFuncCB) override {
        cameraOfflineFuncCB_ = cameraOfflineFuncCB;
    }

    // --- 采图 ---
    ErrorPtr SyncGrab(int &height, int &width, unsigned char* &dataBuffer, int &dataLen) override;
    ErrorPtr AsyncGrab() override;
    ErrorPtr TriggerCapture(
        CameraTriggerSourceType source = CameraTriggerSourceType::SOFTWARE_TRIGGER) override;
    ErrorPtr StopGrab() override;

    // SDK 回调入口
    void imageGrabProcess();
    void offlineProcess();

protected:
    void refreshCameraInfo() override;
    ErrorPtr setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg) override;
    ErrorPtr getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg) override;
    int CameraFilterWheelType2Int(CameraFilterWheelType type) const override;

private:
    ErrorPtr initCamera();
    ErrorPtr resetCamera();
    void setCameraInfo();
    ErrorPtr setWheelPosition(int pos);
    ErrorPtr setNDWheelPosition(int pos);
    void pullImage(int &height, int &width, unsigned char* dataBuffer);
    bool writeUART(const unsigned char* data, int len);
    bool readUART(unsigned char* data, int len);

    ErrorPtr OpenByIndex(int index);
    /** 停止拉流；调用方已持有 mtx_。forceStop 为 true 时允许打断 SyncGrab（Close 用） */
    ErrorPtr stopGrabImpl(bool forceStop = false);

private:
    HToupcam hToupcam_ = nullptr;
    CameraOfflineFuncCB cameraOfflineFuncCB_ = nullptr;
    ImageGrabFuncCB imageGrabFuncCB_ = nullptr;
    unsigned char* imageBuffer_ = nullptr;
    uint64_t imageBufSize_ = 0;
    int imgWidth_ = 0;
    int imgHeight_ = 0;
    bool isGrabbing_ = false;
    bool isStart_ = false;
    std::atomic<bool> asyncFlag_{false};
    std::atomic<bool> syncGrabInProgress_{false};
    int cameraIndex_ = 0;
    std::mutex mtx_;
    std::mutex condmtx_;
    std::condition_variable cond_;
    std::vector<unsigned char> asyncCopyBuffer_;
};

} // namespace device
} // namespace common

#endif
