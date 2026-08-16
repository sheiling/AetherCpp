#pragma once

/**
 * @file mark_point_locator.h
 * @brief Mark 点模板匹配桥接（工步 / 配方共用）
 *
 * 图像约束：
 * - 入参 processImage 须为 debayer 后的 RGB24 ImageInfo（见 ImageCapture::makeRgbForProcess）
 * - 本模块及 algorithm/service/step/flow_host 禁止使用 QImage；UI 在 image_display 边界转换
 *
 * 调用链：
 *   工步：InspectionMarkerService → locate(processImage)
 *   配方：RecipeConfigMarkView（IGraphicsVew::GetImage → qImageToRgbImageInfo）→ locate(processImage)
 */

#include "recipe_config_model.h"

#include <vision_algo/core/service_access.h>

#include <common/core/device/imageCapture/image_capture.h>

#include <QString>

#include <memory>

namespace aoi {

/** Mark 模板匹配单次结果 */
struct MarkLocateResult
{
    bool    ok = false;   ///< 是否匹配成功
    QString errorKey;     ///< 失败时 AOI i18n 键（经 TR 展示）
    QString message;      ///< 失败时已翻译文案（非空时优先于 errorKey）
    double  pixelX = 0;   ///< 匹配中心像素 X
    double  pixelY = 0;   ///< 匹配中心像素 Y
    /** 定位结果预览（RGB24 ImageInfo 深拷贝；UI 侧 image_display::imageInfoToQImage 显示） */
    std::shared_ptr<common::device::ImageInfo> resultImage;
};

class MarkPointLocator
{
public:
    /** CISDK marker_detector + model_processor 是否均已加载 */
    static bool sdkReady();
    /** SDK 未就绪时的完整提示（已翻译；就绪时为空） */
    static QString sdkIssue();

    /**
     * @brief 对 debayer 后的 RGB 图执行 Mark 模板匹配
     * @param processImage  RGB24 ImageInfo，像素格式须与 CISDK MarkDetectParam.pixelFormat 一致
     * @param config        配方 Mark 页配置（模板路径、阈值、Mark 类型等）
     * @return 像素坐标与可选预览图；失败时 ok=false 且 errorKey 非空
     */
    static MarkLocateResult locate(const std::shared_ptr<common::device::ImageInfo> &processImage,
                                   const MarkPointConfig &config);
};

} // namespace aoi
