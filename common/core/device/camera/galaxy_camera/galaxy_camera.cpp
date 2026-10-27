#include "galaxy_camera.h"
#include <iostream>
#include <thread>
#include <vector>

#include <common/utility/errors/include/error.h>
#include "common/core/device/camera/camera_error_codes.h"

namespace {
inline std::string GalaxySdkDetail(CGalaxyException& e) {
    // CGalaxyException::GetErrorCode() 可能不是 const 方法
    const int sdkCode = e.GetErrorCode();
    const char* sdkMsg = e.what();
    std::ostringstream oss;
    oss << "sdk_code=" << sdkCode;
    if (sdkMsg && sdkMsg[0] != '\0') {
        oss << ", sdk_msg=" << sdkMsg;
    }
    return oss.str();
}

inline common::imgproc::ImageFormat ToImageFormat(GX_PIXEL_FORMAT_ENTRY pixelFormat) {
    using common::imgproc::ImageFormat;

    switch (pixelFormat) {
    case GX_PIXEL_FORMAT_MONO8: return ImageFormat::MONO8;
    case GX_PIXEL_FORMAT_MONO10: return ImageFormat::MONO10;
    case GX_PIXEL_FORMAT_MONO12: return ImageFormat::MONO12;
    case GX_PIXEL_FORMAT_MONO14: return ImageFormat::MONO14;
    case GX_PIXEL_FORMAT_MONO16: return ImageFormat::MONO16;
    case GX_PIXEL_FORMAT_BAYER_GR8: return ImageFormat::BAYER_GR8;
    case GX_PIXEL_FORMAT_BAYER_RG8: return ImageFormat::BAYER_RG8;
    case GX_PIXEL_FORMAT_BAYER_GB8: return ImageFormat::BAYER_GB8;
    case GX_PIXEL_FORMAT_BAYER_BG8: return ImageFormat::BAYER_BG8;
    case GX_PIXEL_FORMAT_BAYER_GR10: return ImageFormat::BAYER_GR10;
    case GX_PIXEL_FORMAT_BAYER_RG10: return ImageFormat::BAYER_RG10;
    case GX_PIXEL_FORMAT_BAYER_GB10: return ImageFormat::BAYER_GB10;
    case GX_PIXEL_FORMAT_BAYER_BG10: return ImageFormat::BAYER_BG10;
    case GX_PIXEL_FORMAT_BAYER_GR12: return ImageFormat::BAYER_GR12;
    case GX_PIXEL_FORMAT_BAYER_RG12: return ImageFormat::BAYER_RG12;
    case GX_PIXEL_FORMAT_BAYER_GB12: return ImageFormat::BAYER_GB12;
    case GX_PIXEL_FORMAT_BAYER_BG12: return ImageFormat::BAYER_BG12;
    case GX_PIXEL_FORMAT_BAYER_GR16: return ImageFormat::BAYER_GR16;
    case GX_PIXEL_FORMAT_BAYER_RG16: return ImageFormat::BAYER_RG16;
    case GX_PIXEL_FORMAT_BAYER_GB16: return ImageFormat::BAYER_GB16;
    case GX_PIXEL_FORMAT_BAYER_BG16: return ImageFormat::BAYER_BG16;
    case GX_PIXEL_FORMAT_RGB8: return ImageFormat::RGB24;
    case GX_PIXEL_FORMAT_BGR8: return ImageFormat::BGR24;
    case GX_PIXEL_FORMAT_RGBA8: return ImageFormat::RGBA32;
    case GX_PIXEL_FORMAT_BGRA8: return ImageFormat::BGRA32;
    case GX_PIXEL_FORMAT_ARGB8: return ImageFormat::ARGB32;
    case GX_PIXEL_FORMAT_ABGR8: return ImageFormat::ABGR32;
    case GX_PIXEL_FORMAT_RGB16: return ImageFormat::RGB48;
    case GX_PIXEL_FORMAT_BGR16: return ImageFormat::BGR48;
    case GX_PIXEL_FORMAT_YUV420_8_PLANAR: return ImageFormat::YUV420P;
    case GX_PIXEL_FORMAT_YUV422_8: return ImageFormat::YUYV422;
    case GX_PIXEL_FORMAT_YUV444_8: return ImageFormat::YUV444P;
    default: return ImageFormat::UNKNOWN;
    }
}

inline common::imgproc::ImageFormat ToImageFormat(const std::string& pixelFormat) {
    using common::imgproc::ImageFormat;

    if (pixelFormat == "Mono8") return ImageFormat::MONO8;
    if (pixelFormat == "Mono10") return ImageFormat::MONO10;
    if (pixelFormat == "Mono12") return ImageFormat::MONO12;
    if (pixelFormat == "Mono14") return ImageFormat::MONO14;
    if (pixelFormat == "Mono16") return ImageFormat::MONO16;
    if (pixelFormat == "BayerGR8") return ImageFormat::BAYER_GR8;
    if (pixelFormat == "BayerRG8") return ImageFormat::BAYER_RG8;
    if (pixelFormat == "BayerGB8") return ImageFormat::BAYER_GB8;
    if (pixelFormat == "BayerBG8") return ImageFormat::BAYER_BG8;
    if (pixelFormat == "BayerGR10") return ImageFormat::BAYER_GR10;
    if (pixelFormat == "BayerRG10") return ImageFormat::BAYER_RG10;
    if (pixelFormat == "BayerGB10") return ImageFormat::BAYER_GB10;
    if (pixelFormat == "BayerBG10") return ImageFormat::BAYER_BG10;
    if (pixelFormat == "BayerGR12") return ImageFormat::BAYER_GR12;
    if (pixelFormat == "BayerRG12") return ImageFormat::BAYER_RG12;
    if (pixelFormat == "BayerGB12") return ImageFormat::BAYER_GB12;
    if (pixelFormat == "BayerBG12") return ImageFormat::BAYER_BG12;
    if (pixelFormat == "BayerGR16") return ImageFormat::BAYER_GR16;
    if (pixelFormat == "BayerRG16") return ImageFormat::BAYER_RG16;
    if (pixelFormat == "BayerGB16") return ImageFormat::BAYER_GB16;
    if (pixelFormat == "BayerBG16") return ImageFormat::BAYER_BG16;
    if (pixelFormat == "RGB8") return ImageFormat::RGB24;
    if (pixelFormat == "BGR8") return ImageFormat::BGR24;
    if (pixelFormat == "RGBA8") return ImageFormat::RGBA32;
    if (pixelFormat == "BGRA8") return ImageFormat::BGRA32;
    if (pixelFormat == "ARGB8") return ImageFormat::ARGB32;
    if (pixelFormat == "ABGR8") return ImageFormat::ABGR32;
    if (pixelFormat == "RGB16") return ImageFormat::RGB48;
    if (pixelFormat == "BGR16") return ImageFormat::BGR48;
    if (pixelFormat == "YUV420_8_PLANAR") return ImageFormat::YUV420P;
    if (pixelFormat == "YUV422_8") return ImageFormat::YUYV422;
    if (pixelFormat == "YUV444_8") return ImageFormat::YUV444P;
    return ImageFormat::UNKNOWN;
}
} // namespace

//大恒相机常用修改的参数,防止其他地方使用错误，一个地方修改可以直接同步所有调用
#define GALAXY_ON                       "On"
#define GALAXY_OFF                      "Off"
#define GALAXY_EXPOSURETIME             "ExposureTime"
#define GALAXY_STARTGRAB_CMD            "AcquisitionStart"
#define GALAXY_STOPGRAB_CMD             "AcquisitionStop"
#define GALAXY_TRIGGER_MODE             "TriggerMode"
#define GALAXY_TRIGGER_SOURCE           "TriggerSource"
#define GALAXY_TRIGGER_SOURCE_SOFTWARE  "Software"
#define GALAXY_TRIGGER_SOURCE_LINE0     "Line0" //硬触发line0 输入IO
#define GALAXY_TRIGGER_SOURCE_LINE2     "Line2" //硬触发line2 输入IO
#define GALAXY_TRIGGER_SOURCE_LINE3     "Line3" //硬触发line3 输入IO
#define GALAXY_TRIGGER_SOFTWARE         "TriggerSoftware"
#define GALAXY_CAPTURE_CONTINUES        "Continuous"
#define GALAXY_GAIN                     "Gain"
#define GALAXY_TRIGGER_SELECTOR         "TriggerSelector"
#define GALAXY_SELECTOR_FRAMESTART      "FrameStart"


namespace common
{
namespace device
{
const double dSharpenParamMax = 5;      //锐化最大值 
const double dSharpenParamMin = 0.1;    //锐化最小值
const double dGammmaParamMax = 10;      //Gammma最大值
const double dGammaParamMin = 0.1;      //Gamma最小值
const int nContrastParamMax = 100;      //对比度最大值 
const int nContrastParamMin = -50;      //对比度最小值
const int nLightnessParamMax = 150;     //亮度最大值
const int nLightnessParamMin = -150;    //亮度最小值
const int nSaturationParamMax = 128;    //饱和度最大值 
const int nSaturationParamMin = 0;      //饱和度最小值

template <typename T>
void getLimitValue(const T pLimit, const T nLimit, T& value) {
    if (value > pLimit)
        value = pLimit;
    else if (value < nLimit)
        value = nLimit;
}

bool GalaxyCamera::isInit_ = false;
std::atomic<int> GalaxyCamera::refCnt_(0);

class DeviceOfflineEventHandler : public IDeviceOfflineEventHandler
{
public:
    void DoOnDeviceOfflineEvent(void* userParam)
    {
        if (userParam)
        {
            GalaxyCamera* galaxyCamera = static_cast<GalaxyCamera*>(userParam);
            galaxyCamera->cameraOffline();
        }
    }
};

/**
 * @brief Galaxy SDK 帧回调处理器
 *
 * 线程：SDK 内部线程调用 DoOnImageCaptured，与主线程 SyncGrab/AsyncGrab 并发。
 *
 * 路由规则（asyncFlag_ atomic）：
 * - true  → 异步模式：SDK buffer 拷贝到 asyncCopyBuffer_ 后调用 GrabCB（避免 UAF）
 * - false → 同步模式：拷贝到 imageBuffer_，置 syncCB_，notify SyncGrab 等待线程
 *
 * syncCBed_：任一回帧入口置 true，供 SyncGrab predicate 检测（含失败帧唤醒）
 */
class CaptureEventHandler : public ICaptureEventHandler
{
public:
    ~CaptureEventHandler()
    {
        delete[] imageBuffer_;
        imageBuffer_ = nullptr;
    }

    void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* userParam)
    {
        (void)userParam;
        syncCBed_.store(true, std::memory_order_release);
        if (objImageDataPointer->GetStatus() == GX_FRAME_STATUS_SUCCESS) {
            if (asyncFlag_.load(std::memory_order_acquire)) {
                if (callback_) {
                    unsigned char* buffer = static_cast<unsigned char*>(objImageDataPointer->GetBuffer());
                    const uint64_t buffersize = objImageDataPointer->GetPayloadSize();
                    const uint64_t width = objImageDataPointer->GetWidth();
                    const uint64_t height = objImageDataPointer->GetHeight();
                    const auto pixelFormat = objImageDataPointer->GetPixelFormat();
                    if (buffer && buffersize > 0) {
                        if (asyncCopyBuffer_.size() < buffersize) {
                            asyncCopyBuffer_.resize(static_cast<size_t>(buffersize));
                        }
                        memcpy(asyncCopyBuffer_.data(), buffer, static_cast<size_t>(buffersize));
                        callback_(static_cast<int>(height), static_cast<int>(width),
                                    asyncCopyBuffer_.data(), static_cast<int>(buffersize),
                                    ToImageFormat(pixelFormat));
                    }
                }
            } else {
                std::unique_lock<std::mutex> lck(mtx_);
                syncCB_.store(true, std::memory_order_release);
                unsigned char* buffer = static_cast<unsigned char*>(objImageDataPointer->GetBuffer());
                const uint64_t buffersize = objImageDataPointer->GetPayloadSize();
                const uint64_t width = objImageDataPointer->GetWidth();
                const uint64_t height = objImageDataPointer->GetHeight();
                (void)objImageDataPointer->GetPixelFormat();

                if (buffersize != imageBufSize_) {
                    if (imageBuffer_) {
                        delete[] imageBuffer_;
                        imageBuffer_ = nullptr;
                    }
                    imageBufSize_ = buffersize;
                    imageBuffer_ = new unsigned char[buffersize]{0x00};
                }

                width_ = width;
                height_ = height;

                memcpy(imageBuffer_, buffer, buffersize);
                cond_.notify_one();
            }
        } else {
            if (!asyncFlag_.load(std::memory_order_acquire)) {
                cond_.notify_one();
            }
        }
    }

    void registerCallback(ImageGrabFuncCB callback) {
        callback_ = callback;
    }
public:
    std::mutex mtx_;
    std::condition_variable cond_;
    std::atomic<bool> asyncFlag_{true};
    unsigned char* imageBuffer_ = nullptr;
    uint64_t imageBufSize_ = 0;
    int height_ = 0;
    int width_ = 0;
    std::atomic<bool> syncCB_{false};//认为是回调处理结果，true代表图像获取成功
    std::atomic<bool> syncCBed_{false};//是否回调过，进回调设置成true，cv等待方置false
private:
    ImageGrabFuncCB callback_ = nullptr;
    std::vector<unsigned char> asyncCopyBuffer_;
};

GalaxyCamera::GalaxyCamera() 
{
    deviceOfflineEventHandler_ = new DeviceOfflineEventHandler;
    captureEventHandler_ = new CaptureEventHandler;
    init();
    refCnt_++;
}

GalaxyCamera::~GalaxyCamera() 
{
    (void)Close();

    if (deviceOfflineEventHandler_) {
        delete deviceOfflineEventHandler_;
        deviceOfflineEventHandler_ = nullptr;
    }
    if (captureEventHandler_) {
        delete captureEventHandler_;
        captureEventHandler_ = nullptr;
    }

    refCnt_--;
    uninit(); 
}

common::errors::ErrorPtr GalaxyCamera::Open()
{
    if (m_cameraDevInfo_.connType == CameraConnType::CONN_BY_IP)
        return OpenByIP(m_cameraDevInfo_.ip, m_cameraDevInfo_.port);
    else if (m_cameraDevInfo_.connType == CameraConnType::CONN_BY_SN)
        return OpenBySN(m_cameraDevInfo_.SN);
    else
        return Err("Unsupported connType", common::errors::camera_module::kInvalidConfig);
}

common::errors::ErrorPtr GalaxyCamera::Close()
{
    std::lock_guard<std::mutex> locker(mtx_);
    if (!isConnected_) {
        return Ok();
    }
    if (isGrabbing_) {
        if (auto r = stopGrabImpl(true); r) {
            return r;
        }
    }

    if (deviceOfflineCallbackHandle_) {
        devicePtr_->UnregisterDeviceOfflineCallback(deviceOfflineCallbackHandle_);
        deviceOfflineCallbackHandle_ = nullptr;
    }
    
    if (!streamPtr_.IsNull()) {
        streamPtr_->Close();
    }
    if (!devicePtr_.IsNull()) {
        devicePtr_->Close();
    } 

    SetConnectedState(false);
    SetCaptureState(CaptureState::Idle);
    return Ok();
}

bool GalaxyCamera::IsConnected()
{
    return isConnected_;
}

common::errors::ErrorPtr GalaxyCamera::ReConnect()
{
    std::lock_guard<std::mutex> locker(mtx_);
    try
	{
        if (auto err = resetCamera(GX_MANUFACTURER_SPECIFIC_RECONNECT); err) return err;
    }
    catch (CGalaxyException &e)
    {
        HandleException("相机重连失败", e);
        return Errf(common::errors::camera_module::kSdkError, "Reconnect exception (%s)", GalaxySdkDetail(e).c_str());
    }
    
    SetConnectedState(true);
    isGrabbing_ = false;
    syncGrabInProgress_.store(false);
    SetCaptureState(CaptureState::Idle);
    return Ok();
}


void GalaxyCamera::RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB)
{
    if (captureEventHandler_)
    {
        captureEventHandler_->registerCallback(imageGrabFuncCB);
    }
}


common::errors::ErrorPtr GalaxyCamera::SyncGrab(int &height, int &width, unsigned char *&dataBuffer, int &dataLen)
{
    // syncGrabInProgress_ 阻止外部 StopGrab；析构时 RAII 清标志
    struct SyncGrabScope {
        std::atomic<bool>& flag;
        explicit SyncGrabScope(std::atomic<bool>& f) : flag(f) { flag.store(true); }
        ~SyncGrabScope() { flag.store(false); }
    } syncScope(syncGrabInProgress_);

    std::unique_lock<std::mutex> lock(mtx_);
    if (!checkCamera()) {
        HandleError("相机采集前检测失败", common::errors::camera_module::kNotOpened, "checkCamera failed");
        return Err("checkCamera failed", common::errors::camera_module::kNotOpened);
    }
    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }
    if (isGrabbing_ && captureEventHandler_->asyncFlag_.load(std::memory_order_acquire)) {
        if (auto r = stopGrabImpl(true); r) {
            HandleError("停止采集返回失败", r->Code(), r->Message());
            return r;
        }
    }

    try {
        captureEventHandler_->asyncFlag_.store(false, std::memory_order_release);
        captureEventHandler_->syncCB_.store(false, std::memory_order_release);
        captureEventHandler_->syncCBed_.store(false, std::memory_order_release);
        SetCaptureState(CaptureState::Capturing);

        uint32_t totalMs = 10000;
        bool frameOk = false;
        while (totalMs > 0) {
            streamPtr_->StartGrab();
            featureControlPtr_->GetCommandFeature(GALAXY_STARTGRAB_CMD)->Execute();
            isGrabbing_ = true;

            // 释放 mtx_：允许 cameraOffline/Close 加锁；isGrabbing_ 仍 true 阻挡 Async/Trigger
            lock.unlock();
            std::unique_lock<std::mutex> lck(captureEventHandler_->mtx_);
            const bool signaled = captureEventHandler_->cond_.wait_for(
                lck, std::chrono::milliseconds(2000),
                [&]() {
                    return captureEventHandler_->syncCBed_.load(std::memory_order_acquire);
                });
            lock.lock();

            if (!isConnected_) {
                isGrabbing_ = false;
                SetCaptureState(CaptureState::Idle);
                return Err("camera disconnected during SyncGrab", common::errors::camera_module::kNotOpened);
            }

            captureEventHandler_->syncCBed_.store(false, std::memory_order_release);
            if (signaled && captureEventHandler_->syncCB_.load(std::memory_order_acquire)) {
                frameOk = true;
                break;
            }

            captureEventHandler_->syncCB_.store(false, std::memory_order_release);
            (void)stopGrabImpl(true);
            totalMs -= 2000;
        }

        if (!frameOk) {
            HandleError("图像采集超时", common::errors::camera_module::kFrameTimeout, "SyncGrab timeout");
            isGrabbing_ = false;
            SetCaptureState(CaptureState::Idle);
            captureEventHandler_->syncCBed_.store(false, std::memory_order_release);
            captureEventHandler_->syncCB_.store(false, std::memory_order_release);
            return Err("SyncGrab timeout", common::errors::camera_module::kFrameTimeout);
        }

        captureEventHandler_->syncCBed_.store(false, std::memory_order_release);
        captureEventHandler_->syncCB_.store(false, std::memory_order_release);

        dataBuffer = captureEventHandler_->imageBuffer_;
        width = captureEventHandler_->width_;
        height = captureEventHandler_->height_;
        dataLen = static_cast<int>(captureEventHandler_->imageBufSize_);

        // 借用内部 buffer 返回；调用方须立即拷贝（见 CameraBase::SyncGrab 注释）
        (void)stopGrabImpl(true);
    }
    catch (CGalaxyException& e)
    {
        isGrabbing_ = false;
        SetCaptureState(CaptureState::Idle);
        HandleException("图像采集异常", e);
        return Errf(common::errors::camera_module::kSdkError, "SyncGrab exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::AsyncGrab()
{
    std::lock_guard<std::mutex> locker(mtx_);
    if (!checkCamera()) {
        return Err("checkCamera failed", common::errors::camera_module::kNotOpened);
    }

    // 单帧 TriggerCapture 后流可能仍在跑（isGrabbing_=true + 触发模式），须先停流再切连续采集
    if (isGrabbing_) {
        if (auto err = stopGrabImpl(true); err) {
            return err;
        }
    }

    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }

    try
    {
        captureEventHandler_->asyncFlag_.store(true, std::memory_order_release);
        streamPtr_->StartGrab();
        featureControlPtr_->GetCommandFeature(GALAXY_STARTGRAB_CMD)->Execute();
        isGrabbing_ = true;
        SetCaptureState(CaptureState::Preview);
    }
    catch (CGalaxyException &e)
    {
        isGrabbing_ = false;
        SetCaptureState(CaptureState::Idle);
        HandleException("异步采集失败", e);
        return Errf(common::errors::camera_module::kStartGrabbingFailed, "AsyncGrab exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::TriggerCapture(CameraTriggerSourceType source)
{
    std::lock_guard<std::mutex> locker(mtx_);

    if (!checkCamera()) {
        return Err("checkCamera failed", common::errors::camera_module::kNotOpened);
    }

    // 自由采集预览中切触发：先停流再重建
    if (isGrabbing_ && captureEventHandler_->asyncFlag_.load(std::memory_order_acquire) && !m_triggerSwitch_) {
        if (auto err = stopGrabImpl(); err) {
            return err;
        }
    }

    // 触发流已开启：重复软触发
    if (isGrabbing_ && captureEventHandler_->asyncFlag_.load(std::memory_order_acquire) && m_triggerSwitch_) {
        if (auto err = ensureTriggerMode(source); err) {
            return err;
        }
        if (m_triggerSourceType_ == CameraTriggerSourceType::SOFTWARE_TRIGGER) {
            try {
                featureControlPtr_->GetCommandFeature(GALAXY_TRIGGER_SOFTWARE)->Execute();
            }
            catch (CGalaxyException& e) {
                HandleException("触发采集失败", e);
                return Errf(common::errors::camera_module::kSdkError, "TriggerCapture exception (%s)",
                            GalaxySdkDetail(e).c_str());
            }
        }
        return Ok();
    }

    if (isGrabbing_ && !captureEventHandler_->asyncFlag_.load(std::memory_order_acquire)) {
        HandleError("相机采集失败", common::errors::camera_module::kAlreadyStarted, "sync grab in progress");
        return Err("sync grab in progress", common::errors::camera_module::kAlreadyStarted);
    }

    try {
        if (auto err = ensureTriggerMode(source); err) {
            return err;
        }

        captureEventHandler_->asyncFlag_.store(true, std::memory_order_release);
        streamPtr_->StartGrab();
        featureControlPtr_->GetCommandFeature(GALAXY_STARTGRAB_CMD)->Execute();
        if (m_triggerSourceType_ == CameraTriggerSourceType::SOFTWARE_TRIGGER) {
            featureControlPtr_->GetCommandFeature(GALAXY_TRIGGER_SOFTWARE)->Execute();
        }
        isGrabbing_ = true;
        SetCaptureState(CaptureState::Capturing);
    }
    catch (CGalaxyException& e)
    {
        isGrabbing_ = false;
        SetCaptureState(CaptureState::Idle);
        HandleException("触发采集失败", e);
        return Errf(common::errors::camera_module::kStartGrabbingFailed, "TriggerCapture exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::StopGrab()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return stopGrabImpl();
}


common::errors::ErrorPtr GalaxyCamera::SetAutoWhiteBalance(bool enable) {
    if (featureControlPtr_.IsNull()) {
        return Err("featureControlPtr is null", common::errors::camera_module::kNotOpened);
    }
    try {
        auto enumFeature = featureControlPtr_->GetEnumFeature("BalanceWhiteAuto");
        if (enable) {
            enumFeature->SetValue("Once");  // 或 "Once Continuous"
        }
        else {
            enumFeature->SetValue("Off");
        }
        return Ok();
    }
    catch (const std::exception& e) {
        return Errf(common::errors::camera_module::kSdkError, "SetAutoWhiteBalance exception: %s", e.what());
    }
}

void GalaxyCamera::reflectFromJson(const json& j)
{
    Base_::reflectFromJson(j);
    getFromJson(j, "autoCalibrationBadPoint", m_autoCalibrationBadPoint);
    getFromJson(j, "gammaValue", m_gammaValue);
    getFromJson(j, "contrastValue", m_contrastValue);
    getFromJson(j, "saturationValue", m_saturationValue);
    getFromJson(j, "lightnessValue", m_lightnessValue);
    getFromJson(j, "sharpenValue", m_sharpenValue);
    getFromJson(j, "enableGamma", m_enableGamma);
    getFromJson(j, "enableSharpness", m_enableSharpness);
    getFromJson(j, "enableStaturation", m_enableStaturation);
    getFromJson(j, "enableCheckDenoise", m_enableCheckDenoise);
    getFromJson(j, "pixelFormat", m_pixelFormat);
    getFromJson(j, "reverseX", m_reverseX);
    getFromJson(j, "reverseY", m_reverseY);
    getFromJson(j, "extraParamEnabled", m_extraParamEnabled);
    getFromJson(j, "localMac", m_cameraInfo_.localMac);
}

void GalaxyCamera::reflectToJson(json& j) const
{
    Base_::reflectToJson(j);
    j["autoCalibrationBadPoint"] = m_autoCalibrationBadPoint;
    j["gammaValue"] = m_gammaValue;
    j["contrastValue"] = m_contrastValue;
    j["saturationValue"] = m_saturationValue;
    j["lightnessValue"] = m_lightnessValue;
    j["sharpenValue"] = m_sharpenValue;
    j["enableGamma"] = m_enableGamma;
    j["enableSharpness"] = m_enableSharpness;
    j["enableStaturation"] = m_enableStaturation;
    j["enableCheckDenoise"] = m_enableCheckDenoise;
    j["pixelFormat"] = m_pixelFormat;
    j["reverseX"] = m_reverseX;
    j["reverseY"] = m_reverseY;
    j["extraParamEnabled"] = m_extraParamEnabled;
    j["localMac"] = m_cameraInfo_.localMac;
}

void GalaxyCamera::cameraOffline()
{
    // 仅更新状态+通知；不调用 IGXFactory::Uninit()（全局资源，影响其他实例）
    std::lock_guard<std::mutex> locker(mtx_);
    SetConnectedState(false);
    isGrabbing_ = false;
    syncGrabInProgress_.store(false);
    SetCaptureState(CaptureState::Idle);
    if (cameraOfflineFuncCB_) {
        cameraOfflineFuncCB_(m_cameraInfo_.cameraID + "相机异常掉线！！！");
    }
}


void GalaxyCamera::refreshCameraInfo()
{
    if (featureControlPtr_.IsNull()) {
        return;
    }

    try {
        if (featureControlPtr_->IsImplemented("DeviceModelName")) {
            m_cameraInfo_.cameraModelName =
                featureControlPtr_->GetStringFeature("DeviceModelName")->GetValue();
        }
        if (featureControlPtr_->IsImplemented("DeviceSerialNumber")) {
            m_cameraInfo_.cameraID =
                featureControlPtr_->GetStringFeature("DeviceSerialNumber")->GetValue();
        }
        if (featureControlPtr_->IsImplemented("SensorWidth")) {
            m_cameraInfo_.imageWidth = featureControlPtr_->GetIntFeature("SensorWidth")->GetValue();
        }
        if (featureControlPtr_->IsImplemented("SensorHeight")) {
            m_cameraInfo_.imageHeight =
                featureControlPtr_->GetIntFeature("SensorHeight")->GetValue();
        }

        m_cameraInfo_.pixelFormat = featureControlPtr_->GetEnumFeature("PixelFormat")->GetValue();
        m_cameraInfo_.imageFormat   = ToImageFormat(m_cameraInfo_.pixelFormat);

        std::string pixelSize = featureControlPtr_->GetEnumFeature("PixelSize")->GetValue().c_str();
        if (pixelSize.find("Bpp") != std::string::npos) {
            pixelSize.erase(0, 3);
        }
        m_cameraInfo_.bitDepth = stoi(pixelSize);

        m_cameraInfo_.pixelFormatType = pixelFormatTypeFromSdkString(m_cameraInfo_.pixelFormat);
        m_cameraInfo_.isSupportColor =  (m_cameraInfo_.pixelFormatType == PixelFormatType::Bayer) ? true : false;
    }
    catch (CGalaxyException &e) {
        HandleException("refreshCameraInfo failed", e);
    }
}

common::errors::ErrorPtr GalaxyCamera::setCameraFeatureImpl(CameraFeatureType featureType,
                                                            const CameraFeatureArg& arg)
{
    std::lock_guard<std::mutex> locker(mtx_);
    ErrorPtr triggerErr;
    if (trySetTriggerFeature(featureType, arg, triggerErr)) {
        return triggerErr;
    }

    if (featureControlPtr_.IsNull()) {
        return Err("featureControlPtr is null", common::errors::camera_module::kNotOpened);
    }

    try {
        switch (featureType) {
            case CameraFeatureType::CAMERA_EXPOSURE_TIME:
            if (arg.kind != CameraFeatureArg::Kind::Number) {
                return Err("invalid exposure arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetFloatFeature(GALAXY_EXPOSURETIME)->SetValue(arg.number);
            return Ok();
            case CameraFeatureType::CAMERA_BALANCE_RATIO:
            if (arg.kind != CameraFeatureArg::Kind::Number) {
                return Err("invalid balance ratio arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetFloatFeature("BalanceRatio")->SetValue(static_cast<float>(arg.number));
            return Ok();
            case CameraFeatureType::CAMERA_OFFSETX:
            if (arg.kind != CameraFeatureArg::Kind::Int) {
                return Err("invalid offset arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetIntFeature("OffsetX")->SetValue(arg.integer);
            return Ok();
            case CameraFeatureType::CAMERA_OFFSETY:
            if (arg.kind != CameraFeatureArg::Kind::Int) {
                return Err("invalid offset arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetIntFeature("OffsetY")->SetValue(arg.integer);
            return Ok();
            case CameraFeatureType::CAMERA_HEIGHT:
            if (arg.kind != CameraFeatureArg::Kind::Int) {
                return Err("invalid height arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetIntFeature("Height")->SetValue(arg.integer);
            return Ok();
            case CameraFeatureType::CAMERA_WIDTH:
            if (arg.kind != CameraFeatureArg::Kind::Int) {
                return Err("invalid width arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetIntFeature("Width")->SetValue(arg.integer);
            return Ok();
            case CameraFeatureType::CAMERA_BALANCE_RATIO_SELECTOR:
            if (arg.kind != CameraFeatureArg::Kind::String) {
                return Err("invalid balance selector arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetEnumFeature("BalanceRatioSelector")->SetValue(arg.text.c_str());
            return Ok();
            case CameraFeatureType::CAMERA_LIGHT_SOURCE_PRESET:
            if (arg.kind != CameraFeatureArg::Kind::String) {
                return Err("invalid light source arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetEnumFeature("LightSourcePreset")->SetValue(arg.text.c_str());
            return Ok();
            case CameraFeatureType::CAMERA_BALANCE_WHITE_AUTO:
            if (arg.kind != CameraFeatureArg::Kind::String) {
                return Err("invalid white auto arg", common::errors::camera_module::kInvalidConfig);
            }
            featureControlPtr_->GetEnumFeature("BalanceWhiteAuto")->SetValue(arg.text.c_str());
            return Ok();
            default:
            return Err("feature not supported by galaxy camera", common::errors::camera_module::kInvalidConfig);
        }
    }
    catch (CGalaxyException& e) {
        HandleException("设置相机特征失败", e);
        return Errf(common::errors::camera_module::kSdkError, "setCameraFeatureImpl exception (%s)",
                    GalaxySdkDetail(e).c_str());
    }
}

common::errors::ErrorPtr GalaxyCamera::getCameraFeatureImpl(CameraFeatureType featureType,
                                                            CameraFeatureArg& arg)
{
    std::lock_guard<std::mutex> locker(mtx_);
    if (tryGetTriggerFeature(featureType, arg)) {
        return Ok();
    }

    if (featureControlPtr_.IsNull()) {
        return Err("featureControlPtr is null", common::errors::camera_module::kNotOpened);
    }

    if (featureType == CameraFeatureType::CAMERA_BALANCE_RATIO) {
        SetAutoWhiteBalance(false);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        arg.kind = CameraFeatureArg::Kind::Number;
        arg.number = featureControlPtr_->GetFloatFeature("BalanceRatio")->GetValue();
        return Ok();
    }

    return Err("feature get not supported by galaxy camera", common::errors::camera_module::kInvalidConfig);
}

common::errors::ErrorPtr GalaxyCamera::onTriggerSwitchChanged()
{
    if (featureControlPtr_.IsNull() || !featureControlPtr_->IsImplemented(GALAXY_TRIGGER_MODE)) {
        return Ok();
    }
    try {
        featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_MODE)->SetValue(m_triggerSwitch_ ? GALAXY_ON : GALAXY_OFF);
        return Ok();
    }
    catch (CGalaxyException& e) {
        HandleException("设置触发开关失败", e);
        return Errf(common::errors::camera_module::kSdkError, "onTriggerSwitchChanged exception (%s)",
                    GalaxySdkDetail(e).c_str());
    }
}

common::errors::ErrorPtr GalaxyCamera::onTriggerSourceChanged()
{
    applyTriggerSourceToDevice();
    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::init()
{
    if (!isInit_) {
        
        try
        {
            IGXFactory::GetInstance().Init();

            GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;
            IGXFactory::GetInstance().UpdateAllDeviceList(100, vectorDeviceInfo);
            for (int i = 0; i < vectorDeviceInfo.size(); i++)
            {
                std::string sn = vectorDeviceInfo[i].GetSN().c_str();
                LOG_INFO("相机初始化开始:SN[{}]", sn);
            }
        }
        catch (CGalaxyException &e)
        {
            HandleException("相机初始化失败", e);
            return Errf(common::errors::camera_module::kSdkError, "IGXFactory Init failed (%s)", GalaxySdkDetail(e).c_str());
        }

        isInit_ = true;
    }
    return Ok();
}

void GalaxyCamera::uninit()
{
    if (0 == refCnt_.load()) {
        IGXFactory::GetInstance().Uninit();
        isInit_ = false;
    }
}

common::errors::ErrorPtr GalaxyCamera::initDevicePointer()
{
    if (devicePtr_.IsNull()) {
        return Err("devicePtr is null", common::errors::camera_module::kOpenFailed);
    }

    try
    {
        // 打开第一台设备以及设备下面第一个流
        streamPtr_ = devicePtr_->OpenStream(0);
        // 获取远端设备属性控制器
        featureControlPtr_ = devicePtr_->GetRemoteFeatureControl();
        //提高网络相机的采集性能,设置方法参考以下代码（目前只有千兆网系列相机支持设置最优包长）。
        GX_DEVICE_CLASS_LIST objDeviceClass = devicePtr_->GetDeviceInfo().GetDeviceClass();
        if (GX_DEVICE_CLASS_GEV == objDeviceClass)
        {
            //判断设备是否支持流通道数据包功能
            if (featureControlPtr_->IsImplemented("GevSCPSPacketSize"))
            {
                //获取当前网络环境的最优包长值
                int nPacketSize = streamPtr_->GetOptimalPacketSize();
                //将最优包长值设置为当前设备的流通道包长值
                featureControlPtr_->GetIntFeature("GevSCPSPacketSize")->SetValue(nPacketSize);
            }
        }
        //设置 Buffer 处理模式
        streamPtr_->GetFeatureControl()->GetEnumFeature("StreamBufferHandlingMode")->SetValue("NewestOnly");
    }
    catch (CGalaxyException& e)
    {
        HandleException("相机初始化失败", e);
        return Errf(common::errors::camera_module::kSdkError, "initDevicePointer exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

void GalaxyCamera::initCamera()
{
    // 设置增益
    if (isSettableFeature("GainSelector")) {
        featureControlPtr_->GetEnumFeature("GainSelector")->SetValue("AnalogAll");
    }
    if (isSettableFeature(GALAXY_GAIN)) {
        featureControlPtr_->GetFloatFeature(GALAXY_GAIN)->SetValue(0.0);
    }

    // 设置偏移
    if (isSettableFeature("OffsetX")) {
        featureControlPtr_->GetIntFeature("OffsetX")->SetValue(0);
    }
    if (isSettableFeature("OffsetY")) {
        featureControlPtr_->GetIntFeature("OffsetY")->SetValue(0);
    }

    // 设置翻转
    if (isSettableFeature("ReverseX")) {
        featureControlPtr_->GetBoolFeature("ReverseX")->SetValue(m_reverseX);
    }
    if (isSettableFeature("ReverseY")) {
        featureControlPtr_->GetBoolFeature("ReverseY")->SetValue(m_reverseY);
    }

    // 关闭自动白平衡
    if (isSettableFeature("BalanceWhiteAuto")) {
        featureControlPtr_->GetEnumFeature("BalanceWhiteAuto")->SetValue(GALAXY_OFF);
    }

    // 采集模式
    if (isSettableFeature("AcquisitionMode")) {
        featureControlPtr_->GetEnumFeature("AcquisitionMode")->SetValue(GALAXY_CAPTURE_CONTINUES);
    }

    // 设置像素格式
    //if (isSettableFeature("PixelFormat")) {
    //    featureControlPtr_->GetEnumFeature("PixelFormat")->SetValue(m_pixelFormat.c_str());
    //}

    mac_ = devicePtr_->GetDeviceInfo().GetMAC();

    // 初始化额外参数
    if (m_extraParamEnabled) {
        initImageParams();
    }

    featureControlPtr_->GetCommandFeature(GALAXY_STOPGRAB_CMD)->Execute();
    streamPtr_->StopGrab();

    streamPtr_->RegisterCaptureCallback(captureEventHandler_, this);
    refreshCameraInfo();
}

void GalaxyCamera::applyTriggerSourceToDevice()
{
    if (featureControlPtr_.IsNull() || !featureControlPtr_->IsImplemented(GALAXY_TRIGGER_SOURCE)) {
        HandleError("设置触发源失败", common::errors::camera_module::kInvalidConfig, "trigger source feature is not implemented");
        return;
    }

    try {
        switch (m_triggerSourceType_) {
            case CameraTriggerSourceType::SOFTWARE_TRIGGER:
            featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_SOURCE)->SetValue(GALAXY_TRIGGER_SOFTWARE);
            break;
            case CameraTriggerSourceType::HARD_TRIGGER_LINE0:
            featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_SOURCE)->SetValue(GALAXY_TRIGGER_SOURCE_LINE0);
            break;
            case CameraTriggerSourceType::HARD_TRIGGER_LINE2:
            featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_SOURCE)->SetValue(GALAXY_TRIGGER_SOURCE_LINE2);
            break;
            case CameraTriggerSourceType::HARD_TRIGGER_LINE3:
            featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_SOURCE)->SetValue(GALAXY_TRIGGER_SOURCE_LINE3);
            break;
            default:
            featureControlPtr_->GetEnumFeature(GALAXY_TRIGGER_SOURCE)->SetValue(GALAXY_TRIGGER_SOFTWARE);
            break;
        }
    }
    catch (CGalaxyException& e) {
        HandleException("设置触发源失败", e);
    }
}

common::errors::ErrorPtr GalaxyCamera::resetCamera(GX_RESET_DEVICE_MODE resetMode)
{
    if (!mac_.empty())
    {
        try {
            // 重置相机
            IGXFactory::GetInstance().GigEResetDevice(mac_, resetMode);
        }
        catch (CGalaxyException& e) {
            HandleException("相机重置失败", e);
            return Errf(common::errors::camera_module::kSdkError, "GigEResetDevice failed (%s)", GalaxySdkDetail(e).c_str());
        }
    }
    else
    {
        HandleError("相机mac为空", common::errors::camera_module::kInvalidConfig, "camera mac empty");
        return Err("camera mac empty", common::errors::camera_module::kInvalidConfig);
    }

    return Ok();
}

void GalaxyCamera::initImageParams()
{
    m_cameraProcessConfig = devicePtr_->CreateImageProcessConfig();
    if (isSettableFeature("GammaEnable")) {
        featureControlPtr_->GetBoolFeature("GammaEnable")->SetValue(m_enableGamma);
        getLimitValue(dGammmaParamMax, dGammaParamMin, m_gammaValue);
        m_cameraProcessConfig->SetGammaParam(m_gammaValue);
    }

    if (isSettableFeature("ColorTransformationEnable")) {
        featureControlPtr_->GetBoolFeature("ColorTransformationEnable")->SetValue(m_autoCalibrationBadPoint);
    }

    if (isSettableFeature("SharpnessMode")) {
        featureControlPtr_->GetEnumFeature("SharpnessMode");
        if (m_enableSharpness)
            featureControlPtr_->GetEnumFeature("SharpnessMode")->SetValue("On");
        else
            featureControlPtr_->GetEnumFeature("SharpnessMode")->SetValue("Off");

        getLimitValue(dSharpenParamMax, dSharpenParamMin, m_sharpenValue);
        m_cameraProcessConfig->SetSharpenParam(m_sharpenValue);
    }
    getLimitValue(nLightnessParamMax, nLightnessParamMin, m_lightnessValue);
    m_cameraProcessConfig->SetLightnessParam(m_lightnessValue);

    getLimitValue(nSaturationParamMax, nSaturationParamMin, m_saturationValue);
    m_cameraProcessConfig->SetSaturationParam(m_saturationValue);

    getLimitValue(nContrastParamMax, nContrastParamMin, m_contrastValue);
    m_cameraProcessConfig->SetContrastParam(m_contrastValue);
}

bool GalaxyCamera::checkCamera()
{
    if (streamPtr_.IsNull() || featureControlPtr_.IsNull())
    {
        HandleError("相机采集流通道未打开", common::errors::camera_module::kNotOpened, "stream or feature control is null");
        return false;
    }
    if (!isConnected_) {
        HandleError("相机未连接", common::errors::camera_module::kNotOpened, "camera not connected");
        return false;
    }
    return true;
}

bool GalaxyCamera::isSettableFeature(std::string feature)
{
    if (featureControlPtr_->IsImplemented(feature.c_str()) && featureControlPtr_->IsWritable(feature.c_str())) {
        return true;
    }
    return false;
}


common::errors::ErrorPtr GalaxyCamera::OpenBySN(const std::string& sn)
{
    std::lock_guard<std::mutex> locker(mtx_);
    if (isConnected_) {
        return Ok();
    }

    try
    {
        if (!m_cameraInfo_.localMac.empty()) {
            IGXFactory::GetInstance().GigEResetDevice(gxstring(m_cameraInfo_.localMac.c_str()), GX_RESET_DEVICE_MODE::GX_MANUFACTURER_SPECIFIC_RECONNECT);
        }
        // 使用GX_ACCESS_EXCLUSIVE，在某些情况下会因为权限问题报错-8（Access denied），
        // 可尝试降级为GX_ACCESS_CONTROL模式打开设备（如果允许写入控制，但非完全独占）
        devicePtr_ = IGXFactory::GetInstance().OpenDeviceBySN(sn.c_str(), GX_ACCESS_CONTROL);
        m_cameraInfo_.localMac = devicePtr_->GetDeviceInfo().GetMAC().c_str();

        if (auto err = initDevicePointer(); err) {
            HandleError("相机初始化失败", err->Code(), err->Message());
            return err;
        }

        initCamera();
        if (deviceOfflineEventHandler_) {
            deviceOfflineCallbackHandle_ = devicePtr_->RegisterDeviceOfflineCallback(deviceOfflineEventHandler_, this);
        }

        SetConnectedState(true);
        isGrabbing_ = false;
        SetCaptureState(CaptureState::Idle);
    }
    catch (CGalaxyException& e)
    {
        HandleException("相机初始化失败", e);
        if (!streamPtr_.IsNull())
        {
            streamPtr_->Close();
        }
        if (!devicePtr_.IsNull())
        {
            devicePtr_->Close();
        }
        SetCaptureState(CaptureState::Idle);
        return Errf(common::errors::camera_module::kOpenFailed, "OpenBySN exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::OpenByIP(const std::string& ip, uint16_t port)
{
    (void)port;
    std::lock_guard<std::mutex> locker(mtx_);
    if (isConnected_) {
        return Ok();
    }

    try
    {
        devicePtr_ = IGXFactory::GetInstance().OpenDeviceByIP(ip.c_str(), GX_ACCESS_CONTROL);
        if (auto err = initDevicePointer(); err) {
            HandleError("相机初始化失败", err->Code(), err->Message());
            return err;
        }

        initCamera();
        if (deviceOfflineEventHandler_) {
            deviceOfflineCallbackHandle_ = devicePtr_->RegisterDeviceOfflineCallback(deviceOfflineEventHandler_, this);
        }

        SetConnectedState(true);
        isGrabbing_ = false;
        SetCaptureState(CaptureState::Idle);
    }
    catch (CGalaxyException& e)
    {
        HandleException("相机初始化失败", e);
        if (!streamPtr_.IsNull())
        {
            streamPtr_->Close();
        }
        if (!devicePtr_.IsNull())
        {
            devicePtr_->Close();
        }
        SetCaptureState(CaptureState::Idle);
        return Errf(common::errors::camera_module::kOpenFailed, "OpenByIP exception (%s)", GalaxySdkDetail(e).c_str());
    }

    return Ok();
}

common::errors::ErrorPtr GalaxyCamera::stopGrabImpl(bool forceStop)
{
    // 同步采集中禁止外部 StopGrab；Close 传入 forceStop=true 可打断并 notify SyncGrab
    if (!forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
        HandleError("停止采集失败，同步采集中", common::errors::camera_module::kAlreadyStarted,
                    "sync grab in progress");
        return Err("sync grab in progress", common::errors::camera_module::kAlreadyStarted);
    }
    if (!checkCamera()) {
        return Err("checkCamera failed", common::errors::camera_module::kNotOpened);
    }
    if (!isGrabbing_) {
        return Ok();
    }

    try
    {
        featureControlPtr_->GetCommandFeature(GALAXY_STOPGRAB_CMD)->Execute();
        streamPtr_->StopGrab();
        isGrabbing_ = false;
        if (captureEventHandler_) {
            captureEventHandler_->asyncFlag_.store(false, std::memory_order_release);
        }
        SetCaptureState(CaptureState::Idle);
    }
    catch (CGalaxyException &e)
    {
        HandleException("停止采集时异常", e);
        return Errf(common::errors::camera_module::kStopGrabbingFailed, "StopGrab exception (%s)", GalaxySdkDetail(e).c_str());
    }

    if (forceStop && syncGrabInProgress_.load(std::memory_order_acquire) && captureEventHandler_) {
        captureEventHandler_->syncCBed_.store(true, std::memory_order_release);
        captureEventHandler_->cond_.notify_all();
    }

    return Ok();
}


} // end of namespace device

}