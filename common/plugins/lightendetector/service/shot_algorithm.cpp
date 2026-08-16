/**
 * @file shot_algorithm.cpp
 * @brief CISDK Shot 算法封装实现。
 *
 * 封装 CISDK algo_shot 接口，将原始相机图像转换为可测量的 Shot 对象。
 * 支持彩色/单色相机、黑屏参考图与亮屏图像两条处理路径，以及可选的背景减除。
 */

#include "shot_algorithm.h"

namespace common {
namespace algorithm {

namespace {

// 透传 CISDK 错误码，便于在调用链中统一检查。
inline AlgoError CheckPass(AlgoError err) {
    return err;
}

} // namespace

// ---------------------------------------------------------------------------
// ShotHandle 生命周期管理
// ---------------------------------------------------------------------------

ShotHandle::~ShotHandle() {
    reset();
}

ShotHandle::ShotHandle(ShotHandle&& other) noexcept : handle_(other.handle_) {
    other.handle_ = nullptr;
}

ShotHandle& ShotHandle::operator=(ShotHandle&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    reset();
    handle_ = other.handle_;
    other.handle_ = nullptr;
    return *this;
}

AlgoError ShotHandle::create() {
    reset();
    return ShotCreateHandle(&handle_);
}

void ShotHandle::reset() {
    if (handle_ != nullptr) {
        ShotDestroyHandle(handle_);
        handle_ = nullptr;
    }
}

AlgoHandle ShotHandle::release() {
    AlgoHandle raw = handle_;
    handle_ = nullptr;
    return raw;
}

// ---------------------------------------------------------------------------
// ShotAlgorithm 公开接口
// ---------------------------------------------------------------------------

AlgoError ShotAlgorithm::run(const ShotRequest& request, ShotResult* out) const {
    // 入参快速失败，避免将无效数据传入下游 SDK 调用。
    if (out == nullptr || !request.image || request.image->Empty()) {
        return ALGO_INPUT_INVALID(ALGOMOD_ALGOSHOT);
    }

    ShotHandle shot;
    AlgoError err = createAndPrepareShot(request, &shot);
    if (err != ALGO_PASS) {
        return err;
    }

    err = applyMetaInfo(shot.get(), request);
    if (err != ALGO_PASS) {
        return err;
    }

    return splitChannels(shot.get(), request, out);
}

// ---------------------------------------------------------------------------
// ShotAlgorithm 内部流水线
// ---------------------------------------------------------------------------

AlgoError ShotAlgorithm::createAndPrepareShot(const ShotRequest& request, ShotHandle* shotHandle) const {
    AlgoError err = shotHandle->create();
    if (err != ALGO_PASS) {
        return err;
    }

    AlgoMatType matType = request.matType;
    CIEType cieType = request.cieType;
    BayerType bayerType = request.bayerType;

    if (!request.isColorCamera) {
        // 单色相机路径：强制灰度输入，并根据目标颜色推导 CIE 通道。
        matType = ALGO_8UC1;
        bayerType = COLOR_BayerNone;
        switch (request.color) {
        case COLOR_RED:
            cieType = CAM_R;
            break;
        case COLOR_GREEN:
            cieType = CAM_G;
            break;
        case COLOR_BLUE:
            cieType = CAM_B;
            break;
        default:
            cieType = CIE_UNDEFINED;
            break;
        }
    }

    const int height = request.image->height;
    const int width = request.image->width;
    const char* imageData = reinterpret_cast<const char*>(request.image->data.get());
    if (height <= 0 || width <= 0 || imageData == nullptr) {
        return ALGO_INPUT_INVALID(ALGOMOD_ALGOSHOT);
    }

    if (request.color == COLOR_BLACK) {
        // 黑屏图像作为背景/参考图，不做去马赛克和异常像素修复。
        err = CheckPass(ShotCreateFromMem(
            shotHandle->get(),
            imageData,
            height,
            width,
            matType,
            cieType,
            bayerType));
    } else {
        // 亮屏路径：先解码图像，再执行异常像素修复与去马赛克。
        err = CheckPass(ShotCreateFromMem(
            shotHandle->get(),
            imageData,
            height,
            width,
            matType,
            CIE_UNDEFINED,
            COLOR_BayerNone));
        if (err == ALGO_PASS) {
            err = CheckPass(ShotFixAbnormalPixels(shotHandle->get(), request.fixingDefectiveThreshold));
        }
        if (err == ALGO_PASS) {
            err = CheckPass(ShotDebayer(shotHandle->get(), cieType, bayerType));
        }
    }
    if (err != ALGO_PASS) {
        return err;
    }

    // 可选：将预处理后的中间图像导出到磁盘。
    if (request.saveImage && !request.imageFileName.empty()) {
        err = CheckPass(ShotExportImage(shotHandle->get(), request.imageFileName.c_str(), 1));
    }
    return err;
}

AlgoError ShotAlgorithm::applyMetaInfo(AlgoHandle shotHandle, const ShotRequest& request) const {
    Device device = request.device;
    if (!request.isColorCamera) {
        device.cam_type = CAM_8_BIT_DEPTH;
    }

    // 元信息写入顺序与历史流程保持一致，确保行为稳定。
    AlgoError err = CheckPass(ShotSetDevice(shotHandle, &device));
    if (err != ALGO_PASS) {
        return err;
    }
    err = CheckPass(ShotSetCapture(shotHandle, &request.capture));
    if (err != ALGO_PASS) {
        return err;
    }
    err = CheckPass(ShotSetPattern(shotHandle, &request.pattern));
    if (err != ALGO_PASS) {
        return err;
    }
    err = CheckPass(ShotSetPartition(shotHandle, &request.part));
    if (err != ALGO_PASS) {
        return err;
    }
    err = CheckPass(ShotSetLedPanel(shotHandle, &request.panel));
    if (err != ALGO_PASS) {
        return err;
    }
    err = CheckPass(ShotSetColor(shotHandle, request.color));
    if (err != ALGO_PASS) {
        return err;
    }
    return CheckPass(ShotSetLayer(shotHandle, &request.layer));
}

AlgoError ShotAlgorithm::splitChannels(AlgoHandle shotHandle, const ShotRequest& request, ShotResult* out) const {
    out->original.reset();
    out->red.reset();
    out->green.reset();
    out->blue.reset();

    AlgoError err = out->red.create();
    if (err != ALGO_PASS) {
        return err;
    }
    err = out->green.create();
    if (err != ALGO_PASS) {
        return err;
    }
    err = out->blue.create();
    if (err != ALGO_PASS) {
        return err;
    }

    if (!request.isColorCamera) {
        // 单色输入：将同一通道复制到 RGB 占位，保持调用方 API 统一。
        err = CheckPass(ShotCopyTo(shotHandle, out->red.get()));
        if (err != ALGO_PASS) {
            return err;
        }
        err = CheckPass(ShotCopyTo(shotHandle, out->green.get()));
        if (err != ALGO_PASS) {
            return err;
        }
        return CheckPass(ShotCopyTo(shotHandle, out->blue.get()));
    }

    if (request.needOriginal) {
        // 保留完整 Shot 副本，供背景减除等需要原始数据的场景使用。
        err = out->original.create();
        if (err != ALGO_PASS) {
            return err;
        }
        return CheckPass(ShotCopyTo(shotHandle, out->original.get()));
    }

    // 彩色相机默认路径：拆分为 R/G/B 三通道。
    return CheckPass(ShotSplit(shotHandle, out->red.get(), out->green.get(), out->blue.get()));
}

} // namespace algorithm
} // namespace common
