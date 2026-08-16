/**
 * @file image_capture.cpp
 * @brief ImageCapture 实现：格式解析、debayer、双链路回调与预览工作线程
 *
 * 预览链路时序（大图 9344×7000 量级）：
 *   SDK 回调 → 采集器.帧深拷贝(~25ms) → 覆盖 pending → 立即返回
 *   工作线程 → 采集器.预览debayer(~88–160ms) → 采集器.预览DispCB → FrameDispatcher
 *
 * 工步链路时序：
 *   SDK 回调 → 采集器.帧深拷贝 → 采集器.打包原图 → 队列入队 → getImage 出队
 *   工步线程 → 采集器.debayer计算 → normalizeForDisplay（经 convertForDisplay）
 */
#include "image_capture.h"

#include <common/base/imgproc/imgproc.h>
#include <common/core/device/lamphouse/lamphouse_base.h>
#include <common/utility/diagnostics/timing_debug.h>
#include <Logger.h>

#include <opencv2/imgproc.hpp>

#include <chrono>

#include <optional>
#include <string>
#include <opencv2/imgcodecs.hpp>

namespace common {
namespace device {

namespace {

/**
 * 文件内工具函数：格式/Bayer 推断、OpenCV 视图与 RGB 转换。
 * 无状态，均在调用线程执行（debayer 除外由 ImageCapture 成员调度）。
 */

using common::imgproc::ImageFormat;
using common::imgproc::ImageFormatToType;
using common::imgproc::IsBayerFormat;
using common::imgproc::IsColorFormat;
using common::imgproc::IsValidImageType;

/** 优先使用回调 fmt，其次 CameraInfo.imageFormat */
ImageFormat resolveFrameFormat(ImageFormat callbackFormat, const CameraInfo &info)
{
    if (callbackFormat != ImageFormat::UNKNOWN) {
        return callbackFormat;
    }
    if (info.imageFormat != ImageFormat::UNKNOWN) {
        return info.imageFormat;
    }
    return ImageFormat::UNKNOWN;
}

/** 由 ImageFormat + CameraInfo 推断 AlgoMatType（滤片/RGB/Bayer 兜底） */
AlgoMatType matTypeFromFormat(ImageFormat fmt, const CameraInfo &info)
{
    const int imgType = ImageFormatToType(fmt);
    if (imgType >= 0 && IsValidImageType(imgType)) {
        return static_cast<AlgoMatType>(imgType);
    }

    if (info.pixelFormatType == PixelFormatType::Filter) {
        return info.bitDepth > 8 ? ALGO_16UC1 : ALGO_8UC1;
    }
    if (info.pixelFormatType == PixelFormatType::RGB || IsColorFormat(fmt)) {
        return info.bitDepth > 8 ? ALGO_16UC3 : ALGO_8UC3;
    }
    return info.bitDepth > 8 ? ALGO_16UC1 : ALGO_8UC1;
}

/** 耗时日志用：AlgoMatType 可读字符串 */
const char *algoMatTypeLabel(AlgoMatType type)
{
    switch (type) {
    case ALGO_8UC1: return "ALGO_8UC1";
    case ALGO_8SC1: return "ALGO_8SC1";
    case ALGO_16UC1: return "ALGO_16UC1";
    case ALGO_16SC1: return "ALGO_16SC1";
    case ALGO_32SC1: return "ALGO_32SC1";
    case ALGO_32FC1: return "ALGO_32FC1";
    case ALGO_64FC1: return "ALGO_64FC1";
    case ALGO_8UC2: return "ALGO_8UC2";
    case ALGO_8SC2: return "ALGO_8SC2";
    case ALGO_16UC2: return "ALGO_16UC2";
    case ALGO_16SC2: return "ALGO_16SC2";
    case ALGO_32SC2: return "ALGO_32SC2";
    case ALGO_32FC2: return "ALGO_32FC2";
    case ALGO_64FC2: return "ALGO_64FC2";
    case ALGO_8UC3: return "ALGO_8UC3";
    case ALGO_8SC3: return "ALGO_8SC3";
    case ALGO_16UC3: return "ALGO_16UC3";
    case ALGO_16SC3: return "ALGO_16SC3";
    case ALGO_32SC3: return "ALGO_32SC3";
    case ALGO_32FC3: return "ALGO_32FC3";
    case ALGO_64FC3: return "ALGO_64FC3";
    case ALGO_8UC4: return "ALGO_8UC4";
    case ALGO_8SC4: return "ALGO_8SC4";
    case ALGO_16UC4: return "ALGO_16UC4";
    case ALGO_16SC4: return "ALGO_16SC4";
    case ALGO_32SC4: return "ALGO_32SC4";
    case ALGO_32FC4: return "ALGO_32FC4";
    case ALGO_64FC4: return "ALGO_64FC4";
    default: return nullptr;
    }
}

/** 从相机 pixelFormat 字符串解析 Bayer 排列（BGGR/GBRG/GRBG/RGGB） */
BayerType bayerFromPixelFormatString(const std::string &pixelFormat)
{
    if (pixelFormat.empty()) {
        return COLOR_BayerRG;
    }
    const std::string lower = toLowerCopy(pixelFormat);
    if (lower.find("bayerbg") != std::string::npos || lower.find("bggr") != std::string::npos) {
        return COLOR_BayerBG;
    }
    if (lower.find("bayergb") != std::string::npos || lower.find("gbrg") != std::string::npos) {
        return COLOR_BayerGB;
    }
    if (lower.find("bayergr") != std::string::npos || lower.find("grbg") != std::string::npos) {
        return COLOR_BayerGR;
    }
    if (lower.find("bayerrg") != std::string::npos || lower.find("rggb") != std::string::npos) {
        return COLOR_BayerRG;
    }
    return COLOR_BayerRG;
}

/** 由帧格式枚举 + CameraInfo 确定 Debayer 用的 BayerType */
BayerType bayerFromFormat(ImageFormat fmt, const CameraInfo &info)
{
    switch (fmt) {
    case ImageFormat::BAYER_BG8:
    case ImageFormat::BAYER_BG10:
    case ImageFormat::BAYER_BG12:
    case ImageFormat::BAYER_BG14:
    case ImageFormat::BAYER_BG16:
        return COLOR_BayerBG;
    case ImageFormat::BAYER_RG8:
    case ImageFormat::BAYER_RG10:
    case ImageFormat::BAYER_RG12:
    case ImageFormat::BAYER_RG14:
    case ImageFormat::BAYER_RG16:
        return COLOR_BayerRG;
    case ImageFormat::BAYER_GB8:
    case ImageFormat::BAYER_GB10:
    case ImageFormat::BAYER_GB12:
    case ImageFormat::BAYER_GB14:
    case ImageFormat::BAYER_GB16:
        return COLOR_BayerGB;
    case ImageFormat::BAYER_GR8:
    case ImageFormat::BAYER_GR10:
    case ImageFormat::BAYER_GR12:
    case ImageFormat::BAYER_GR14:
    case ImageFormat::BAYER_GR16:
        return COLOR_BayerGR;
    default:
        break;
    }

    if (IsBayerFormat(fmt)) {
        return COLOR_BayerRG;
    }
    if (info.pixelFormatType == PixelFormatType::Filter) {
        return COLOR_BayerNone;
    }
    if (info.pixelFormatType == PixelFormatType::Bayer) {
        return bayerFromPixelFormatString(info.pixelFormat);
    }
    return COLOR_BayerNone;
}

/** 滤片相机不解 Bayer；Bayer 格式或 pixelFormatType=Bayer 时需 debayer */
bool shouldDebayerToRgb(const CameraInfo &info, ImageFormat fmt)
{
    if (info.pixelFormatType == PixelFormatType::Filter) {
        return false;
    }
    const ImageFormat resolved = resolveFrameFormat(fmt, info);
    if (IsBayerFormat(resolved)) {
        return true;
    }
    return info.pixelFormatType == PixelFormatType::Bayer;
}

/** 将 CV_8UC3 RGB Mat 拷贝为独立 AlgoMat（连续内存） */
AlgoMat ownedMatFromCvRgb8(const cv::Mat &rgb8)
{
    if (rgb8.empty() || rgb8.type() != CV_8UC3) {
        return {};
    }
    cv::Mat continuous = rgb8.isContinuous() ? rgb8 : rgb8.clone();
    const unsigned int len =
        static_cast<unsigned int>(static_cast<size_t>(continuous.rows) * continuous.cols * 3);
    return AlgoMatCreateFromMem(continuous.rows, continuous.cols, ALGO_8UC3,
                                reinterpret_cast<const char *>(continuous.data), len);
}

/** BGR/16bit BGR → RGB8 cv::Mat（OpenCV 默认 BGR 通道序） */
bool cvBgrToRgb8(const cv::Mat &bgr, cv::Mat &rgb8)
{
    if (bgr.empty()) {
        return false;
    }
    try {
        if (bgr.type() == CV_16UC3) {
            cv::Mat bgr8;
            if (auto err = common::imgproc::ConvertMat12To8(bgr, bgr8)) {
                (void)err;
                return false;
            }
            cv::cvtColor(bgr8, rgb8, cv::COLOR_BGR2RGB);
            return true;
        }
        if (bgr.type() == CV_8UC3) {
            cv::cvtColor(bgr, rgb8, cv::COLOR_BGR2RGB);
            return true;
        }
    } catch (const cv::Exception &ex) {
        LOG_ERROR("[ImageCapture] cvBgrToRgb8 failed: {}", ex.what());
    }
    return false;
}

/** 校验 AlgoMat 是否为完整 8UC3 缓冲（rows*cols*3） */
bool algoMatMatches8uc3(const AlgoMat &raw)
{
    if (AlgoMatIsEmpty(&raw) || !raw.data || raw.rows <= 0 || raw.cols <= 0) {
        return false;
    }
    if (raw.type != ALGO_8UC3) {
        return false;
    }
    const size_t expected =
        static_cast<size_t>(raw.rows) * static_cast<size_t>(raw.cols) * 3u;
    return raw.len >= expected;
}

/** 非拥有权 cv::Mat 视图；非连续则 clone */
cv::Mat view8uc3FromAlgoMat(const AlgoMat &raw)
{
    cv::Mat view(raw.rows, raw.cols, CV_8UC3, raw.data);
    return view.isContinuous() ? view : view.clone();
}

/** Bayer raw 的非拥有权 cv::Mat 视图（类型由 ImageFormat 决定） */
cv::Mat bayerViewFromRawMat(const AlgoMat &raw, ImageFormat fmt)
{
    const int cvType = ImageFormatToType(fmt);
    if (!IsValidImageType(cvType) || !raw.data) {
        return {};
    }
    return cv::Mat(raw.rows, raw.cols, cvType, raw.data);
}

/** 滤片 Mono 不解 Bayer；Bayer 或 BGR24 需经 normalizeForDisplay */
bool needsRgbConversionImpl(const CameraInfo &info, ImageFormat fmt)
{
    if (info.pixelFormatType == PixelFormatType::Filter) {
        return false;
    }
    const ImageFormat resolved = resolveFrameFormat(fmt, info);
    if (shouldDebayerToRgb(info, fmt)) {
        return true;
    }
    if (resolved == ImageFormat::BGR24) {
        return true;
    }
    return false;
}

} // namespace

void CaptureLightData::reflectFromJson(const json &j)
{
    fromJson(j, "", channels);
}

void CaptureLightData::reflectToJson(json &j) const
{
    toJson(j, channels);
}

void ImageCapture::reflectFromJson(const json &j)
{
    getFromJson(j, "cameraName", m_cameraName_);
    getFromJson(j, "lightName", m_lightName_);
    getFromJson(j, "delayTime", m_delayTime);
    fromJson(j, "lightChannelData", m_defaultCaptureLight_);
    fromJson(j, "axisNames", m_axisNames_);

    if (m_delayTime <= 1000) {
        m_delayTime = 1000;
    }
}

void ImageCapture::reflectToJson(json &j) const
{
    j["cameraName"] = m_cameraName_;
    j["lightName"]  = m_lightName_;
    j["delayTime"]  = m_delayTime;
    toJson(j["lightChannelData"], m_defaultCaptureLight_);
    j["axisNames"] = m_axisNames_;
}

// ---- 配置与设备解析 ----

void ImageCapture::configure(std::string captureName, std::string cameraName, std::string lightName)
{
    m_imageCaptureName_ = std::move(captureName);
    m_cameraName_       = std::move(cameraName);
    m_lightName_        = std::move(lightName);
}

void ImageCapture::setDeviceResolvers(CameraResolver cameraResolver, LamphouseResolver lampResolver)
{
    m_cameraResolver_ = std::move(cameraResolver);
    m_lampResolver_   = std::move(lampResolver);
}

bool ImageCapture::ensureGrabCallbackRegistered()
{
    auto camera = resolveCamera();
    if (!camera) {
        LOG_ERROR("[ImageCapture] camera {} unavailable", m_cameraName_);
        return false;
    }

    std::lock_guard<std::mutex> lock(m_grabCbMutex);
    if (m_grabCbRegistered) {
        return true;
    }

    camera->RegisterGrabCB(
        [this](int height, int width, unsigned char *data, int len, common::imgproc::ImageFormat fmt) {
            imageCallback(height, width, data, len, fmt);
        });
    m_grabCbRegistered = true;
    return true;
}

std::shared_ptr<CameraBase> ImageCapture::resolveCamera() const
{
    if (!m_cameraResolver_ || m_cameraName_.empty()) {
        return nullptr;
    }
    return m_cameraResolver_(m_cameraName_);
}

std::shared_ptr<LamphouseBase> ImageCapture::resolveLamp() const
{
    if (m_lightName_.empty() || !m_lampResolver_) {
        return nullptr;
    }
    return m_lampResolver_(m_lightName_);
}

bool ImageCapture::isCameraConnected() const
{
    const auto camera = resolveCamera();
    return camera && camera->IsConnected();
}

CameraInfo ImageCapture::cameraInfo() const
{
    const auto camera = resolveCamera();
    if (!camera) {
        return {};
    }
    return camera->GetCameraInfo();
}

// ---- 采集启停 ----

void ImageCapture::captureImage(bool execDispFuncCB, const CaptureLightData &light)
{
    if (!ensureGrabCallbackRegistered() || !isCameraConnected()) {
        LOG_ERROR("[ImageCapture] captureImage failed for {}", m_cameraName_);
        return;
    }

    if (light != m_defaultCaptureLight_) {
        m_defaultCaptureLight_ = light;
        setLight(light);
    }

    m_execDispFuncCB  = execDispFuncCB;
    m_acceptingImages = true;
    {
        common::utility::TimingDebugScope triggerTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "采集器.相机软触发",
            m_imageCaptureName_);
        if (auto err = resolveCamera()->TriggerCapture()) {
            LOG_ERROR("[ImageCapture] TriggerCapture failed: {}", err->Message());
            m_acceptingImages = false;
        }
    }
}

bool ImageCapture::continuousCapture(bool execDispFuncCB, const CaptureLightData &light)
{
    if (!ensureGrabCallbackRegistered() || !isCameraConnected()) {
        LOG_ERROR("[ImageCapture] continuousCapture failed for {}", m_cameraName_);
        return false;
    }

    if (light != m_defaultCaptureLight_) {
        m_defaultCaptureLight_ = light;
        setLight(light);
    }

    m_execDispFuncCB  = execDispFuncCB;
    m_acceptingImages = true;
    if (auto err = resolveCamera()->AsyncGrab()) {
        LOG_ERROR("[ImageCapture] AsyncGrab failed: {}", err->Message());
        m_acceptingImages = false;
        return false;
    }
    return true;
}

void ImageCapture::suppressFrameDispatch()
{
    // 不 StopGrab：相机可能仍出帧，但回调入口因 m_acceptingImages=false 直接 notify drain
    m_acceptingImages = false;
    stopPreviewWorker();
    m_drainCV.notify_all();
}

bool ImageCapture::stopCapture(bool waitDrain)
{
    m_acceptingImages = false;
    stopPreviewWorker();
    m_drainCV.notify_all();

    const auto camera = resolveCamera();
    if (!camera) {
        return false;
    }

    if (auto err = camera->StopGrab(); err) {
        LOG_ERROR("[ImageCapture] StopGrab failed for {}: {}", m_cameraName_, err->Message());
    }

    if (waitDrain) {
        // 等待在途 SDK 回调看到 m_acceptingImages=false 并 notify（最多 kDrainQuietMs 静默）
        static constexpr int kDrainQuietMs = 200;
        std::unique_lock<std::mutex> lock(m_drainMutex);
        while (m_drainCV.wait_for(lock, std::chrono::milliseconds(kDrainQuietMs))
               != std::cv_status::timeout) {
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_imageDequeMutex);
        resetFrameCounterLocked();
    }
    return true;
}

// ---- 工步出队 ----

std::shared_ptr<ImageInfo> ImageCapture::getImage()
{
    common::utility::TimingDebugScope timing(common::utility::TimingDebugKeys::CaptureTiming,
                                             "采集器.等待出队", m_imageCaptureName_);
    std::unique_lock<std::mutex> lock(m_imageDequeMutex);
    if (!m_imageDequeCV.wait_for(lock, std::chrono::milliseconds(m_delayTime),
                               [this] { return !m_imageDeque.empty(); })) {
        LOG_ERROR("[ImageCapture] getImage timeout");
        return nullptr;
    }

    auto image = m_imageDeque.front();
    m_imageDeque.pop_front();
    return image;
}

std::vector<std::shared_ptr<ImageInfo>> ImageCapture::getAllTypeImage()
{
    std::vector<std::shared_ptr<ImageInfo>> vec;
    std::shared_ptr<ImageInfo>              image;
    do {
        image = getImage();
        if (image) {
            vec.emplace_back(image);
        }
    } while (image != nullptr);
    return vec;
}

void ImageCapture::setLight(const CaptureLightData &light)
{
    const auto lamp = resolveLamp();
    if (!lamp) {
        return;
    }
    for (int i = 1; i <= lamp->GetChannelNum(); ++i) {
        lamp->SetBrightness(static_cast<uint8_t>(i), static_cast<uint8_t>(light.channel(i)));
    }
}

AlgoMatType ImageCapture::getAlgoMatType(common::imgproc::ImageFormat frameFormat,
                                         const CameraInfo &info) const
{
    return matTypeFromFormat(resolveFrameFormat(frameFormat, info), info);
}

BayerType ImageCapture::getBayerType(common::imgproc::ImageFormat frameFormat, const CameraInfo &info) const
{
    return bayerFromFormat(resolveFrameFormat(frameFormat, info), info);
}

bool ImageCapture::needsRgbConversion(const ImageInfo &frame, const CameraInfo &cameraInfo) const
{
    if (!frame.imageMat || AlgoMatIsEmpty(frame.imageMat.get())) {
        return false;
    }
    return needsRgbConversionImpl(cameraInfo, frame.imageFormat);
}

// ---- 像素格式转换（debayer / BGR→RGB / 原样透传）----
// 调用关系：
//   packRawFrame        — 回调线程：仅包装 ImageInfo，保留原始 imageFormat
//   normalizeForDisplay — 按 imageFormat + CameraInfo 决定是否 debayer（核心实现）
//   convertForDisplay   — 预览/工步：detach Mat 后 normalizeForDisplay
//   makeRgbForProcess   — 工步：clone 原图后再 convertForDisplay，不破坏队列中的 raw

std::shared_ptr<ImageInfo> ImageCapture::normalizeForDisplay(
    AlgoMat &&rawMat, ImageFormat frameFormat, const CameraInfo &cameraInfo) const
{
    if (AlgoMatIsEmpty(&rawMat)) {
        return nullptr;
    }

    const ImageFormat resolved = resolveFrameFormat(frameFormat, cameraInfo);
    auto              imageInfo = std::make_shared<ImageInfo>();

    if (shouldDebayerToRgb(cameraInfo, frameFormat)) {
        const cv::Mat bayerView = bayerViewFromRawMat(rawMat, resolved);
        if (bayerView.empty()) {
            LOG_ERROR("[ImageCapture] invalid bayer view for {}", m_cameraName_);
            AlgoMatFree(&rawMat);
            return nullptr;
        }

        
        cv::Mat bgr;        // debayer以后是RGB通道？
        if (auto err = common::imgproc::Debayer(bayerView, resolved, bgr)) {
            LOG_ERROR("[ImageCapture] Debayer failed for {}: {}", m_cameraName_, err->Message());
            AlgoMatFree(&rawMat);
            return nullptr;
        }

        AlgoMat rgbOwned = ownedMatFromCvRgb8(bgr);
        AlgoMatFree(&rawMat);
        if (AlgoMatIsEmpty(&rgbOwned)) {
            LOG_ERROR("[ImageCapture] RGB AlgoMat create failed for {}", m_cameraName_);
            return nullptr;
        }

        imageInfo->imageMat    = adoptAlgoMat(std::move(rgbOwned));
        imageInfo->imageFormat = ImageFormat::RGB24;
        imageInfo->bayerType   = COLOR_BayerNone;
        return imageInfo;
    }

    // 相机直出 BGR24：转 RGB24 供 UI 显示
    if (resolved == ImageFormat::BGR24) {
        if (!algoMatMatches8uc3(rawMat)) {
            LOG_ERROR("[ImageCapture] BGR24 buffer size mismatch for {}: {}x{} len={}",
                      m_cameraName_, rawMat.rows, rawMat.cols, rawMat.len);
            AlgoMatFree(&rawMat);
            return nullptr;
        }
        const cv::Mat bgr = view8uc3FromAlgoMat(rawMat);
        cv::Mat       rgb8;
        if (!cvBgrToRgb8(bgr, rgb8)) {
            AlgoMatFree(&rawMat);
            return nullptr;
        }
        AlgoMat rgbOwned = ownedMatFromCvRgb8(rgb8);
        AlgoMatFree(&rawMat);
        if (AlgoMatIsEmpty(&rgbOwned)) {
            return nullptr;
        }
        imageInfo->imageMat    = adoptAlgoMat(std::move(rgbOwned));
        imageInfo->imageFormat = ImageFormat::RGB24;
        imageInfo->bayerType   = COLOR_BayerNone;
        return imageInfo;
    }

    // 已是 RGB 或彩色直出：透传 AlgoMat
    if (resolved == ImageFormat::RGB24
        || (IsColorFormat(resolved) && cameraInfo.pixelFormatType == PixelFormatType::RGB)) {
        imageInfo->imageMat    = adoptAlgoMat(std::move(rawMat));
        imageInfo->imageFormat = ImageFormat::RGB24;
        imageInfo->bayerType   = COLOR_BayerNone;
        return imageInfo;
    }

    // 滤片 Mono 等：保留原始格式，不解 Bayer
    imageInfo->imageMat    = adoptAlgoMat(std::move(rawMat));
    imageInfo->imageFormat = resolved;
    imageInfo->bayerType   = getBayerType(frameFormat, cameraInfo);
    return imageInfo;
}

std::shared_ptr<ImageInfo> ImageCapture::normalizeForDisplay(
    std::shared_ptr<ImageInfo> raw, const CameraInfo &cameraInfo) const
{
    if (!raw || !raw->imageMat || AlgoMatIsEmpty(raw->imageMat.get())) {
        return nullptr;
    }

    AlgoMat ownedMat = detachAlgoMat(raw->imageMat);
    if (AlgoMatIsEmpty(&ownedMat)) {
        return nullptr;
    }

    auto out = normalizeForDisplay(std::move(ownedMat), raw->imageFormat, cameraInfo);
    if (out) {
        out->frameIndex = raw->frameIndex;
    }
    return out;
}

/** 工步/预览共用：packRawFrame 仅写元数据，不在此 debayer */
std::shared_ptr<ImageInfo> ImageCapture::packRawFrame(AlgoMat &&rawMat, ImageFormat frameFormat,
                                                     const CameraInfo &cameraInfo) const
{
    if (AlgoMatIsEmpty(&rawMat)) {
        return nullptr;
    }

    const ImageFormat resolved = resolveFrameFormat(frameFormat, cameraInfo);
    auto              imageInfo = std::make_shared<ImageInfo>();
    imageInfo->imageMat         = adoptAlgoMat(std::move(rawMat));
    imageInfo->imageFormat      = resolved;
    imageInfo->bayerType        = getBayerType(frameFormat, cameraInfo);
    return imageInfo;
}

// ---- ImageInfo 拷贝与工步 debayer ----

std::shared_ptr<ImageInfo> ImageCapture::cloneImageInfo(
    const std::shared_ptr<ImageInfo> &src)
{
    if (!src || !src->imageMat || AlgoMatIsEmpty(src->imageMat.get())) {
        return nullptr;
    }
    AlgoMat copy{};
    AlgoMatCopy(src->imageMat.get(), &copy);
    if (AlgoMatIsEmpty(&copy)) {
        return nullptr;
    }
    auto out         = std::make_shared<ImageInfo>();
    out->imageMat    = adoptAlgoMat(std::move(copy));
    out->imageFormat = src->imageFormat;
    out->bayerType   = src->bayerType;
    out->frameIndex  = src->frameIndex;
    return out;
}

std::shared_ptr<ImageInfo> ImageCapture::makeRgbForProcess(
    const std::shared_ptr<ImageInfo> &raw) const
{
    if (!raw) {
        return nullptr;
    }
    auto debayerInput = cloneImageInfo(raw);
    if (!debayerInput) {
        return nullptr;
    }
    return convertForDisplay(std::move(debayerInput), cameraInfo());
}

std::shared_ptr<ImageInfo> ImageCapture::convertForDisplay(
    std::shared_ptr<ImageInfo> raw, const CameraInfo &cameraInfo) const
{
    if (!raw || !raw->imageMat || AlgoMatIsEmpty(raw->imageMat.get())) {
        return nullptr;
    }

    common::utility::TimingDebugScope debayerTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "采集器.debayer计算",
        m_imageCaptureName_);
    return normalizeForDisplay(std::move(raw), cameraInfo);
}

// ---- 相机 Grab 回调（SDK 线程）----

void ImageCapture::imageCallback(int height,
                                 int width,
                                 unsigned char *dataBuffer,
                                 int dataLen,
                                 common::imgproc::ImageFormat imageFormat)
{
    // 预览：帧深拷贝 → 覆盖 pending → 工作线程 debayer → DispCB（不入队）
    // 工步：帧深拷贝 → 打包原图 → 入队（不上抛 UI）
    if (!m_acceptingImages) {
        m_drainCV.notify_all();
        return;
    }

    if (!dataBuffer || dataLen <= 0 || height <= 0 || width <= 0) {
        LOG_ERROR("[ImageCapture] imageCallback invalid buffer");
        return;
    }

    const CameraInfo  info    = cameraInfo();
    const AlgoMatType matType = getAlgoMatType(imageFormat, info);
    const char *const matTypeName = algoMatTypeLabel(matType);
    const std::string matTypeDetail =
        matTypeName ? std::string(matTypeName)
                    : ("ALGO_" + std::to_string(static_cast<int>(matType)));
    const std::string frameDetail = std::to_string(width) + "x" + std::to_string(height) + " "
                                    + std::to_string(dataLen / 1024) + "KB matType="
                                    + matTypeDetail;

    AlgoMat rawMat;
    {
        common::utility::TimingDebugScope copyTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "采集器.帧深拷贝",
            m_imageCaptureName_ + " " + frameDetail);
        rawMat = AlgoMatCreateFromMem(height,
                                      width,
                                      matType,
                                      reinterpret_cast<const char *>(dataBuffer),
                                      static_cast<unsigned int>(dataLen));
    }
    if (AlgoMatIsEmpty(&rawMat)) {
        LOG_ERROR("[ImageCapture] imageCallback AlgoMatCreateFromMem failed");
        return;
    }

    if (m_execDispFuncCB) {
        // 预览：与工步相同先 packRawFrame，worker 内再按 imageFormat 决定是否 debayer
        std::shared_ptr<ImageInfo> rawFrame;
        {
            common::utility::TimingDebugScope packTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "采集器.预览打包原图",
                m_imageCaptureName_ + " " + frameDetail);
            rawFrame = packRawFrame(std::move(rawMat), imageFormat, info);
        }
        if (!rawFrame) {
            return;
        }

        PreviewRawPacket packet;
        packet.rawFrame    = std::move(rawFrame);
        packet.cameraInfo  = info;
        packet.frameDetail = frameDetail;
        {
            std::lock_guard<std::mutex> lock(m_previewMutex);
            m_pendingPreviewRaw = std::move(packet);
        }
        ensurePreviewWorkerStarted();
        m_previewCV.notify_one();
        return;
    }

    // 工步：打包原图入队
    std::shared_ptr<ImageInfo> imageInfo;
    {
        common::utility::TimingDebugScope packTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "采集器.打包原图",
            m_imageCaptureName_ + " " + frameDetail);
        imageInfo = packRawFrame(std::move(rawMat), imageFormat, info);
    }
    if (!imageInfo) {
        return;
    }

    imageInfo->frameIndex = ++m_frameCounter;

    {
        std::lock_guard<std::mutex> lock(m_imageDequeMutex);
        if (m_imageDeque.size() >= static_cast<size_t>(m_maxImageCount)) {
            m_imageDeque.pop_front();
        }
        {
            common::utility::TimingDebugScope enqueueTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "采集器.队列入队",
                m_imageCaptureName_ + " " + frameDetail);
            m_imageDeque.push_back(imageInfo);
        }
    }
    m_imageDequeCV.notify_one();
}

void ImageCapture::setMaxImageCount(int count) { m_maxImageCount = count; }

void ImageCapture::clearImageDeque()
{
    std::lock_guard<std::mutex> lock(m_imageDequeMutex);
    m_imageDeque.clear();
    resetFrameCounterLocked();
}

void ImageCapture::setExposureTime(double exposureTime)
{
    const auto camera = resolveCamera();
    if (camera) {
        camera->SetExposureTime(exposureTime);
    }
}

void ImageCapture::resetFrameCounterLocked() { m_frameCounter = 0; }

// ---- 预览 debayer 工作线程 ----

ImageCapture::~ImageCapture() { stopPreviewWorker(); }

void ImageCapture::ensurePreviewWorkerStarted()
{
    std::lock_guard<std::mutex> lock(m_previewMutex);
    if (m_previewWorkerThread.joinable()) {
        return; // 已在运行
    }
    m_previewWorkerStop = false;
    m_previewWorkerThread = std::thread(&ImageCapture::previewWorkerLoop, this);
}

void ImageCapture::stopPreviewWorker()
{
    {
        std::lock_guard<std::mutex> lock(m_previewMutex);
        if (!m_previewWorkerThread.joinable()) {
            return;
        }
        m_previewWorkerStop = true;
        m_pendingPreviewRaw.reset();
    }
    m_previewCV.notify_all();
    m_previewWorkerThread.join(); // join 后 thread 不可 joinable，下次采集会新建线程
}

void ImageCapture::previewWorkerLoop()
{
    // 策略：debayer 前合并 raw（丢掉中间帧原图）；debayer 后必须 DispCB。
    // 切勿在 debayer 后因 pending 非空而 continue 丢弃 RGB——相机帧间隔(~68ms)短于 debayer(~56ms)
    // 时 pending 几乎总非空，会导致只 debayer 不上屏（日志里长时间无 DispCB/UI 刷新）。
    while (true) {
        PreviewRawPacket packet;
        {
            std::unique_lock<std::mutex> lock(m_previewMutex);
            if (!m_pendingPreviewRaw) {
                m_previewCV.wait(lock, [this] {
                    return m_previewWorkerStop.load() || m_pendingPreviewRaw.has_value();
                });
            }
            if (m_previewWorkerStop && !m_pendingPreviewRaw) {
                return;
            }
            if (!m_pendingPreviewRaw) {
                continue;
            }
            packet = std::move(*m_pendingPreviewRaw);
            m_pendingPreviewRaw.reset();
            while (m_pendingPreviewRaw) {
                packet = std::move(*m_pendingPreviewRaw);
                m_pendingPreviewRaw.reset();
            }
        }

        if (m_previewWorkerStop.load()) {
            return;
        }

        std::shared_ptr<ImageInfo> imageInfo;
        {
            common::utility::TimingDebugScope debayerTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "采集器.预览debayer",
                m_imageCaptureName_ + " " + packet.frameDetail);
            imageInfo = convertForDisplay(std::move(packet.rawFrame), packet.cameraInfo);
        }
        if (!imageInfo) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_previewMutex);
            if (m_previewWorkerStop.load()) {
                return;
            }
            imageInfo->frameIndex = ++m_frameCounter;
        }
        if (m_imageDispFuncCB_) {
            common::utility::TimingDebugScope dispTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "采集器.预览DispCB",
                m_imageCaptureName_ + " " + packet.frameDetail);
            m_imageDispFuncCB_(m_imageCaptureName_, imageInfo);
        }
        // pending 非空则立即下一轮（不 wait），FrameDispatcher 侧仍合并为最新 QImage
    }
}

} // namespace device
} // namespace common
