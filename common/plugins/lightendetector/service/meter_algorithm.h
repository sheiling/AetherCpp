#pragma once

/**
 * @file meter_algorithm.h
 * @brief CISDK Meter 算法封装声明。
 *
 * 提供 MeterHandle RAII、MeterRequest/MeterResult 数据结构及 MeterAlgorithm 流水线接口，
 * 从 Shot 定位 LED 轮廓并提取亮度测量数据。实现见 meter_algorithm.cpp。
 */

#include <common/3rdparty/cisdk/measurement/include/algo_led_meter.h>
#include <common/3rdparty/cisdk/measurement/include/algo_shot.h>
#include <common/3rdparty/cisdk/measurement/include/led_contour_searcher.h>
#include <common/3rdparty/cisdk/measurement/include/led_lum_extractor.h>
#include <common/3rdparty/cisdk/measurement/include/point_finder.h>
#include <common/3rdparty/cisdk/measurement/include/point_mapper.h>
#include <common/3rdparty/cisdk/measurement/include/roi_detector.h>

#include <string>

namespace common {
namespace algorithm {

// CISDK Meter / Contour 句柄的 RAII 封装。
// 根据 Kind 区分 LedMeter 与 LedContour 两种句柄的创建与销毁。
class MeterHandle final {
public:
    enum class Kind {
        Meter,   // LedMeter 亮度测量句柄
        Contour, // LedContour LED 轮廓搜索句柄
    };

    MeterHandle() = default;
    ~MeterHandle();

    MeterHandle(const MeterHandle&) = delete;
    MeterHandle& operator=(const MeterHandle&) = delete;
    MeterHandle(MeterHandle&& other) noexcept;
    MeterHandle& operator=(MeterHandle&& other) noexcept;

    // 创建 LedMeter 测量句柄。
    AlgoError create();
    // 创建 LedContour 轮廓搜索句柄。
    AlgoError createContour();
    // 释放当前持有的句柄。
    void reset();
    AlgoHandle get() const { return handle_; }
    // 转移句柄所有权给调用方，内部不再持有。
    AlgoHandle release();
    bool valid() const { return handle_ != nullptr; }

private:
    Kind kind_ = Kind::Meter;
    AlgoHandle handle_ = nullptr;
};

// Meter 处理入参，封装 Shot 句柄、ROI 定位参数及可选的持久化选项。
struct MeterRequest final {
    // 输入 Shot 句柄，由 ShotAlgorithm 产出。
    AlgoHandle shotHandle = nullptr;
    // 图像尺寸，手动 ROI 模式下使用。
    AlgoSize imageSize = {0, 0};
    // 四角坐标，手动 ROI 或侧视透视校正时使用。
    AlgoPoint corner4[4] = {};

    // 是否自动定位 ROI；false 时使用 corner4 手动指定。
    bool useAutoROI = true;
    // 是否为侧视拍摄，启用透视校正流程。
    bool isSide = false;
    // ROI 旋转角度（度）。
    int angle = 0;
    // 点查找降采样比例，范围 (0, 1)。
    float downSampling = 0.6f;
    // 轮廓边框水平/垂直收缩系数。
    float shrinkW = 1.0f;
    float shrinkH = 1.0f;

    // 是否复用已保存的轮廓文件，跳过在线搜索。
    bool contourReuse = false;
    // 是否将轮廓序列化保存到文件。
    bool saveContour = false;
    std::string contourFileName;
    // 是否将轮廓可视化图像导出到文件。
    bool saveContourPicture = false;
    std::string contourPicturePath;
};

// Meter 处理结果，包含亮度测量数据与 LED 轮廓信息。
struct MeterResult final {
    MeterHandle meter;
    MeterHandle contour;
};

// CISDK Meter 算法封装，从 Shot 生成 LED 亮度测量数据。
//
// 处理流程：
//   prepareContour -> createMeter
class MeterAlgorithm final {
public:
    // 执行完整 Meter 处理流水线。
    // request: 输入参数；out: 输出结果，不可为空。
    // 返回 ALGO_PASS 表示成功，否则返回 CISDK 错误码。
    AlgoError run(const MeterRequest& request, MeterResult* out) const;

    // 释放外部持有的 Meter 句柄（裸指针场景）。
    static void releaseMeter(AlgoHandle* meterHandle);

private:
    // 准备轮廓：复用已有文件或在线搜索 LED 轮廓。
    AlgoError prepareContour(const MeterRequest& request, AlgoHandle contourHandle) const;
    // 创建 ROI 区域（自动定位 / 手动四角 / 侧视透视校正）。
    AlgoError createRoi(const MeterRequest& request, AlgoHandle roiHandle) const;
    // 通过点查找与点映射定位 LED 轮廓。
    AlgoError locateContour(const MeterRequest& request, AlgoHandle roiHandle, AlgoHandle contourHandle) const;
    // 从 Shot 和轮廓提取亮度并写入 Meter。
    AlgoError createMeter(const MeterRequest& request, AlgoHandle contourHandle, AlgoHandle meterHandle) const;
};

} // namespace algorithm
} // namespace common
