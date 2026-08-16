/**
 * @file meter_algorithm.cpp
 * @brief CISDK Meter 算法封装实现。
 *
 * 封装 CISDK LED 亮度测量接口，从 Shot 定位 LED 轮廓并提取亮度数据。
 * 支持轮廓复用、自动/手动 ROI、侧视透视校正，以及可选的轮廓持久化。
 */

#include "meter_algorithm.h"
#include <common/utility/defer/include/Defer.h>

namespace common {
namespace algorithm {

namespace {

// 判断 PointMapper 模块的错误是否可恢复（可回退到已保存的轮廓文件）。
// 可恢复错误码：2, 102, 104, 105, 106, 113。
inline bool IsRecoverablePointMapError(AlgoError err) {
    if (ERROR_MODULE_NAME(err) != ALGOMOD_POINT_MAPPER) {
        return false;
    }
    const int code = ERROR_CODE_NO_MODULE(err);
    return code == 104 || code == 105 || code == 106 || code == 113 || code == 102 || code == 2;
}

} // namespace

// ---------------------------------------------------------------------------
// MeterHandle 生命周期管理
// ---------------------------------------------------------------------------

// 析构时自动释放持有的 Meter 或 Contour 句柄。
MeterHandle::~MeterHandle() {
    reset();
}

// 移动构造：转移句柄所有权，源对象置空。
MeterHandle::MeterHandle(MeterHandle&& other) noexcept : handle_(other.handle_) {
    kind_ = other.kind_;
    other.handle_ = nullptr;
    other.kind_ = Kind::Meter;
}

// 移动赋值：先释放自身资源，再接管源对象句柄。
MeterHandle& MeterHandle::operator=(MeterHandle&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    reset();
    handle_ = other.handle_;
    kind_ = other.kind_;
    other.handle_ = nullptr;
    other.kind_ = Kind::Meter;
    return *this;
}

// 创建 LedMeter 测量句柄，会先释放已有句柄。
AlgoError MeterHandle::create() {
    reset();
    kind_ = Kind::Meter;
    return LedMeterCreateHandle(&handle_);
}

// 创建 LedContour 轮廓搜索句柄，会先释放已有句柄。
AlgoError MeterHandle::createContour() {
    reset();
    kind_ = Kind::Contour;
    return LedContourCreateHandle(&handle_);
}

// 按 Kind 调用对应的 CISDK 销毁接口。
void MeterHandle::reset() {
    if (handle_ != nullptr) {
        if (kind_ == Kind::Contour) {
            LedContourDestroyHandle(handle_);
        } else {
            LedMeterDestroyHandle(handle_);
        }
        handle_ = nullptr;
        kind_ = Kind::Meter;
    }
}

// 转移句柄所有权给调用方，内部不再持有。
AlgoHandle MeterHandle::release() {
    AlgoHandle raw = handle_;
    handle_ = nullptr;
    return raw;
}

// ---------------------------------------------------------------------------
// MeterAlgorithm 公开接口
// ---------------------------------------------------------------------------

// 执行完整 Meter 处理流水线：轮廓准备 -> 亮度提取。
AlgoError MeterAlgorithm::run(const MeterRequest& request, MeterResult* out) const {
    // 入参快速失败。
    if (out == nullptr || request.shotHandle == nullptr) {
        return ALGO_INPUT_INVALID(ALGOMOD_ALGOLEDMETER);
    }

    out->meter.reset();
    out->contour.reset();

    // 步骤 1：创建轮廓句柄并准备 LED 轮廓数据。
    AlgoError err = out->contour.createContour();
    if (err != ALGO_PASS) {
        return err;
    }

    err = prepareContour(request, out->contour.get());
    if (err != ALGO_PASS) {
        return err;
    }

    // 步骤 2：创建 Meter 句柄并从 Shot + 轮廓提取亮度。
    err = out->meter.create();
    if (err != ALGO_PASS) {
        return err;
    }

    err = createMeter(request, out->contour.get(), out->meter.get());
    return err;
}

// 释放外部持有的裸 Meter 句柄指针。
void MeterAlgorithm::releaseMeter(AlgoHandle* meterHandle) {
    if (meterHandle == nullptr || *meterHandle == nullptr) {
        return;
    }
    LedMeterDestroyHandle(*meterHandle);
    *meterHandle = nullptr;
}

// ---------------------------------------------------------------------------
// MeterAlgorithm 内部流水线
// ---------------------------------------------------------------------------

// 准备 LED 轮廓：复用已有文件，或在线搜索；搜索失败时尝试回退加载。
AlgoError MeterAlgorithm::prepareContour(const MeterRequest& request, AlgoHandle contourHandle) const {
    if (request.contourReuse) {
        // 直接加载已保存的轮廓，跳过在线搜索。
        if (request.contourFileName.empty()) {
            return ALGO_INPUT_INVALID(ALGOMOD_LED_COUTOUR_SEARCHER);
        }
        return LedContourLoad(contourHandle, request.contourFileName.c_str());
    }

    // 在线搜索路径：创建临时 ROI 句柄执行轮廓定位。
    AlgoHandle roiHandle = nullptr;
    AlgoError err = ROICreateHandle(&roiHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    err = locateContour(request, roiHandle, contourHandle);
    ROIDestroyHandle(roiHandle);

    // 点映射失败时，若存在已保存的轮廓文件则回退加载。
    if (err != ALGO_PASS && IsRecoverablePointMapError(err) && !request.contourFileName.empty()) {
        return LedContourLoad(contourHandle, request.contourFileName.c_str());
    }
    return err;
}

// 创建 ROI 区域：自动定位或手动四角，侧视时追加透视校正与旋转。
AlgoError MeterAlgorithm::createRoi(const MeterRequest& request, AlgoHandle roiHandle) const {
    AlgoError err = ALGO_PASS;
    if (request.useAutoROI) {
        // 自动定位 ROI 区域。
        err = ROIAutoLocate(roiHandle, request.shotHandle);
    } else {
        // 使用四角坐标手动指定 ROI。
        err = ROISet4Corner(roiHandle, &request.imageSize, request.corner4, 4);
    }
    if (err != ALGO_PASS) {
        return err;
    }

    if (request.isSide) {
        // 侧视路径：透视校正后再旋转。
        err = ROIPerspectiveSet4Corner(roiHandle, request.corner4, 4);
        if (err != ALGO_PASS) {
            return err;
        }
        err = ROIWarpPerspective(roiHandle, request.shotHandle);
        if (err != ALGO_PASS) {
            return err;
        }
    }
    return ROIRotate(roiHandle, request.angle);
}

// 在线定位 LED 轮廓：ROI -> 点查找 -> 点映射 -> 轮廓搜索 -> 可选持久化。
AlgoError MeterAlgorithm::locateContour(const MeterRequest& request, AlgoHandle roiHandle, AlgoHandle contourHandle) const {
    AlgoError err = createRoi(request, roiHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    // 创建点查找与点映射句柄，函数退出时由 DEFER 自动销毁。
    AlgoHandle pointFindHandle = nullptr;
    AlgoHandle pointMapHandle = nullptr;
    err = PointFindCreateHandle(&pointFindHandle);
    if (err != ALGO_PASS) {
        return err;
    }
    DEFER_NAMED(point_find_defer, [&]() {
        if (pointFindHandle != nullptr) {
            PointFindDestroyHandle(pointFindHandle);
            pointFindHandle = nullptr;
        }
    });

    err = PointMapCreateHandle(&pointMapHandle);
    if (err != ALGO_PASS) {
        return err;
    }
    DEFER_NAMED(point_map_defer, [&]() {
        if (pointMapHandle != nullptr) {
            PointMapDestroyHandle(pointMapHandle);
            pointMapHandle = nullptr;
        }
    });

    // 可选：设置点查找降采样比例以加速搜索。
    if (request.downSampling > 0.f && request.downSampling < 1.f) {
        err = PointFindSetDefaultDownsampling(pointFindHandle, request.downSampling);
        if (err != ALGO_PASS) {
            return err;
        }
    }

    // 步骤 1：在 ROI 内定位 LED 灯点。
    err = PointFindLocate(pointFindHandle, request.shotHandle, roiHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    // 步骤 2：从 Shot 元信息获取面板布局与点亮区域，配置点映射参数。
    AlgoShot shotInfo;
    err = ShotGet(request.shotHandle, &shotInfo);
    if (err != ALGO_PASS) {
        return err;
    }

    err = PointMapSetMapInfo(pointMapHandle, shotInfo.panel->layout, shotInfo.pattern->lighted_led_wh);
    if (err != ALGO_PASS) {
        return err;
    }

    // 步骤 3：将点查找结果传入点映射器并执行映射。
    err = PointMapSetInputFromHandle(pointMapHandle, pointFindHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    err = PointMapMapping(pointMapHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    // 步骤 4：设置轮廓边框收缩系数，侧视时按旋转角度补偿宽高比。
    float shrinkW = request.shrinkW;
    float shrinkH = request.shrinkH;
    if (request.isSide) {
        if (request.angle == 0 || request.angle == 180) {
            shrinkW = 1.5f;
            shrinkH = 1.0f;
        } else if (request.angle == 90 || request.angle == 270) {
            shrinkW = 1.0f;
            shrinkH = 1.5f;
        }
    }
    err = LedContourSetBorderShrink(contourHandle, shrinkW, shrinkH);
    if (err != ALGO_PASS) {
        return err;
    }

    // 步骤 5：基于点映射结果搜索 LED 轮廓。
    err = LedContourSearch(contourHandle, request.shotHandle, pointMapHandle, ALGOMOD_POINT_MAPPER);
    if (err != ALGO_PASS) {
        return err;
    }

    // 可选：保存轮廓文件与可视化图像。
    if (request.saveContour && !request.contourFileName.empty()) {
        err = LedContourSave(contourHandle, request.contourFileName.c_str());
        if (err != ALGO_PASS) {
            return err;
        }
    }
    if (request.saveContourPicture && !request.contourPicturePath.empty()) {
        err = LedContourExportImage(contourHandle, request.contourPicturePath.c_str(), 1, 0, request.shotHandle);
    }
    return err;
}

// 从 Shot 和轮廓提取亮度数据，写入 Meter 句柄。
AlgoError MeterAlgorithm::createMeter(const MeterRequest& request, AlgoHandle contourHandle, AlgoHandle meterHandle) const {
    AlgoHandle lumHandle = nullptr;
    AlgoError err = LedLumCreateHandle(&lumHandle);
    if (err != ALGO_PASS) {
        return err;
    }

    // 提取各 LED 灯点的亮度值。
    err = LedLumExtract(lumHandle, request.shotHandle, contourHandle);
    if (err != ALGO_PASS) {
        LedLumDestroyHandle(lumHandle);
        return err;
    }

    // 将亮度数据追加到 Meter。
    int outRemain = 0;
    err = LedMeterAdd(meterHandle, request.shotHandle, nullptr, lumHandle, &outRemain);

    LedLumDestroyHandle(lumHandle);
    return err;
}

} // namespace algorithm
} // namespace common
