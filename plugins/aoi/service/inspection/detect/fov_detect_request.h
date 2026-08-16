#pragma once

/**
 * @file fov_detect_request.h
 * @brief 单张 FOV 检测请求（产线 Planned / 任意图 AdHoc 统一入口）
 *
 * Planned：采图工步已知 fovIndex、grid、物理坐标，DL 组定位默认裁剪。
 * AdHoc：配方虚拟调测等对任意内存图送检，DL 组定位默认不裁剪整板预览。
 */

#include "data/dataModel/recipe_config_model.h"
#include "data/dataDefinition/inspection_types.h"

#include <common/core/device/imageCapture/image_capture.h>

#include <memory>

namespace aoi::inspection {

enum class FovDetectMode
{
    /** 产线扫描：FOV 索引与网格来自 FovCaptureRecord */
    Planned,
    /** 任意单张 RGB 图（虚拟调测 / 离线调试） */
    AdHoc,
};

/** 单张 FOV 检测运行时选项（与 FovCaptureRecord 解耦，便于 AdHoc 复用） */
struct FovDetectOptions
{
    int  fovIndex        = 1;
    int  gridRow         = 0;
    int  gridCol         = 0;
    bool useDeepLearning = true;
    /** Planned 默认 true；AdHoc 整板预览默认 false */
    bool dlCropLocate    = true;
};

/** InspectionDetectEngine::detectFov 统一入参 */
struct FovDetectRequest
{
    FovDetectMode mode = FovDetectMode::Planned;
    std::shared_ptr<common::device::ImageInfo> processImage;
    RecipeDocument                             recipe;
    FovDetectOptions                           options;
};

inline FovDetectRequest makePlannedFovRequest(const FovCaptureRecord &capture,
                                              const RecipeDocument &recipe)
{
    FovDetectRequest req;
    req.mode          = FovDetectMode::Planned;
    req.processImage  = capture.processImage;
    req.recipe        = recipe;
    req.options.fovIndex     = capture.fovIndex;
    req.options.gridRow      = capture.gridRow;
    req.options.gridCol      = capture.gridCol;
    req.options.dlCropLocate = true;
    return req;
}

inline FovDetectRequest makeAdHocFovRequest(std::shared_ptr<common::device::ImageInfo> processImage,
                                           const RecipeDocument &recipe, int fovIndex = 1)
{
    FovDetectRequest req;
    req.mode                 = FovDetectMode::AdHoc;
    req.processImage         = std::move(processImage);
    req.recipe               = recipe;
    req.options.fovIndex     = fovIndex;
    req.options.dlCropLocate = false;
    return req;
}

} // namespace aoi::inspection
