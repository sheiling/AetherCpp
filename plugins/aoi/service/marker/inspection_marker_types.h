#pragma once

/**
 * @file inspection_marker_types.h
 * @brief Marker 定位工步相关类型：槽位结果、运行时缓存约定
 */

#include "data/dataModel/recipe_config_model.h"

#include <common/core/device/imageCapture/image_capture.h>

#include <memory>

namespace aoi::inspection::marker {

/** 单轨/双轨下 Marker 槽位上限（与 InspectionSession 数组长度一致） */
constexpr int kMaxMarkerSlotCount = 4;

struct MarkerLocateSlotResult
{
    int slot = -1;
    aoi::Point locatedPhysical;
    bool templateMatched = false;
    bool valid           = false;
    /** debayer 后 RGB 预览（模板匹配叠加图或原采图） */
    std::shared_ptr<common::device::ImageInfo> previewImage;
};

aoi::Point computeMarkerPhysicalCenter(const RecipeDocument &recipe,
                                       const MarkPointConfig &marker,
                                       double pixelX,
                                       double pixelY);

} // namespace aoi::inspection::marker
