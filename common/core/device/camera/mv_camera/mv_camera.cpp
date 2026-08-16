#include "mv_camera.h"

#include "../camera_error_codes.h"

#include <cstring>
#include <thread>

namespace {

common::imgproc::ImageFormat ToImageFormat(MvGvspPixelType pixelType) {
    using common::imgproc::ImageFormat;

    switch (pixelType) {
    case PixelType_Gvsp_Mono8: return ImageFormat::MONO8;
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed: return ImageFormat::MONO10;
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed: return ImageFormat::MONO12;
    case PixelType_Gvsp_Mono16: return ImageFormat::MONO16;
    case PixelType_Gvsp_RGB8_Packed: return ImageFormat::RGB24;
    case PixelType_Gvsp_BGR8_Packed: return ImageFormat::BGR24;
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed: return ImageFormat::YUYV422;
    case PixelType_Gvsp_BayerGR8: return ImageFormat::BAYER_GR8;
    case PixelType_Gvsp_BayerRG8: return ImageFormat::BAYER_RG8;
    case PixelType_Gvsp_BayerGB8: return ImageFormat::BAYER_GB8;
    case PixelType_Gvsp_BayerBG8: return ImageFormat::BAYER_BG8;
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed: return ImageFormat::BAYER_GR10;
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed: return ImageFormat::BAYER_RG10;
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed: return ImageFormat::BAYER_GB10;
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed: return ImageFormat::BAYER_BG10;
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed: return ImageFormat::BAYER_GR12;
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed: return ImageFormat::BAYER_RG12;
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed: return ImageFormat::BAYER_GB12;
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed: return ImageFormat::BAYER_BG12;
    case PixelType_Gvsp_BayerGR16: return ImageFormat::BAYER_GR16;
    case PixelType_Gvsp_BayerRG16: return ImageFormat::BAYER_RG16;
    case PixelType_Gvsp_BayerGB16: return ImageFormat::BAYER_GB16;
    case PixelType_Gvsp_BayerBG16: return ImageFormat::BAYER_BG16;
    default: return ImageFormat::UNKNOWN;
    }
}

// MVS 图像回调（SDK 线程）：根据 asyncFlag_ 路由到 GrabCB 或 SyncGrab 缓冲
void __stdcall MvImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    if (!pFrameInfo || !pUser) {
        return;
    }
    auto* camera = reinterpret_cast<common::device::MvCamera*>(pUser);
    camera->ImageGrabProcess(static_cast<int>(pFrameInfo->nHeight),
        static_cast<int>(pFrameInfo->nWidth),
        pData,
        static_cast<int>(pFrameInfo->nFrameLen),
        ToImageFormat(pFrameInfo->enPixelType));
}

// MVS exception callback entry
void __stdcall MvExceptionCallBack(unsigned int nMsgType, void* pUser) {
    if (!pUser) {
        return;
    }
    auto* camera = reinterpret_cast<common::device::MvCamera*>(pUser);
    if (nMsgType == MV_EXCEPTION_DEV_DISCONNECT) {
        camera->offlineProcess();
    }
}

} // namespace

namespace common
{
namespace device
{

using namespace common::errors::camera_module;

bool MvCamera::isInit_ = false;
std::atomic<int> MvCamera::refCnt_(0);

MvCamera::MvCamera() {
    if (!isInit_) {
        const int nRet = MV_CC_Initialize();
        if (MV_OK != nRet) {
            // 初始化失败时不抛异常，交给 Open 返回 ErrorPtr
            // 这里只做最小化记录：后续 Open 也会返回错误
            errCode_ = kSdkError;
            errMsg_ = "MV_CC_Initialize failed";
            return;
        }
        isInit_ = true;
    }
    refCnt_++;
}

MvCamera::~MvCamera() {
    (void)Close();
    if (imageBuffer_) {
        delete[] imageBuffer_;
        imageBuffer_ = nullptr;
    }

    refCnt_--;
    if (0 == refCnt_.load()) {
        MV_CC_Finalize();
        isInit_ = false;
    }
}

ErrorPtr MvCamera::Open() {
    std::lock_guard<std::mutex> locker(mtx_);
    if (isOpen_.load()) {
        return Ok();
    }

    if (!isInit_) {
        HandleError("camera sdk not initialized", kSdkError);
        return Err("Mv camera sdk not initialized", kSdkError);
    }

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    const int nRet = MV_CC_EnumDevices(
        MV_GIGE_DEVICE | MV_USB_DEVICE | MV_GENTL_CAMERALINK_DEVICE | MV_GENTL_CXP_DEVICE | MV_GENTL_XOF_DEVICE,
        &stDeviceList);
    if (MV_OK != nRet) {
        HandleError("Enum Devices failed", kSdkError, "MV_CC_EnumDevices failed");
        return Err("MV_CC_EnumDevices failed", kSdkError);
    }

    if (stDeviceList.nDeviceNum <= 0 || stDeviceList.pDeviceInfo == nullptr) {
        HandleError("Find no devices", kDeviceNotFound);
        return Err("camera device not found", kDeviceNotFound);
    }

    bool found = false;
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (!pDeviceInfo) {
            continue;
        }

        bool match = false;
        if (m_cameraDevInfo_.connType == CameraConnType::CONN_BY_SN) {
            match = (!m_cameraDevInfo_.SN.empty() && getSerialNumber(pDeviceInfo) == m_cameraDevInfo_.SN);
        } else if (m_cameraDevInfo_.connType == CameraConnType::CONN_BY_INDEX) {
            match = (static_cast<int>(i) == m_cameraDevInfo_.index);
        } else {
            // 当前 Demo/实现仅支持 SN / INDEX 方式
            match = (m_cameraDevInfo_.connType == CameraConnType::CONN_BY_SN &&
                getSerialNumber(pDeviceInfo) == m_cameraDevInfo_.SN);
        }

        if (match) {
            if (!_open(pDeviceInfo)) {
                (void)_close();
                return Err("open device failed", kOpenFailed);
            }
            found = true;
            break;
        }
    }

    if (!found) {
        HandleError("camera not found by conn opts", kDeviceNotFound,
            "no matching device by connType");
        return Err("camera device not found", kDeviceNotFound);
    }

    if (auto err = initCamera()) {
        (void)_close();
        return err;
    }
    refreshCameraInfo();

    // register callbacks
    int n = MV_CC_RegisterImageCallBackEx(cam_, MvImageCallBackEx, this);
    if (MV_OK != n) {
        (void)_close();
        HandleError("Register image callback failed", kSdkError, "MV_CC_RegisterImageCallBackEx failed");
        return Err("MV_CC_RegisterImageCallBackEx failed", kSdkError);
    }

    n = MV_CC_RegisterExceptionCallBack(cam_, MvExceptionCallBack, this);
    if (MV_OK != n) {
        (void)_close();
        HandleError("Register exception callback failed", kSdkError, "MV_CC_RegisterExceptionCallBack failed");
        return Err("MV_CC_RegisterExceptionCallBack failed", kSdkError);
    }

    if (cameraOnlineFuncCB_) {
        cameraOnlineFuncCB_(m_cameraDevInfo_.SN.empty() ? "camera online" : (m_cameraDevInfo_.SN + " camera online"));
    }

    return Ok();
}

ErrorPtr MvCamera::Close() {
    std::lock_guard<std::mutex> locker(mtx_);
    return _close();
}

bool MvCamera::IsConnected() {
    return cam_ && isOpen_.load() && MV_CC_IsDeviceConnected(cam_);
}

ErrorPtr MvCamera::ReConnect() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        (void)_close();
    }
    return Open();
}

ErrorPtr MvCamera::SyncGrab(int& height, int& width, unsigned char*& dataBuffer, int& dataLen) {
    struct SyncGrabScope {
        std::atomic<bool>& flag;
        explicit SyncGrabScope(std::atomic<bool>& f) : flag(f) { flag.store(true); }
        ~SyncGrabScope() { flag.store(false); }
    } syncScope(syncGrabInProgress_);

    std::unique_lock<std::mutex> lock(mtx_);
    if (!cam_ || !isOpen_.load()) {
        HandleError("SyncGrab camera not opened", kNotOpened);
        return Err("camera not opened", kNotOpened);
    }

    if (isGrabbing_.load() && asyncFlag_.load(std::memory_order_acquire)) {
        if (auto err = stopGrabImpl(true); err) {
            return err;
        }
    }

    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }

    asyncFlag_.store(false, std::memory_order_release);
    isGrabbing_.store(true);
    SetCaptureState(CaptureState::Capturing);
    imageBufSize_ = 0;

    if (!isStartGrabbing_) {
        int nRet = MV_CC_SetImageNodeNum(cam_, imageNodeNum_);
        if (MV_OK != nRet) {
            isGrabbing_.store(false);
            SetCaptureState(CaptureState::Idle);
            HandleError("SetImageNodeNum failed", kSdkError, "MV_CC_SetImageNodeNum failed");
            return Err("MV_CC_SetImageNodeNum failed", kSdkError);
        }

        nRet = MV_CC_StartGrabbing(cam_);
        if (MV_OK != nRet) {
            isGrabbing_.store(false);
            SetCaptureState(CaptureState::Idle);
            HandleError("StartGrabbing failed", kStartGrabbingFailed, "MV_CC_StartGrabbing failed");
            return Err("StartGrabbing failed", kStartGrabbingFailed);
        }
        isStartGrabbing_ = true;
    }

    const int retryCount = 5;
    for (int retry = 0; retry < retryCount; retry++) {
        // 释放 mtx_ 等帧；isGrabbing_==true 阻止其他采图 API 进入
        lock.unlock();
        std::unique_lock<std::mutex> lck(condmtx_);
        const bool got = cond_.wait_for(lck, std::chrono::milliseconds(2000), [&]() { return imageBufSize_ > 0; });
        lock.lock();

        if (!isOpen_.load()) {
            (void)stopGrabImpl(true);
            return Err("camera disconnected during SyncGrab", kNotOpened);
        }

        if (got && imageBufSize_ > 0) {
            height = imgHeight_;
            width = imgWidth_;
            dataBuffer = imageBuffer_;  // 借用内部 buffer，调用方须立即拷贝
            dataLen = static_cast<int>(imageBufSize_);
            isGrabbing_.store(false);
            SetCaptureState(CaptureState::Idle);
            return Ok();
        }

        imageBufSize_ = 0;
        if (retry == retryCount - 1) {
            break;
        }
    }

    (void)stopGrabImpl(true);
    HandleError("SyncGrab frame timeout", kFrameTimeout);
    return Err("frame timeout", kFrameTimeout);
}

ErrorPtr MvCamera::AsyncGrab() {
    std::lock_guard<std::mutex> locker(mtx_);
    if (!cam_ || !isOpen_.load()) {
        HandleError("AsyncGrab camera not opened", kNotOpened);
        return Err("camera not opened", kNotOpened);
    }

    if (isGrabbing_.load()) {
        if (auto err = stopGrabImpl(true); err) {
            return err;
        }
    }

    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }

    asyncFlag_.store(true, std::memory_order_release);
    isGrabbing_.store(true);
    imageBufSize_ = 0;

    if (!isStartGrabbing_) {
        int nRet = MV_CC_StartGrabbing(cam_);
        if (MV_OK != nRet) {
            isGrabbing_.store(false);
            SetCaptureState(CaptureState::Idle);
            HandleError("StartGrabbing failed", kStartGrabbingFailed, "MV_CC_StartGrabbing failed");
            return Err("StartGrabbing failed", kStartGrabbingFailed);
        }
        isStartGrabbing_ = true;
    }

    SetCaptureState(CaptureState::Preview);
    return Ok();
}

ErrorPtr MvCamera::TriggerCapture(CameraTriggerSourceType source) {
    std::lock_guard<std::mutex> locker(mtx_);
    if (!cam_ || !isOpen_.load()) {
        HandleError("TriggerCapture camera not opened", kNotOpened);
        return Err("camera not opened", kNotOpened);
    }

    if (isGrabbing_.load() && asyncFlag_.load(std::memory_order_acquire) && !m_triggerSwitch_) {
        if (auto err = stopGrabImpl(); err) {
            return err;
        }
    }

    if (isGrabbing_.load() && asyncFlag_.load(std::memory_order_acquire) && m_triggerSwitch_) {
        if (auto err = ensureTriggerMode(source); err) {
            return err;
        }
        if (m_triggerSourceType_ == CameraTriggerSourceType::SOFTWARE_TRIGGER) {
            const int nRet = MV_CC_SetCommandValue(cam_, "TriggerSoftware");
            if (MV_OK != nRet) {
                HandleError("TriggerCapture TriggerSoftware failed", kSdkError);
                return Err("TriggerSoftware failed", kSdkError);
            }
        }
        return Ok();
    }
    if (isGrabbing_.load() && !asyncFlag_.load(std::memory_order_acquire)) {
        HandleError("TriggerCapture sync grab in progress", kAlreadyStarted);
        return Err("sync grab in progress", kAlreadyStarted);
    }

    asyncFlag_.store(true, std::memory_order_release);
    isGrabbing_.store(true);
    imageBufSize_ = 0;

    if (auto err = ensureTriggerMode(source); err) {
        isGrabbing_.store(false);
        asyncFlag_.store(false, std::memory_order_release);
        return err;
    }

    if (!isStartGrabbing_) {
        int nRet = MV_CC_SetImageNodeNum(cam_, imageNodeNum_);
        if (MV_OK != nRet) {
            isGrabbing_.store(false);
            asyncFlag_.store(false, std::memory_order_release);
            HandleError("TriggerCapture SetImageNodeNum failed", kSdkError);
            return Err("MV_CC_SetImageNodeNum failed", kSdkError);
        }

        nRet = MV_CC_StartGrabbing(cam_);
        if (MV_OK != nRet) {
            isGrabbing_.store(false);
            asyncFlag_.store(false, std::memory_order_release);
            HandleError("TriggerCapture StartGrabbing failed", kStartGrabbingFailed);
            return Err("StartGrabbing failed", kStartGrabbingFailed);
        }
        isStartGrabbing_ = true;
    }

    if (m_triggerSourceType_ == CameraTriggerSourceType::SOFTWARE_TRIGGER) {
        const int nRet = MV_CC_SetCommandValue(cam_, "TriggerSoftware");
        if (MV_OK != nRet) {
            HandleError("TriggerCapture TriggerSoftware failed", kSdkError);
            return Err("TriggerSoftware failed", kSdkError);
        }
    }

    SetCaptureState(CaptureState::Capturing);
    return Ok();
}

ErrorPtr MvCamera::StopGrab() {
    std::lock_guard<std::mutex> locker(mtx_);
    return stopGrabImpl();
}


void MvCamera::reflectFromJson(const json& j) {
    Base_::reflectFromJson(j);
    getFromJson(j, "pixelFormat", pixelFormat_);
    getFromJson(j, "reConnectAuto", reConnectAuto_);
    getFromJson(j, "reverseX", reverseX_);
    getFromJson(j, "reverseY", reverseY_);
}

void MvCamera::reflectToJson(json& j) const {
    Base_::reflectToJson(j);
    j["pixelFormat"] = pixelFormat_;
    j["reConnectAuto"] = reConnectAuto_;
    j["reverseX"] = reverseX_;
    j["reverseY"] = reverseY_;
}


void MvCamera::offlineProcess() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        isOpen_.store(false);
        SetConnectedState(false);
        isGrabbing_.store(false);
        isStartGrabbing_ = false;
        asyncFlag_.store(false, std::memory_order_release);
        SetCaptureState(CaptureState::Idle);
    }

    if (cameraOfflineFuncCB_) {
        const std::string msg = "相机" + m_cameraDevInfo_.SN + "掉线了!";
        cameraOfflineFuncCB_(msg);
    }

    // 重连在 detached 线程；须保证 Camera 实例生命周期覆盖该线程（通常由 DeviceManager 持有）
    if (reConnectAuto_) {
        std::thread([this]() { (void)ReConnect(); }).detach();
    }
}

void MvCamera::ImageGrabProcess(int height, int width, unsigned char* dataBuffer, int dataLen,
    common::imgproc::ImageFormat imageFormat) {
    if (!isGrabbing_.load()) {
        return;
    }

    if (asyncFlag_.load(std::memory_order_acquire)) {
        // 异步：拷贝到 asyncCopyBuffer_ 再回调，SDK buffer 返回后仍安全
        if (imageGrabFuncCB_ && dataBuffer && dataLen > 0) {
            if (asyncCopyBuffer_.size() < static_cast<size_t>(dataLen)) {
                asyncCopyBuffer_.resize(static_cast<size_t>(dataLen));
            }
            memcpy(asyncCopyBuffer_.data(), dataBuffer, static_cast<size_t>(dataLen));
            imageGrabFuncCB_(height, width, asyncCopyBuffer_.data(), dataLen, imageFormat);
        }
        return;
    }

    // sync grab: 拷贝到内部 buffer，置 imageBufSize_ 唤醒 SyncGrab（condmtx_ 保护 notify/wait）
    std::unique_lock<std::mutex> lck(condmtx_);
    if (!imageBuffer_ || imageBufSize_ != static_cast<uint64_t>(dataLen)) {
        if (imageBuffer_) {
            delete[] imageBuffer_;
            imageBuffer_ = nullptr;
        }
        imageBuffer_ = new unsigned char[dataLen] {0x00};
        imageBufSize_ = 0;
    }

    memcpy(imageBuffer_, dataBuffer, static_cast<size_t>(dataLen));
    imgHeight_ = height;
    imgWidth_ = width;
    imageBufSize_ = static_cast<uint64_t>(dataLen);

    cond_.notify_one();
}


void MvCamera::refreshCameraInfo()
{
    if (!cam_ || !isOpen_.load()) {
        return;
    }

    m_cameraInfo_.cameraID = m_cameraDevInfo_.SN;

    MVCC_STRINGVALUE stStringValue;
    memset(&stStringValue, 0, sizeof(stStringValue));
    MV_CC_GetStringValue(cam_, "DeviceModelName", &stStringValue);
    m_cameraInfo_.cameraModelName = stStringValue.chCurValue;

    MVCC_INTVALUE_EX stIntValue;
    memset(&stIntValue, 0, sizeof(stIntValue));
    MV_CC_GetIntValueEx(cam_, "Width", &stIntValue);
    m_cameraInfo_.imageWidth = stIntValue.nCurValue;
    memset(&stIntValue, 0, sizeof(stIntValue));
    MV_CC_GetIntValueEx(cam_, "Height", &stIntValue);
    m_cameraInfo_.imageHeight = stIntValue.nCurValue;

    MVCC_ENUMVALUE stEnumValue;
    memset(&stEnumValue, 0, sizeof(stEnumValue));
    MV_CC_GetEnumValue(cam_, "PixelSize", &stEnumValue);
    m_cameraInfo_.bitDepth = stEnumValue.nCurValue;

    const auto pixelFormatIt = pixelFormatMap_.find(pixelFormat_);
    const auto pixelType = pixelFormatIt != pixelFormatMap_.end() ? pixelFormatIt->second : PixelType_Gvsp_Undefined;

    m_cameraInfo_.pixelFormat = pixelFormat_;
    m_cameraInfo_.imageFormat = ToImageFormat(pixelType);
    m_cameraInfo_.isSupportColor = isColor(pixelType);
    m_cameraInfo_.pixelFormatType =
        m_cameraInfo_.isSupportColor ? PixelFormatType::Bayer : PixelFormatType::Mono;
}

ErrorPtr MvCamera::setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg)
{
    std::lock_guard<std::mutex> locker(mtx_);
    ErrorPtr triggerErr;
    if (trySetTriggerFeature(featureType, arg, triggerErr)) {
        return triggerErr;
    }

    switch (featureType) {
    case CameraFeatureType::CAMERA_EXPOSURE_TIME: {
        if (arg.kind != CameraFeatureArg::Kind::Number) {
            return Err("invalid exposure arg", kInvalidConfig);
        }
        if (!cam_ || !isOpen_.load()) {
            HandleError("set exposure camera not opened", kNotOpened);
            return Err("camera not opened", kNotOpened);
        }
        const int nRet = MV_CC_SetFloatValue(cam_, "ExposureTime", static_cast<float>(arg.number));
        if (MV_OK != nRet) {
            HandleError("set ExposureTime failed", kSdkError, "MV_CC_SetFloatValue ExposureTime failed");
            return Err("set ExposureTime failed", kSdkError);
        }
        return Ok();
    }
    case CameraFeatureType::CAMERA_FILTER_WHEEL:
    case CameraFeatureType::CAMERA_ND_FILTER_WHEEL:
        return Err("filter wheel not supported by mv camera", kInvalidConfig);
    default:
        return Err("feature not supported by mv camera", kInvalidConfig);
    }
}

ErrorPtr MvCamera::getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg)
{
    std::lock_guard<std::mutex> locker(mtx_);
    if (tryGetTriggerFeature(featureType, arg)) {
        return Ok();
    }

    return Err("feature get not supported by mv camera", kInvalidConfig);
}

ErrorPtr MvCamera::onTriggerSwitchChanged()
{
    if (!cam_ || !isOpen_.load()) {
        return Ok();
    }
    const int mode = m_triggerSwitch_ ? MV_TRIGGER_MODE_ON : MV_TRIGGER_MODE_OFF;
    const int nRet = MV_CC_SetEnumValue(cam_, "TriggerMode", mode);
    if (MV_OK != nRet) {
        HandleError("set TriggerMode failed", kSdkError, "MV_CC_SetEnumValue TriggerMode failed");
        return Err("set TriggerMode failed", kSdkError);
    }
    if (!m_triggerSwitch_) {
        const int acqRet = MV_CC_SetEnumValue(cam_, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);
        if (MV_OK != acqRet) {
            HandleError("set AcquisitionMode failed", kSdkError);
            return Err("set AcquisitionMode failed", kSdkError);
        }
    }
    return Ok();
}

ErrorPtr MvCamera::onTriggerSourceChanged()
{
    if (!cam_ || !isOpen_.load()) {
        return Ok();
    }

    MV_CAM_TRIGGER_SOURCE src = MV_TRIGGER_SOURCE_SOFTWARE;
    switch (m_triggerSourceType_) {
    case CameraTriggerSourceType::SOFTWARE_TRIGGER:
        src = MV_TRIGGER_SOURCE_SOFTWARE;
        break;
    case CameraTriggerSourceType::HARD_TRIGGER_LINE0:
        src = MV_TRIGGER_SOURCE_LINE0;
        break;
    case CameraTriggerSourceType::HARD_TRIGGER_LINE1:
        src = MV_TRIGGER_SOURCE_LINE1;
        break;
    case CameraTriggerSourceType::HARD_TRIGGER_LINE2:
        src = MV_TRIGGER_SOURCE_LINE2;
        break;
    case CameraTriggerSourceType::HARD_TRIGGER_LINE3:
        src = MV_TRIGGER_SOURCE_LINE3;
        break;
    default:
        src = MV_TRIGGER_SOURCE_SOFTWARE;
        break;
    }

    const int nRet = MV_CC_SetEnumValue(cam_, "TriggerSource", src);
    if (MV_OK != nRet) {
        HandleError("set TriggerSource failed", kSdkError, "MV_CC_SetEnumValue TriggerSource failed");
        return Err("set TriggerSource failed", kSdkError);
    }
    return Ok();
}


bool MvCamera::_open(MV_CC_DEVICE_INFO* pstMVDevInfo) {
    if (!pstMVDevInfo) {
        HandleError("open device info is null", kInvalidConfig);
        return false;
    }

    int nRet = MV_CC_CreateHandle(&cam_, pstMVDevInfo);
    if (MV_OK != nRet) {
        HandleError("CreateHandle failed", kOpenFailed, "MV_CC_CreateHandle failed");
        return false;
    }

    nRet = MV_CC_OpenDevice(cam_);
    if (MV_OK != nRet) {
        HandleError("OpenDevice failed", kOpenFailed, "MV_CC_OpenDevice failed");
        return false;
    }

    isOpen_.store(true);
    SetConnectedState(true);
    isStartGrabbing_ = false;
    isGrabbing_.store(false);
    asyncFlag_.store(false, std::memory_order_release);
    SetCaptureState(CaptureState::Idle);
    return true;
}

ErrorPtr MvCamera::_close() {
    (void)stopGrabImpl(true);

    if (cam_ && isOpen_.load()) {
        int nRet = MV_CC_CloseDevice(cam_);
        if (MV_OK != nRet) {
            HandleError("CloseDevice failed", kSdkError, "MV_CC_CloseDevice failed");
            return Err("MV_CC_CloseDevice failed", kSdkError);
        }
        isOpen_.store(false);
        SetConnectedState(false);
    }

    if (cam_) {
        int nRet = MV_CC_DestroyHandle(cam_);
        if (MV_OK != nRet) {
            HandleError("DestroyHandle failed", kSdkError, "MV_CC_DestroyHandle failed");
            return Err("MV_CC_DestroyHandle failed", kSdkError);
        }
        cam_ = nullptr;
    }

    return Ok();
}

ErrorPtr MvCamera::stopGrabImpl(bool forceStop) {
    if (!forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
        HandleError("StopGrab rejected during sync grab", kAlreadyStarted, "sync grab in progress");
        return Err("sync grab in progress", kAlreadyStarted);
    }
    if (!cam_ || !isOpen_.load()) {
        return Ok();
    }

    if (!isGrabbing_.load() && !isStartGrabbing_) {
        return Ok();
    }

    if (isStartGrabbing_) {
        const int nRet = MV_CC_StopGrabbing(cam_);
        if (MV_OK != nRet) {
            HandleError("StopGrabbing failed", kStopGrabbingFailed, "MV_CC_StopGrabbing failed");
            return Err("MV_CC_StopGrabbing failed", kStopGrabbingFailed);
        }
        isStartGrabbing_ = false;
    }

    isGrabbing_.store(false);
    asyncFlag_.store(false, std::memory_order_release);
    SetCaptureState(CaptureState::Idle);

    if (forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
        imageBufSize_ = 0;
        cond_.notify_all();
    }

    return Ok();
}

ErrorPtr MvCamera::initCamera() {
    // default settings
    int nRet = MV_CC_SetIntValueEx(cam_, "OffsetX", 0);
    if (MV_OK != nRet) {
        return Err("set OffsetX failed", kSdkError);
    }
    nRet = MV_CC_SetIntValueEx(cam_, "OffsetY", 0);
    if (MV_OK != nRet) {
        return Err("set OffsetY failed", kSdkError);
    }

    nRet = MV_CC_SetEnumValue(cam_, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
    if (MV_OK != nRet) {
        return Err("set ExposureAuto failed", kSdkError);
    }

    // default to off trigger mode; SyncGrab will reconfigure if needed
    nRet = MV_CC_SetEnumValue(cam_, "TriggerMode", MV_TRIGGER_MODE_OFF);
    if (MV_OK != nRet) {
        return Err("set TriggerMode failed", kSdkError);
    }

    nRet = MV_CC_SetEnumValue(cam_, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);
    if (MV_OK != nRet) {
        return Err("set AcquisitionMode failed", kSdkError);
    }

    nRet = MV_CC_SetIntValueEx(cam_, "GevHeartbeatTimeout", 3000);
    if (MV_OK != nRet) {
        return Err("set GevHeartbeatTimeout failed", kSdkError);
    }

    return Ok();
}


std::string MvCamera::getSerialNumber(MV_CC_DEVICE_INFO* pstMVDevInfo) {
    if (!pstMVDevInfo) {
        return "";
    }

    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stGigEInfo.chSerialNumber;
    } else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
    } else if (pstMVDevInfo->nTLayerType == MV_GENTL_GIGE_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stGigEInfo.chSerialNumber;
    } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CAMERALINK_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stCMLInfo.chSerialNumber;
    } else if (pstMVDevInfo->nTLayerType == MV_GENTL_CXP_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stCXPInfo.chSerialNumber;
    } else if (pstMVDevInfo->nTLayerType == MV_GENTL_XOF_DEVICE) {
        return (char*)pstMVDevInfo->SpecialInfo.stXoFInfo.chSerialNumber;
    }
    return "";
}

bool MvCamera::isColor(MvGvspPixelType enType) const {
    switch (enType) {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
    case PixelType_Gvsp_BayerRBGG8:
    case PixelType_Gvsp_BayerGR16:
    case PixelType_Gvsp_BayerRG16:
    case PixelType_Gvsp_BayerGB16:
    case PixelType_Gvsp_BayerBG16:
        return true;
    default:
        return false;
    }
}


} // namespace device
} // namespace common

