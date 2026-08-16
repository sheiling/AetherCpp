#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <common/base/algorithm/algorithm_packet.h>
#include <common/base/imgproc/imgproc_error_codes.h>
#include <common/base/imgproc/image_type_map.h>
#include <common/base/imgproc/imgproc_global.h>
#include <common/utility/errors/include/error.h>
#include <opencv2/core/mat.hpp>

namespace common::imgproc {

enum class EncodeFormat : int {
    JPG = 0,
    PNG,
    BMP
};

struct EncodeOptions {
    EncodeFormat format = EncodeFormat::JPG;
    int jpeg_quality = 95;      // [1, 100]
    int png_compression = 3;    // [0, 9]
};

struct EncodedBuffer {
    EncodeFormat format = EncodeFormat::JPG;
    std::shared_ptr<std::vector<uint8_t>> bytes;

    bool Empty() const noexcept { return !bytes || bytes->empty(); }
    std::size_t Size() const noexcept { return bytes ? bytes->size() : 0U; }
    const uint8_t* Data() const noexcept { return bytes && !bytes->empty() ? bytes->data() : nullptr; }
};

IMGPROC_EXPORT common::errors::ErrorPtr Debayer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    std::shared_ptr<common::algorithm::ImageBuffer>& dst);

IMGPROC_EXPORT common::errors::ErrorPtr Debayer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    cv::Mat& dst);

/** 直接对 cv::Mat Bayer 视图解马赛克，输出 BGR（与 ImageBuffer 版语义一致） */
IMGPROC_EXPORT common::errors::ErrorPtr Debayer(
    const cv::Mat& src,
    ImageFormat src_format,
    cv::Mat& dst);

IMGPROC_EXPORT common::errors::ErrorPtr EncodeToBuffer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    std::shared_ptr<EncodedBuffer>& out);

IMGPROC_EXPORT common::errors::ErrorPtr EncodeToBuffer(
    const cv::Mat& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    std::shared_ptr<EncodedBuffer>& out);

/** 将 12-bit 有效位深（存储于 uint16）的 cv::Mat 线性缩放到 8-bit，保留通道数，供预览/编码使用 */
IMGPROC_EXPORT common::errors::ErrorPtr ConvertMat12To8(
    const cv::Mat& src,
    cv::Mat& dst);

IMGPROC_EXPORT common::errors::ErrorPtr Rotate(
    const cv::Mat& src,
    int rotate_code,
    cv::Mat& dst);

}  // namespace common::imgproc
