#include "virtual_camera.h"

#include "../camera_error_codes.h"

#include <Config.h>
#include <Logger.h>
#include <utility/diagnostics/timing_debug.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <thread>

namespace common {
namespace device {

namespace {

bool isHardTrigger(CameraTriggerSourceType source)
{
    return source != CameraTriggerSourceType::SOFTWARE_TRIGGER;
}

std::string toLowerExt(const std::filesystem::path &path)
{
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

common::imgproc::ImageFormat ToImageFormat(const std::string &pixelFormat)
{
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

VirtualCamera::~VirtualCamera()
{
    (void)Close();
}

void VirtualCamera::loadConfig()
{
    imageRootPath_ = CFG_GET("virtual_debug.camera.image_path", std::string, "");
    const auto fps = CFG_GET("virtual_debug.camera.frame_rate", int64_t, 10);
    frameRateFps_  = static_cast<int>(fps > 0 ? fps : 10);

    const auto cfgW = CFG_GET("virtual_debug.camera.width", int64_t, 1920);
    const auto cfgH = CFG_GET("virtual_debug.camera.height", int64_t, 1080);
    syntheticWidth_  = static_cast<int>(cfgW > 0 ? cfgW : 1920);
    syntheticHeight_ = static_cast<int>(cfgH > 0 ? cfgH : 1080);

    m_cameraInfo_.pixelFormat = pixelFormat_;
    m_cameraInfo_.imageFormat = ToImageFormat(pixelFormat_);
}

void VirtualCamera::reflectFromJson(const json &j)
{
    CameraBase::reflectFromJson(j);
    getFromJson(j, "pixelFormat", pixelFormat_);
}

void VirtualCamera::reflectToJson(json &j) const
{
    CameraBase::reflectToJson(j);
    j["pixelFormat"] = pixelFormat_;
}

bool VirtualCamera::isRawFile(const std::filesystem::path &path) const
{
    return path.has_extension() && toLowerExt(path) == ".raw";
}

bool VirtualCamera::isImageFile(const std::filesystem::path &path) const
{
    if (!path.has_extension()) {
        return false;
    }
    const std::string ext = toLowerExt(path);
    return ext == ".raw" || ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp"
           || ext == ".tif" || ext == ".tiff";
}

void VirtualCamera::scanPlaylist()
{
    flatImages_.clear();
    subdirGroups_.clear();
    playlistSequence_.clear();
    clearPlaylistCache();
    useSyntheticOnly_ = true;

    if (imageRootPath_.empty()) {
        return;
    }

    std::error_code ec;
    const std::filesystem::path root(imageRootPath_);
    if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) {
        LOG_WARN("[VirtualCamera] image path not found: {}", imageRootPath_);
        return;
    }

    std::vector<std::filesystem::path> subdirs;
    for (const auto &entry : std::filesystem::directory_iterator(root, ec)) {
        if (ec) {
            break;
        }
        if (entry.is_directory()) {
            subdirs.push_back(entry.path());
        } else if (entry.is_regular_file() && isImageFile(entry.path())) {
            flatImages_.push_back(entry.path());
        }
    }

    std::sort(flatImages_.begin(), flatImages_.end());
    std::sort(subdirs.begin(), subdirs.end());

    for (const auto &subdir : subdirs) {
        std::vector<std::filesystem::path> group;
        for (const auto &entry : std::filesystem::directory_iterator(subdir, ec)) {
            if (ec) {
                break;
            }
            if (entry.is_regular_file() && isImageFile(entry.path())) {
                group.push_back(entry.path());
            }
        }
        std::sort(group.begin(), group.end());
        if (!group.empty()) {
            subdirGroups_.push_back(std::move(group));
        }
    }

    useSyntheticOnly_ = flatImages_.empty() && subdirGroups_.empty();
    if (useSyntheticOnly_) {
        LOG_WARN("[VirtualCamera] no images under {}, fallback to synthetic", imageRootPath_);
    } else {
        buildPlaylistSequence();
        {
            std::lock_guard<std::mutex> cacheLock(cacheMtx_);
            playlistCache_.assign(playlistSequence_.size(), CachedFrame{});
        }
        LOG_INFO(
            "[VirtualCamera] playlist: {} subdir group(s), {} flat image(s), {} frame(s) "
            "(background preload)",
            subdirGroups_.size(), flatImages_.size(), playlistSequence_.size());
    }
}

void VirtualCamera::buildPlaylistSequence()
{
    playlistSequence_.clear();
    for (const auto &group : subdirGroups_) {
        playlistSequence_.insert(playlistSequence_.end(), group.begin(), group.end());
    }
    playlistSequence_.insert(playlistSequence_.end(), flatImages_.begin(), flatImages_.end());
}

void VirtualCamera::clearPlaylistCache()
{
    std::lock_guard<std::mutex> lock(cacheMtx_);
    playlistCache_.clear();
}

void VirtualCamera::preloadPlaylistCache(const std::vector<std::filesystem::path> &sequence)
{
    if (sequence.empty()) {
        return;
    }

    size_t loadedCount = 0;
    size_t totalBytes  = 0;
    const auto preloadStart = std::chrono::steady_clock::now();
    for (size_t i = 0; i < sequence.size(); ++i) {
        CachedFrame local;
        local.valid = false;
        if (loadMediaFromPath(sequence[i], local.pixels, local.width, local.height, local.format)) {
            local.valid = true;
            totalBytes += local.pixels.size();
            ++loadedCount;
            std::lock_guard<std::mutex> lock(cacheMtx_);
            if (i < playlistCache_.size())
                playlistCache_[i] = std::move(local);
        } else {
            LOG_WARN("[VirtualCamera] playlist preload failed: {}", sequence[i].string());
        }
    }

    const auto preloadMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now() - preloadStart)
                               .count();
    if (loadedCount == 0) {
        LOG_WARN("[VirtualCamera] playlist preload failed, fallback to synthetic");
        std::lock_guard<std::mutex> lock(mtx_);
        playlistSequence_.clear();
        clearPlaylistCache();
        useSyntheticOnly_ = true;
    } else {
        LOG_INFO(
            "[VirtualCamera] playlist preload done: {}/{} frame(s), {:.1f} MB, {} ms",
            loadedCount, sequence.size(),
            static_cast<double>(totalBytes) / (1024.0 * 1024.0), preloadMs);
        if (loadedCount < sequence.size()) {
            LOG_WARN("[VirtualCamera] playlist preload partial: {}/{}", loadedCount,
                     sequence.size());
        }
    }
}

void VirtualCamera::startPlaylistPreloadAsync()
{
    joinPlaylistPreload();

    std::vector<std::filesystem::path> sequence;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (useSyntheticOnly_ || playlistSequence_.empty())
            return;
        sequence = playlistSequence_;
        if (playlistCache_.size() != sequence.size()) {
            std::lock_guard<std::mutex> cacheLock(cacheMtx_);
            playlistCache_.assign(sequence.size(), CachedFrame{});
        }
    }

    preloadThread_ = std::thread([this, sequence = std::move(sequence)]() {
        preloadPlaylistCache(sequence);
    });
}

void VirtualCamera::joinPlaylistPreload()
{
    if (preloadThread_.joinable())
        preloadThread_.join();
}

const VirtualCamera::CachedFrame *VirtualCamera::getCachedFrame(size_t index) const
{
    std::lock_guard<std::mutex> lock(cacheMtx_);
    if (playlistCache_.empty()) {
        return nullptr;
    }
    const auto &cached = playlistCache_[index % playlistCache_.size()];
    if (!cached.valid || cached.pixels.empty()) {
        return nullptr;
    }
    return &cached;
}

const VirtualCamera::CachedFrame *VirtualCamera::getCachedFrameByPath(
    const std::filesystem::path &path) const
{
    std::lock_guard<std::mutex> lock(cacheMtx_);
    for (size_t i = 0; i < playlistSequence_.size(); ++i) {
        if (playlistSequence_[i] == path) {
            if (playlistCache_.empty())
                return nullptr;
            const auto &cached = playlistCache_[i];
            if (!cached.valid || cached.pixels.empty())
                return nullptr;
            return &cached;
        }
    }
    return nullptr;
}

std::vector<std::filesystem::path> VirtualCamera::currentHardTriggerGroup() const
{
    if (!subdirGroups_.empty()) {
        return subdirGroups_[hardSubdirIndex_ % subdirGroups_.size()];
    }
    return flatImages_;
}

size_t VirtualCamera::nextSequentialIndex()
{
    if (playlistSequence_.empty()) {
        return 0;
    }
    const size_t idx = sequentialIndex_ % playlistSequence_.size();
    ++sequentialIndex_;
    return idx;
}

void VirtualCamera::updateCameraInfoFromFrame(int w, int h, common::imgproc::ImageFormat fmt)
{
    if (cameraInfoInitialized_) {
        return;
    }
    auto &info              = GetCameraInfo();
    info.cameraModelName    = "VirtualCamera";
    info.cameraID           = m_cameraDevInfo_.SN.empty() ? "VIRTUAL" : m_cameraDevInfo_.SN;
    info.imageWidth         = w;
    info.imageHeight        = h;
    info.bitDepth           = 8;
    info.imageFormat        = fmt;
    info.isSupportColor     = fmt == common::imgproc::ImageFormat::RGB24
                          || fmt == common::imgproc::ImageFormat::BGR24;
    info.pixelFormatType    = info.isSupportColor ? PixelFormatType::RGB : PixelFormatType::Mono;
    info.pixelFormat        = info.isSupportColor ? "RGB8" : "Mono8";
    cameraInfoInitialized_  = true;
    LOG_INFO("[VirtualCamera] camera info {}x{} format={}", w, h, static_cast<int>(fmt));
}

bool VirtualCamera::loadSyntheticFrame(std::vector<unsigned char> &out, int &w, int &h,
                                       common::imgproc::ImageFormat &fmt)
{
    w = syntheticWidth_;
    h = syntheticHeight_;
    cv::Mat img(h, w, CV_8UC3);
    const uint64_t frameNo = syntheticFrameNo_++;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.at<cv::Vec3b>(y, x) =
                cv::Vec3b(static_cast<uchar>((x + frameNo) % 256),
                          static_cast<uchar>((y + frameNo) % 256),
                          static_cast<uchar>((x + y + frameNo) % 256));
        }
    }
    fmt = common::imgproc::ImageFormat::RGB24;
    out.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 3);
    std::memcpy(out.data(), img.data, out.size());
    updateCameraInfoFromFrame(w, h, fmt);
    return true;
}

bool VirtualCamera::loadFromPath(const std::filesystem::path &path,
                                 std::vector<unsigned char> &out, int &w, int &h,
                                 common::imgproc::ImageFormat &fmt)
{
    cv::Mat loaded = cv::imread(path.string(), cv::IMREAD_UNCHANGED);
    if (loaded.empty()) {
        LOG_WARN("[VirtualCamera] failed to read image: {}", path.string());
        return false;
    }

    // 统一输出 8 位，且 fmt 与像素布局一致（MONO8=单通道，BGR24=8UC3），避免 ImageCapture 侧 AlgoMat 长度不匹配
    if (loaded.channels() == 1) {
        cv::Mat mono8;
        if (loaded.depth() == CV_16U) {
            loaded.convertTo(mono8, CV_8U, 255.0 / 65535.0);
        } else if (loaded.depth() == CV_8U) {
            mono8 = loaded;
        } else {
            loaded.convertTo(mono8, CV_8U);
        }
        if (!mono8.isContinuous()) {
            mono8 = mono8.clone();
        }
        fmt = common::imgproc::ImageFormat::MONO8;
        w   = mono8.cols;
        h   = mono8.rows;
        out.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
        std::memcpy(out.data(), mono8.data, out.size());
        updateCameraInfoFromFrame(w, h, fmt);
        return true;
    }

    cv::Mat bgr8;
    if (loaded.channels() == 3) {
        if (loaded.depth() == CV_16U) {
            cv::Mat tmp8;
            loaded.convertTo(tmp8, CV_8U, 255.0 / 65535.0);
            bgr8 = tmp8;
        } else if (loaded.depth() == CV_8U) {
            bgr8 = loaded;
        } else {
            loaded.convertTo(bgr8, CV_8U);
        }
    } else if (loaded.channels() == 4) {
        cv::Mat src8 = loaded;
        if (loaded.depth() == CV_16U) {
            loaded.convertTo(src8, CV_8U, 255.0 / 65535.0);
        } else if (loaded.depth() != CV_8U) {
            loaded.convertTo(src8, CV_8U);
        }
        cv::cvtColor(src8, bgr8, cv::COLOR_BGRA2BGR);
    } else {
        return false;
    }

    if (!bgr8.isContinuous()) {
        bgr8 = bgr8.clone();
    }
    fmt = common::imgproc::ImageFormat::BGR24;
    w   = bgr8.cols;
    h   = bgr8.rows;
    out.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 3u);
    std::memcpy(out.data(), bgr8.data, out.size());
    updateCameraInfoFromFrame(w, h, fmt);
    return true;
}

bool VirtualCamera::loadRawFromPath(const std::filesystem::path &path,
                                    std::vector<unsigned char> &out, int &w, int &h,
                                    common::imgproc::ImageFormat &fmt)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        LOG_WARN("[VirtualCamera] failed to open raw file: {}", path.string());
        return false;
    }

    ifs.seekg(0, std::ios::end);
    const std::streamoff fileSize = ifs.tellg();
    if (fileSize <= 0) {
        LOG_WARN("[VirtualCamera] raw file is empty: {}", path.string());
        return false;
    }
    ifs.seekg(0, std::ios::beg);

    out.resize(static_cast<size_t>(fileSize));
    if (!ifs.read(reinterpret_cast<char *>(out.data()), fileSize)) {
        LOG_WARN("[VirtualCamera] failed to read raw file: {}", path.string());
        out.clear();
        return false;
    }

    w   = syntheticWidth_;
    h   = syntheticHeight_;
    fmt = GetCameraInfo().imageFormat;
    if (fmt == common::imgproc::ImageFormat::UNKNOWN) {
        LOG_WARN("[VirtualCamera] CameraInfo.imageFormat unknown for raw, fallback BAYER_RG8");
        fmt = common::imgproc::ImageFormat::BAYER_RG8;
    }

    LOG_INFO("[VirtualCamera] loaded raw {} ({} bytes, {}x{}, fmt={})", path.filename().string(),
             fileSize, w, h, static_cast<int>(fmt));
    return true;
}

bool VirtualCamera::loadMediaFromPath(const std::filesystem::path &path,
                                      std::vector<unsigned char> &out, int &w, int &h,
                                      common::imgproc::ImageFormat &fmt)
{
    if (isRawFile(path)) {
        return loadRawFromPath(path, out, w, h, fmt);
    }
    return loadFromPath(path, out, w, h, fmt);
}

bool VirtualCamera::loadFrameAt(size_t index, std::vector<unsigned char> &out, int &w, int &h,
                                common::imgproc::ImageFormat &fmt)
{
    if (useSyntheticOnly_) {
        return loadSyntheticFrame(out, w, h, fmt);
    }

    if (playlistSequence_.empty()) {
        return loadSyntheticFrame(out, w, h, fmt);
    }

    if (const CachedFrame *cached = getCachedFrame(index)) {
        w   = cached->width;
        h   = cached->height;
        fmt = cached->format;
        updateCameraInfoFromFrame(w, h, fmt);
        out = cached->pixels;
        return true;
    }

    const auto &path = playlistSequence_[index % playlistSequence_.size()];
    if (!loadMediaFromPath(path, out, w, h, fmt)) {
        return loadSyntheticFrame(out, w, h, fmt);
    }
    return true;
}

void VirtualCamera::emitGrabFrame(int w, int h, const unsigned char *data, int len,
                                  common::imgproc::ImageFormat fmt)
{
    if (!imageGrabFuncCB_ || !data || len <= 0) {
        return;
    }
    // 同步 GrabCB：data 须在回调返回前保持有效（软触发可直传 playlist 缓存指针）
    imageGrabFuncCB_(h, w, const_cast<unsigned char *>(data), len, fmt);
}

void VirtualCamera::softwareTriggerGrabAndEmit()
{
    const size_t      idx    = nextSequentialIndex();
    const std::string detail = m_cameraDevInfo_.SN.empty() ? "virtual" : m_cameraDevInfo_.SN;

    if (const CachedFrame *cached = getCachedFrame(idx)) {
        {
            common::utility::TimingDebugScope cacheTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "虚拟相机.缓存命中",
                detail + " " + std::to_string(cached->width) + "x"
                    + std::to_string(cached->height));
        }
        {
            common::utility::TimingDebugScope emitTiming(
                common::utility::TimingDebugKeys::CaptureTiming, "虚拟相机.回调派发", detail);
            emitGrabFrame(cached->width, cached->height, cached->pixels.data(),
                          static_cast<int>(cached->pixels.size()), cached->format);
        }
        return;
    }

    std::vector<unsigned char>   buffer;
    int                          w   = 0;
    int                          h   = 0;
    common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
    {
        common::utility::TimingDebugScope readTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "虚拟相机.读盘解码", detail);
        if (!loadFrameAt(idx, buffer, w, h, fmt)) {
            return;
        }
    }

    common::utility::TimingDebugScope emitTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "虚拟相机.回调派发", detail);
    emitGrabFrame(w, h, buffer.data(), static_cast<int>(buffer.size()), fmt);
}

void VirtualCamera::refreshCameraInfo()
{
    int                            w = 0;
    int                            h = 0;
    common::imgproc::ImageFormat   fmt = common::imgproc::ImageFormat::UNKNOWN;
    std::vector<unsigned char>     tmp;
    if (loadFrameAt(0, tmp, w, h, fmt)) {
        updateCameraInfoFromFrame(w, h, fmt);
    }
}

ErrorPtr VirtualCamera::setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg)
{
    (void)featureType;
    (void)arg;
    return Ok();
}

ErrorPtr VirtualCamera::getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg)
{
    (void)featureType;
    (void)arg;
    return Ok();
}

ErrorPtr VirtualCamera::Open()
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        loadConfig();
        scanPlaylist();
        sequentialIndex_       = 0;
        hardSubdirIndex_       = 0;
        syntheticFrameNo_      = 0;
        cameraInfoInitialized_ = false;
        stopWorkers_           = false;
        SetConnectedState(true);
        SetCaptureState(CaptureState::Idle);

        int                          w = 0;
        int                          h = 0;
        common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
        std::vector<unsigned char>   tmp;
        loadFrameAt(0, tmp, w, h, fmt);
    }
    startPlaylistPreloadAsync();
    return Ok();
}

ErrorPtr VirtualCamera::Close()
{
    joinPlaylistPreload();
    std::lock_guard<std::mutex> lock(mtx_);
    (void)stopGrabImpl(true);
    playlistSequence_.clear();
    clearPlaylistCache();
    SetConnectedState(false);
    SetCaptureState(CaptureState::Idle);
    return Ok();
}

bool VirtualCamera::IsConnected()
{
    return isConnected_;
}

ErrorPtr VirtualCamera::stopGrabImpl(bool forceStop)
{
    if (!forceStop && syncGrabInProgress_.load(std::memory_order_acquire)) {
        return Err("sync grab in progress", common::errors::camera_module::kAlreadyStarted);
    }

    stopWorkers_.store(true);

    if (asyncThread_.joinable()) {
        asyncThread_.join();
    }
    if (hardBurstThread_.joinable()) {
        hardBurstThread_.join();
    }

    stopWorkers_.store(false);
    isGrabbing_.store(false);
    asyncFlag_.store(false);
    SetCaptureState(CaptureState::Idle);
    return Ok();
}

void VirtualCamera::asyncWorkerLoop()
{
    const int intervalMs = std::max(1, 1000 / std::max(1, frameRateFps_));
    while (!stopWorkers_.load(std::memory_order_acquire) && isGrabbing_.load()) {
        const auto tickStart = std::chrono::steady_clock::now();

        size_t idx = 0;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            idx = nextSequentialIndex();
        }

        if (const CachedFrame *cached = getCachedFrame(idx)) {
            emitGrabFrame(cached->width, cached->height, cached->pixels.data(),
                          static_cast<int>(cached->pixels.size()), cached->format);
        } else {
            std::vector<unsigned char>   buffer;
            int                          w = 0;
            int                          h = 0;
            common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
            {
                std::lock_guard<std::mutex> lock(mtx_);
                if (!loadFrameAt(idx, buffer, w, h, fmt)) {
                    continue;
                }
            }
            emitGrabFrame(w, h, buffer.data(), static_cast<int>(buffer.size()), fmt);
        }

        const auto elapsed = std::chrono::steady_clock::now() - tickStart;
        const auto sleepMs =
            intervalMs - static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                              elapsed)
                                              .count());
        if (sleepMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        }
    }
}

void VirtualCamera::hardBurstWorker(std::vector<std::filesystem::path> group)
{
    if (group.empty()) {
        group = currentHardTriggerGroup();
    }
    if (group.empty()) {
        std::vector<unsigned char>   buffer;
        int                          w = 0;
        int                          h = 0;
        common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
        if (loadSyntheticFrame(buffer, w, h, fmt)) {
            emitGrabFrame(w, h, buffer.data(), static_cast<int>(buffer.size()), fmt);
        }
        return;
    }

    const int intervalMs = std::max(1, 1000 / std::max(1, frameRateFps_));
    for (const auto &path : group) {
        if (stopWorkers_.load(std::memory_order_acquire)) {
            break;
        }
        std::vector<unsigned char>   buffer;
        int                          w = 0;
        int                          h = 0;
        common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
        if (const CachedFrame *cached = getCachedFrameByPath(path)) {
            emitGrabFrame(cached->width, cached->height, cached->pixels.data(),
                          static_cast<int>(cached->pixels.size()), cached->format);
        } else if (loadMediaFromPath(path, buffer, w, h, fmt)) {
            emitGrabFrame(w, h, buffer.data(), static_cast<int>(buffer.size()), fmt);
        } else {
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
}

void VirtualCamera::startHardBurst(const std::vector<std::filesystem::path> &group)
{
    if (hardBurstThread_.joinable()) {
        stopWorkers_.store(true);
        hardBurstThread_.join();
        stopWorkers_.store(false);
    }
    hardBurstThread_ = std::thread(&VirtualCamera::hardBurstWorker, this, group);
}

ErrorPtr VirtualCamera::SyncGrab(int &height, int &width, unsigned char *&dataBuffer, int &dataLen)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (!isConnected_) {
        return Err("camera not opened", common::errors::camera_module::kNotOpened);
    }
    if (isGrabbing_.load() && !syncGrabInProgress_.load()) {
        if (auto err = stopGrabImpl(true); err) {
            return err;
        }
    }
    if (syncGrabInProgress_.exchange(true)) {
        return Err("sync grab already running", common::errors::camera_module::kAlreadyStarted);
    }

    struct ResetFlag {
        std::atomic<bool> &flag;
        ~ResetFlag() { flag.store(false); }
    } resetFlag{syncGrabInProgress_};

    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }

    const size_t idx = nextSequentialIndex();
    common::imgproc::ImageFormat fmt = common::imgproc::ImageFormat::UNKNOWN;
    if (!loadFrameAt(idx, syncBuffer_, width, height, fmt)) {
        HandleError("VirtualCamera SyncGrab load failed", common::errors::camera_module::kFrameTimeout);
        return Err("load frame failed", common::errors::camera_module::kFrameTimeout);
    }

    dataBuffer = syncBuffer_.data();
    dataLen    = static_cast<int>(syncBuffer_.size());
    SetCaptureState(CaptureState::Idle);
    return Ok();
}

ErrorPtr VirtualCamera::AsyncGrab()
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (!isConnected_) {
        return Err("camera not opened", common::errors::camera_module::kNotOpened);
    }

    if (isGrabbing_.load()) {
        if (auto err = stopGrabImpl(true); err) {
            return err;
        }
    }
    if (auto err = ensureFreeRunMode(); err) {
        return err;
    }

    asyncFlag_.store(true);
    isGrabbing_.store(true);
    stopWorkers_.store(false);
    asyncThread_ = std::thread(&VirtualCamera::asyncWorkerLoop, this);
    SetCaptureState(CaptureState::Preview);
    return Ok();
}

ErrorPtr VirtualCamera::TriggerCapture(CameraTriggerSourceType source)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (!isConnected_) {
        return Err("camera not opened", common::errors::camera_module::kNotOpened);
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
            softwareTriggerGrabAndEmit();
        } else if (isHardTrigger(source)) {
            startHardBurst(currentHardTriggerGroup());
            if (!subdirGroups_.empty()) {
                hardSubdirIndex_ = (hardSubdirIndex_ + 1) % subdirGroups_.size();
            }
        }
        return Ok();
    }

    if (isGrabbing_.load() && !asyncFlag_.load(std::memory_order_acquire)) {
        return Err("sync grab in progress", common::errors::camera_module::kAlreadyStarted);
    }

    if (auto err = ensureTriggerMode(source); err) {
        return err;
    }

    asyncFlag_.store(true);
    isGrabbing_.store(true);
    stopWorkers_.store(false);

    if (m_triggerSourceType_ == CameraTriggerSourceType::SOFTWARE_TRIGGER) {
        softwareTriggerGrabAndEmit();
    } else if (isHardTrigger(source)) {
        startHardBurst(currentHardTriggerGroup());
        if (!subdirGroups_.empty()) {
            hardSubdirIndex_ = (hardSubdirIndex_ + 1) % subdirGroups_.size();
        }
    }

    SetCaptureState(CaptureState::Preview);
    return Ok();
}

ErrorPtr VirtualCamera::StopGrab()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return stopGrabImpl(false);
}

} // namespace device
} // namespace common
