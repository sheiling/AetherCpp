#include "image/image_info_display.h"

/**
 * @file image_info_display.cpp
 * @brief UI 层 ImageInfo ↔ QImage 实现（见 image_info_display.h）
 */

#include <common/base/imgproc/image_type_map.h>
#include <common/core/device/imageCapture/algo_mat_shared.h>

#include <cstring>

namespace aoi::image_display {

QImage imageInfoToQImage(const std::shared_ptr<common::device::ImageInfo> &info,
                         int maxDisplayEdge)
{
    if (!info || !info->imageMat || AlgoMatIsEmpty(info->imageMat.get()))
        return {};
    if (info->imageFormat != common::imgproc::ImageFormat::RGB24)
        return {};

    const AlgoMat &mat = *info->imageMat;
    QImage         image(mat.cols, mat.rows, QImage::Format_RGB888);
    const int      rowBytes = mat.cols * 3;
    for (int y = 0; y < mat.rows; ++y) {
        std::memcpy(image.scanLine(y), mat.data + static_cast<size_t>(y) * rowBytes,
                    static_cast<size_t>(rowBytes));
    }

    if (maxDisplayEdge > 0) {
        const int edge = qMax(image.width(), image.height());
        if (edge > maxDisplayEdge) {
            image = image.scaled(maxDisplayEdge, maxDisplayEdge, Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        }
    }
    return image;
}

std::shared_ptr<common::device::ImageInfo> qImageToRgbImageInfo(const QImage &image)
{
    const QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    if (rgb.isNull())
        return nullptr;

    const int w = rgb.width();
    const int h = rgb.height();
    if (w <= 0 || h <= 0)
        return nullptr;

    // 须逐行拷贝：QImage::bytesPerLine 可能大于 width*3，不可对 constBits() 整块 CreateFromMem
    AlgoMat mat = AlgoMatCreate(h, w, ALGO_8UC3);
    if (AlgoMatIsEmpty(&mat))
        return nullptr;

    const int dstRowBytes = w * 3;
    for (int y = 0; y < h; ++y)
        std::memcpy(mat.data + static_cast<size_t>(y) * static_cast<size_t>(dstRowBytes),
                    rgb.constScanLine(y), static_cast<size_t>(dstRowBytes));

    auto out         = std::make_shared<common::device::ImageInfo>();
    out->imageMat    = common::device::adoptAlgoMat(std::move(mat));
    out->imageFormat = common::imgproc::ImageFormat::RGB24;
    out->bayerType   = COLOR_BayerNone;
    return out;
}

} // namespace aoi::image_display
