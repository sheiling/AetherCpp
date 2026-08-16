/**
 * @file image_capture.h
 * @brief 图像采集器：相机回调、预览 debayer、工步原图入队
 *
 * 职责概要：
 * - 绑定一台相机（及可选光源），注册 Grab 回调，将 SDK 缓冲转为 AlgoMat
 * - **预览链路**（execDispFuncCB=true）：回调 packRawFrame → pending；工作线程 convertForDisplay → DispCB
 * - **工步链路**（execDispFuncCB=false）：回调 packRawFrame 入队；getImage 后 makeRgbForProcess
 *
 * 线程模型：
 * | 线程           | 操作                                              |
 * |----------------|---------------------------------------------------|
 * | 相机/SDK 回调  | 帧深拷贝、预览入队或工步打包入队                    |
 * | 预览工作线程   | debayer、DispCB（m_previewMutex 保护 pending）    |
 * | 工步/调用方    | getImage 出队、makeRgbForProcess / convertForDisplay |
 *
 * 设计文档：plugins/aoi/docs/AOI 检测软件 设计方案.md（相机/采集相关章节）
 */
#ifndef IMAGE_CAPTURE_H
#define IMAGE_CAPTURE_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <common/3rdparty/cisdk/measurement/include/ci_utils.h>
#include <common/base/imgproc/image_type_map.h>
#include <common/core/device/camera/camera_base.h>
#include <common/core/device/imageCapture/algo_mat_shared.h>
#include <common/utility/json_tool/reflector.h>

namespace common {
namespace device {

class CameraBase;
class LamphouseBase;

/** 单次采集时的光源通道亮度（1-based 索引，与 LamphouseBase 通道号一致） */
struct CaptureLightData
{
    /** 支持的最大光源通道数（与设备配置上限对齐） */
    static constexpr int kMaxChannels = 6;

    /** 各通道亮度 0~255；下标 0 对应通道 1 */
    std::array<uint16_t, kMaxChannels> channels{};

    CaptureLightData() { channels.fill(100); }

    bool operator==(const CaptureLightData &other) const { return channels == other.channels; }
    bool operator!=(const CaptureLightData &other) const { return !(*this == other); }

    /** @param index 通道号 1..kMaxChannels，越界返回 0 */
    uint16_t channel(int index) const
    {
        if (index < 1 || index > kMaxChannels) {
            return 0;
        }
        return channels[static_cast<size_t>(index - 1)];
    }

    /** @param index 通道号 1..kMaxChannels，越界忽略 */
    void setChannel(int index, uint16_t value)
    {
        if (index >= 1 && index <= kMaxChannels) {
            channels[static_cast<size_t>(index - 1)] = value;
        }
    }

    void reflectFromJson(const json &j);
    void reflectToJson(json &j) const;
};

/** 一帧图像的元数据与像素缓冲（AlgoMat 堆所有权由 SharedAlgoMat 管理） */
struct ImageInfo
{
    /** Bayer 排列；已 debayer 或非 Bayer 时为 COLOR_BayerNone */
    BayerType                bayerType   = COLOR_BayerRG;
    /** 像素数据；预览/工步 debayer 后通常为 RGB24 AlgoMat */
    SharedAlgoMat            imageMat;
    /** 单调递增帧序号，预览与工步共用计数器（互斥路径下分别递增） */
    uint64_t                 frameIndex  = 0;
    /**
     * 像素语义格式：
     * - 预览链路 debayer 后为 RGB24
     * - 工步链路为相机原始格式（Bayer / BGR / Mono 等）
     */
    common::imgproc::ImageFormat imageFormat = common::imgproc::ImageFormat::UNKNOWN;
};

/** 预览帧上抛回调：name 为采集器名，imgInfo 已为 RGB24（或滤片 Mono 原样） */
using ImageDispFuncCB = std::function<void(std::string name, std::shared_ptr<ImageInfo> imgInfo)>;

/** 按名称解析 CameraBase（由 DeviceManager 注入） */
using CameraResolver    = std::function<std::shared_ptr<CameraBase>(const std::string &name)>;
/** 按名称解析 LamphouseBase（由 DeviceManager 注入） */
using LamphouseResolver = std::function<std::shared_ptr<LamphouseBase>(const std::string &name)>;

/**
 * @brief 单路图像采集：软触发/连续采集、预览分发、工步出队
 *
 * 生命周期：configure → setDeviceResolvers → RegisterDispCB（预览）
 *          → continuousCapture / captureImage → suppressFrameDispatch / stopCapture
 */
class ImageCapture
{
public:
    ImageCapture() = default;
    /** 停止预览工作线程并 join（析构安全） */
    ~ImageCapture();

    /** 设置采集器逻辑名、绑定的相机名与光源名（不参与 JSON 反射的 captureName） */
    void configure(std::string captureName, std::string cameraName, std::string lightName = {});
    void reflectFromJson(const json &j);
    void reflectToJson(json &j) const;
    /** 向相机注册 Grab 回调（幂等，m_grabCbMutex 保护） */
    bool ensureGrabCallbackRegistered();

    /**
     * @brief 软触发单帧
     * @param execDispFuncCB true=预览链路（DispCB）；false=工步入队
     */
    void captureImage(bool execDispFuncCB = true, const CaptureLightData &light = CaptureLightData());
    /**
     * @brief 连续采集（AsyncGrab）
     * @param execDispFuncCB 同 captureImage
     * @return AsyncGrab 是否成功
     */
    bool continuousCapture(bool execDispFuncCB = true, const CaptureLightData &light = CaptureLightData());
    /**
     * @brief 仅停止向上层分发帧（不 StopGrab）
     * @note 置 m_acceptingImages=false 并 stopPreviewWorker，用于页面切走快速释放 UI
     */
    void suppressFrameDispatch();
    /**
     * @brief 停止采集并可选等待回调排空
     * @param waitDrain true 时在 m_drainCV 上等待 kDrainQuietMs，避免 StopGrab 与在途回调竞态
     */
    bool stopCapture(bool waitDrain = true);
    /** 工步阻塞出队一帧原图（超时 m_delayTime ms 返回 nullptr） */
    std::shared_ptr<ImageInfo>            getImage();
    /** 非阻塞耗尽当前队列（循环 getImage 直到超时） */
    std::vector<std::shared_ptr<ImageInfo>> getAllTypeImage();

    /** 工步队列上限，超出时 pop_front 丢弃最旧帧 */
    void setMaxImageCount(int count);
    void clearImageDeque();
    void setExposureTime(double exposureTime);

    std::string captureName() const { return m_imageCaptureName_; }
    std::string getCameraName() const { return m_cameraName_; }
    std::string getLightName() const { return m_lightName_; }
    /** 关联运控轴名列表（配置/工艺用，采集逻辑不直接依赖） */
    const std::vector<std::string> &axisNames() const { return m_axisNames_; }
    CameraInfo cameraInfo() const;
    bool       isReady() const { return isCameraConnected(); }

    /** 注册预览 DispCB（Camera 插件内通常接 FrameDispatcher::publishFrame） */
    void RegisterDispCB(ImageDispFuncCB imageDispFuncCB) { m_imageDispFuncCB_ = std::move(imageDispFuncCB); }

    /**
     * @brief 将 packRawFrame 原图转为可显示/检测用的 RGB24（或滤片 Mono 透传）
     * @param raw 消耗 raw 内 Mat 所有权（detachAlgoMat，非 std::move POD 假移动）
     * @note 预览 worker 与工步 clone 后均走此入口；实现为 normalizeForDisplay
     */
    std::shared_ptr<ImageInfo> convertForDisplay(std::shared_ptr<ImageInfo> raw,
                                                 const CameraInfo &cameraInfo) const;

    /**
     * @brief 根据 ImageInfo.imageFormat + CameraInfo 判断是否需要 debayer / BGR→RGB
     * @note 预览与工步在转换前均可调用；滤片 Mono、已是 RGB24 的直出彩色返回 false
     */
    bool needsRgbConversion(const ImageInfo &frame, const CameraInfo &cameraInfo) const;

    /** 深拷贝 ImageInfo（AlgoMatCopy），供多消费者共享同帧原图时使用 */
    static std::shared_ptr<ImageInfo> cloneImageInfo(
        const std::shared_ptr<ImageInfo> &src);

    /**
     * @brief 工步单次 debayer：clone 原图后解 Bayer，不修改入参 raw
     * @note 预览/检测/存盘应共用此返回值，避免重复 debayer
     */
    std::shared_ptr<ImageInfo> makeRgbForProcess(
        const std::shared_ptr<ImageInfo> &raw) const;

    void setDeviceResolvers(CameraResolver cameraResolver, LamphouseResolver lampResolver);

private:
    std::shared_ptr<CameraBase>     resolveCamera() const;
    std::shared_ptr<LamphouseBase>  resolveLamp() const;
    bool                            isCameraConnected() const;

    AlgoMatType getAlgoMatType(common::imgproc::ImageFormat frameFormat, const CameraInfo &info) const;
    BayerType   getBayerType(common::imgproc::ImageFormat frameFormat, const CameraInfo &info) const;
    /**
     * @brief 预览/工步 debayer 统一入口：输出 RGB24（或滤片保留 Mono）
     * @param rawMat       相机回调深拷贝后的 AlgoMat（调用后释放）
     * @param frameFormat  回调 imageFormat；与 ImageInfo.imageFormat 语义一致
     * @param cameraInfo   回调时刻相机快照
     */
    std::shared_ptr<ImageInfo> normalizeForDisplay(AlgoMat &&rawMat,
                                                   common::imgproc::ImageFormat frameFormat,
                                                   const CameraInfo &cameraInfo) const;
    /** 基于 packRawFrame 产物做 RGB 转换（读取 imageFormat / bayerType） */
    std::shared_ptr<ImageInfo> normalizeForDisplay(std::shared_ptr<ImageInfo> raw,
                                                   const CameraInfo &cameraInfo) const;
    /**
     * @brief 相机回调统一入口：仅包装元数据（imageFormat、bayerType），不做 debayer
     * @note 预览 pending 与工步队列均先经此函数
     */
    std::shared_ptr<ImageInfo> packRawFrame(AlgoMat &&rawMat,
                                            common::imgproc::ImageFormat frameFormat,
                                            const CameraInfo &cameraInfo) const;
    void setLight(const CaptureLightData &light);
    /** 相机 SDK Grab 回调入口（在 SDK 线程执行） */
    void imageCallback(int height,
                       int width,
                       unsigned char *dataBuffer,
                       int dataLen,
                       common::imgproc::ImageFormat imageFormat);
    /** 重置 m_frameCounter；调用方须已持有 m_imageDequeMutex 或 m_previewMutex（预览路径） */
    void resetFrameCounterLocked();

    /**
     * @brief 预览 raw 入队包（与工步 ImageInfo 同构，均经 packRawFrame）
     * @note 相机回调仅深拷贝 + packRawFrame → 覆盖 pending；debayer 在 previewWorkerLoop
     */
    struct PreviewRawPacket
    {
        std::shared_ptr<ImageInfo>   rawFrame;    /**< packRawFrame 产物，保留原始 imageFormat */
        CameraInfo                     cameraInfo;  /**< 回调时刻快照，worker 不再跨线程读相机 */
        std::string                    frameDetail; /**< 耗时日志：分辨率/大小/matType */
    };

    /** 懒启动预览工作线程（首帧预览入队时，m_previewMutex 内创建） */
    void ensurePreviewWorkerStarted();
    /** 置停止标志、清空 pending、notify 并 join 工作线程 */
    void stopPreviewWorker();
    /** 预览工作线程：合并 latest raw → debayer → 必 DispCB；合并丢弃的是 raw 而非 RGB 结果 */
    void previewWorkerLoop();

    // ---- 配置与设备解析 ----
    std::string              m_imageCaptureName_; /**< 采集器逻辑名（DispCB / 日志） */
    std::string              m_cameraName_;       /**< 绑定的相机设备名 */
    std::string              m_lightName_;        /**< 绑定的光源设备名，可为空 */
    std::vector<std::string> m_axisNames_;        /**< 关联轴名（JSON axisNames） */
    int                      m_delayTime     = 1000;  /**< getImage 等待超时（ms），JSON 最小 1000 */
    int                      m_maxImageCount = 1000;    /**< 工步队列 m_imageDeque 容量上限 */

    // ---- 工步原图队列（execDispFuncCB=false）----
    std::mutex              m_imageDequeMutex;   /**< 保护 m_imageDeque / m_frameCounter 重置 */
    std::condition_variable m_imageDequeCV;      /**< getImage 阻塞等待非空队列 */
    std::deque<std::shared_ptr<ImageInfo>> m_imageDeque; /**< FIFO 原图队列 */
    uint64_t                m_frameCounter = 0;  /**< 帧序号；工步在回调递增，预览在工作线程递增 */

    // ---- 采集启停与回调排空 ----
    std::mutex              m_drainMutex;        /**< stopCapture waitDrain 专用 */
    std::condition_variable m_drainCV;           /**< imageCallback 在 !m_acceptingImages 时 notify */
    std::atomic<bool>       m_acceptingImages{false}; /**< true 时处理回调，false 时仅 notify drain */

    // ---- Grab 回调注册 ----
    std::mutex              m_grabCbMutex;       /**< 保护 m_grabCbRegistered 与 RegisterGrabCB */
    bool                    m_grabCbRegistered = false;

    // ---- 光源 / 预览 DispCB / 链路模式 ----
    CaptureLightData m_defaultCaptureLight_;     /**< 默认通道亮度，JSON lightChannelData */
    ImageDispFuncCB  m_imageDispFuncCB_ = nullptr; /**< 预览 RGB 帧上抛（可为空） */
    bool             m_execDispFuncCB   = false;   /**< 当前采集是否为预览链路（由 capture/continuous 设置） */

    CameraResolver    m_cameraResolver_;         /**< DeviceManager 注入的相机查找 */
    LamphouseResolver m_lampResolver_;           /**< DeviceManager 注入的光源查找 */

    // ---- 预览 debayer 工作线程（execDispFuncCB=true）----
    std::mutex              m_previewMutex;      /**< 保护 pending、frameCounter 递增、线程创建 */
    std::condition_variable m_previewCV;         /**< pending 入队或 stop 时唤醒工作线程 */
    std::optional<PreviewRawPacket> m_pendingPreviewRaw; /**< latest-only 待 debayer 的 raw（覆盖合并） */
    std::atomic<bool>       m_previewWorkerStop{true};   /**< true=工作线程应退出或尚未启动 */
    std::thread             m_previewWorkerThread;       /**< debayer + DispCB 专用线程 */
};

} // namespace device
} // namespace common

#endif // IMAGE_CAPTURE_H
