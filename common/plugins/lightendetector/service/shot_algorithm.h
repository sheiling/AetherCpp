#pragma once

/**
 * @file shot_algorithm.h
 * @brief CISDK Shot 算法封装声明。
 *
 * 提供 ShotHandle RAII、ShotRequest/ShotResult 数据结构及 ShotAlgorithm 流水线接口，
 * 将原始相机图像转换为可测量的 Shot 对象。实现见 shot_algorithm.cpp。
 */

#include <common/base/algorithm/algorithm_packet.h>
#include <common/3rdparty/cisdk/measurement/include/algo_shot.h>

#include <string>

namespace common {
namespace algorithm {

// CISDK ShotHandle 的 RAII 封装。
// 在任意提前返回路径上自动释放句柄，避免资源泄漏。
class ShotHandle final {
public:
    ShotHandle() = default;
    ~ShotHandle();

    ShotHandle(const ShotHandle&) = delete;
    ShotHandle& operator=(const ShotHandle&) = delete;

    ShotHandle(ShotHandle&& other) noexcept;
    ShotHandle& operator=(ShotHandle&& other) noexcept;

    // 创建新的 CISDK Shot 句柄。
    AlgoError create();
    // 释放当前持有的句柄。
    void reset();
    // 转移句柄所有权给调用方，内部不再持有。
    AlgoHandle release();
    AlgoHandle get() const { return handle_; }
    bool valid() const { return handle_ != nullptr; }

private:
    AlgoHandle handle_ = nullptr;
};

// Shot 处理入参，封装图像、相机参数、LED 面板元信息及可选的持久化选项。
struct ShotRequest final {
    // 输入图像缓冲区。
    std::shared_ptr<common::algorithm::ImageBuffer> image;

    // 是否为彩色相机；false 时按单通道灰度输入处理并映射到目标颜色通道。
    bool isColorCamera = true;
    // 是否需要保留未拆分的原始 Shot（用于背景减除等后续处理）。
    bool needOriginal = false;
    // 异常像素修复阈值，仅亮屏图像路径生效。
    float fixingDefectiveThreshold = 3.0f;

    // 目标检测颜色。
    AlgoColor color = COLOR_BLACK;
    // 图像矩阵类型（如 16UC1）。
    AlgoMatType matType = ALGO_16UC1;
    // CIE 颜色空间类型。
    CIEType cieType = CAM_RGB;
    // Bayer 排列类型。
    BayerType bayerType = COLOR_BayerRG;

    // 屏幕分区信息。
    Partition part = {{0, 0, 0, 0}, {0, 0}};
    // LED 面板参数。
    LedPanel panel = {LED_PANEL_COB, LED_LAYOUT_RECTANGLE, LED_SUBPIXEL_UNDEFINED, {0, 0}, -1.f};
    // 采集设备信息。
    Device device = {"", CAM_MER_2000_5GC, "", LENS_FIXED_35MM};
    // 点亮图案配置。
    Pattern pattern = {PATTERN_GRID, {1, 1}, {0, 0}, {-1, -1}};
    // 曝光等采集参数。
    Capture capture = {6000, 200, 300, 1000, FILTER_XYZ};
    // 亮度层参数。
    Layer layer = {0, 0.2f, 255};

    // 是否将中间图像导出到文件。
    bool saveImage = false;
    std::string imageFileName;
};

// Shot 处理结果。
// needOriginal=true 时填充 original；否则仅返回 RGB 三通道结果。
struct ShotResult final {
    ShotHandle original;
    ShotHandle red;
    ShotHandle green;
    ShotHandle blue;
};

// CISDK Shot 算法封装，完成从原始图像到可测量 Shot 的完整流水线。
//
// 处理流程：
//   createAndPrepareShot -> applyMetaInfo -> splitChannels
class ShotAlgorithm final {
public:
    // 执行完整 Shot 处理流水线。
    // request: 输入参数；out: 输出结果，不可为空。
    // 返回 ALGO_PASS 表示成功，否则返回 CISDK 错误码。
    AlgoError run(const ShotRequest& request, ShotResult* out) const;

private:
    // 从内存创建 Shot 并执行解码、异常像素修复、去马赛克等预处理。
    AlgoError createAndPrepareShot(const ShotRequest& request, ShotHandle* shotHandle) const;
    // 写入设备、采集、面板等元信息，顺序与历史流程保持一致。
    AlgoError applyMetaInfo(AlgoHandle shotHandle, const ShotRequest& request) const;
    // 按相机类型拆分或复制 RGB 通道。
    AlgoError splitChannels(AlgoHandle shotHandle, const ShotRequest& request, ShotResult* out) const;
};

} // namespace algorithm
} // namespace common
