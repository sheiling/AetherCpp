#pragma once

#include "../camera_base.h"
#include "virtual_camera_global.h"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

namespace common {
namespace device {

/**
 * @brief 虚拟相机：从目录读图或自生成图，模拟 SyncGrab / AsyncGrab / TriggerCapture。
 */
class VIRTUAL_CAMERA_EXPORT VirtualCamera : public CameraBase
{
public:
    VirtualCamera()  = default;
    ~VirtualCamera() override;

    ErrorPtr Open() override;
    ErrorPtr Close() override;
    bool     IsConnected() override;

    void RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB) override
    {
        imageGrabFuncCB_ = std::move(imageGrabFuncCB);
    }
    void RegisterOfflineCB(CameraOfflineFuncCB cameraOfflineFuncCB) override
    {
        cameraOfflineFuncCB_ = std::move(cameraOfflineFuncCB);
    }

    ErrorPtr SyncGrab(int &height, int &width, unsigned char *&dataBuffer, int &dataLen) override;
    ErrorPtr AsyncGrab() override;
    ErrorPtr TriggerCapture(
        CameraTriggerSourceType source = CameraTriggerSourceType::SOFTWARE_TRIGGER) override;
    ErrorPtr StopGrab() override;

    void reflectFromJson(const json &j) override;
    void reflectToJson(json &j) const override;

protected:
    void refreshCameraInfo() override;
    ErrorPtr setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg) override;
    ErrorPtr getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg) override;

private:
    void   loadConfig();
    void   scanPlaylist();
    void   preloadPlaylistCache(const std::vector<std::filesystem::path> &sequence);
    void   startPlaylistPreloadAsync();
    void   joinPlaylistPreload();
    void   clearPlaylistCache();
    bool   isImageFile(const std::filesystem::path &path) const;
    bool   isRawFile(const std::filesystem::path &path) const;
    void   buildPlaylistSequence();
    std::vector<std::filesystem::path> currentHardTriggerGroup() const;

    struct CachedFrame {
        std::vector<unsigned char>   pixels;
        int                          width{0};
        int                          height{0};
        common::imgproc::ImageFormat format{common::imgproc::ImageFormat::UNKNOWN};
        bool                         valid{false};
    };

    bool loadFrameAt(size_t index, std::vector<unsigned char> &out, int &w, int &h,
                     common::imgproc::ImageFormat &fmt);
    /** 预加载缓存只读视图；调用方须保证缓存生命周期覆盖同步 GrabCB */
    const CachedFrame *getCachedFrame(size_t index) const;
    const CachedFrame *getCachedFrameByPath(const std::filesystem::path &path) const;
    bool        loadSyntheticFrame(std::vector<unsigned char> &out, int &w, int &h,
                                   common::imgproc::ImageFormat &fmt);
    bool        loadFromPath(const std::filesystem::path &path, std::vector<unsigned char> &out,
                           int &w, int &h, common::imgproc::ImageFormat &fmt);
    /** 读取 .raw 文件：原样拷贝字节，宽高取自 virtual_debug.camera，格式取自 CameraInfo */
    bool        loadRawFromPath(const std::filesystem::path &path,
                                std::vector<unsigned char> &out, int &w, int &h,
                                common::imgproc::ImageFormat &fmt);
    /** raw / 解码图统一入口 */
    bool        loadMediaFromPath(const std::filesystem::path &path,
                                  std::vector<unsigned char> &out, int &w, int &h,
                                  common::imgproc::ImageFormat &fmt);
    void        updateCameraInfoFromFrame(int w, int h, common::imgproc::ImageFormat fmt);
    void        emitGrabFrame(int w, int h, const unsigned char *data, int len,
                              common::imgproc::ImageFormat fmt);
    /** 软触发：优先零拷贝命中 playlist 预加载缓存 */
    void        softwareTriggerGrabAndEmit();
    ErrorPtr    stopGrabImpl(bool forceStop = false);
    void        asyncWorkerLoop();
    void        hardBurstWorker(std::vector<std::filesystem::path> group);
    void        startHardBurst(const std::vector<std::filesystem::path> &group);
    size_t      nextSequentialIndex();

    ImageGrabFuncCB     imageGrabFuncCB_;
    CameraOfflineFuncCB cameraOfflineFuncCB_;

    mutable std::mutex              cacheMtx_;
    std::mutex                      mtx_;
    std::atomic<bool>       isGrabbing_{false};
    std::atomic<bool>       asyncFlag_{false};
    std::atomic<bool>       syncGrabInProgress_{false};
    std::atomic<bool>       stopWorkers_{false};

    std::string             pixelFormat_ = "BayerRG8";
    std::string             imageRootPath_;
    int                     frameRateFps_{10};
    /** 无图目录或读图失败时自生成帧的宽高（来自 virtual_debug.camera.width/height） */
    int                     syntheticWidth_{1920};
    int                     syntheticHeight_{1080};

    std::vector<std::filesystem::path>              flatImages_;
    std::vector<std::vector<std::filesystem::path>> subdirGroups_;
    std::vector<std::filesystem::path>              playlistSequence_;
    std::vector<CachedFrame>                          playlistCache_;
    size_t                                            sequentialIndex_{0};
    size_t                                            hardSubdirIndex_{0};
    uint64_t                                          syntheticFrameNo_{0};
    bool                                              cameraInfoInitialized_{false};
    bool                                              useSyntheticOnly_{true};

    std::vector<unsigned char> syncBuffer_;

    std::thread asyncThread_;
    std::thread hardBurstThread_;
    std::thread preloadThread_;
};

} // namespace device
} // namespace common
