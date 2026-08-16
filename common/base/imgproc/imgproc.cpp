#include <common/base/imgproc/imgproc.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace common::imgproc {
namespace {

[[maybe_unused]] const bool kImgprocCodersRegistered = common::errors::imgproc_module::RegisterImgprocCoders();

int Clamp(int value, int min_value, int max_value)
{
    return std::max(min_value, std::min(value, max_value));
}

common::errors::ErrorPtr ValidateImageBuffer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& image)
{
    if (!image || image->Empty()) {
        return common::errors::New(common::errors::imgproc_module::kInvalidImageBuffer, "image buffer is empty");
    }
    if (image->width <= 0 || image->height <= 0) {
        return common::errors::New(common::errors::imgproc_module::kInvalidImageBuffer, "image buffer size is invalid");
    }
    if (image->channels <= 0 || image->elem_size <= 0) {
        return common::errors::New(common::errors::imgproc_module::kInvalidImageBuffer, "image buffer layout is invalid");
    }
    const int min_step = image->width * image->channels * image->elem_size;
    if (image->step < min_step) {
        return common::errors::Newf(
            common::errors::imgproc_module::kInvalidImageBuffer,
            "image step is too small: %d < %d",
            image->step,
            min_step);
    }
    return nullptr;
}

common::errors::ErrorPtr ValidateMat(
    const cv::Mat& image)
{
    if (image.empty()) {
        return common::errors::New(common::errors::imgproc_module::kInvalidArgument, "cv::Mat is empty");
    }
    if (image.cols <= 0 || image.rows <= 0) {
        return common::errors::New(common::errors::imgproc_module::kInvalidArgument, "cv::Mat size is invalid");
    }
    return nullptr;
}

common::errors::ErrorPtr WrapMatView(
    const std::shared_ptr<common::algorithm::ImageBuffer>& image,
    int cv_type,
    cv::Mat& out)
{
    if (auto err = ValidateImageBuffer(image)) {
        return err;
    }
    out = cv::Mat(image->height, image->width, cv_type, image->data.get(), image->step);
    return nullptr;
}

int BayerToOpenCvCode(ImageFormat format)
{
    switch (format) {
    case ImageFormat::BAYER_BG8:
    case ImageFormat::BAYER_BG10:
    case ImageFormat::BAYER_BG12:
    case ImageFormat::BAYER_BG14:
    case ImageFormat::BAYER_BG16:
        return cv::COLOR_BayerBG2BGR;
    case ImageFormat::BAYER_RG8:
    case ImageFormat::BAYER_RG10:
    case ImageFormat::BAYER_RG12:
    case ImageFormat::BAYER_RG14:
    case ImageFormat::BAYER_RG16:
        return cv::COLOR_BayerRG2BGR;
    case ImageFormat::BAYER_GB8:
    case ImageFormat::BAYER_GB10:
    case ImageFormat::BAYER_GB12:
    case ImageFormat::BAYER_GB14:
    case ImageFormat::BAYER_GB16:
        return cv::COLOR_BayerGB2BGR;
    case ImageFormat::BAYER_GR8:
    case ImageFormat::BAYER_GR10:
    case ImageFormat::BAYER_GR12:
    case ImageFormat::BAYER_GR14:
    case ImageFormat::BAYER_GR16:
        return cv::COLOR_BayerGR2BGR;
    default:
        return -1;
    }
}

ImageFormat DebayerOutputFormat(ImageFormat format)
{
    return ImageFormatToPixelDepth(format) == PixelDepth::UINT16
        ? ImageFormat::BGR48
        : ImageFormat::BGR24;
}

std::string ExtensionOf(EncodeFormat format)
{
    switch (format) {
    case EncodeFormat::JPG: return ".jpg";
    case EncodeFormat::PNG: return ".png";
    case EncodeFormat::BMP: return ".bmp";
    default:                return ".jpg";
    }
}

std::vector<int> BuildEncodeParams(const EncodeOptions& options)
{
    switch (options.format) {
    case EncodeFormat::JPG:
        return {cv::IMWRITE_JPEG_QUALITY, Clamp(options.jpeg_quality, 1, 100)};
    case EncodeFormat::PNG:
        return {cv::IMWRITE_PNG_COMPRESSION, Clamp(options.png_compression, 0, 9)};
    case EncodeFormat::BMP:
    default:
        return {};
    }
}

common::errors::ErrorPtr MakeDebayeredMat(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    cv::Mat& out)
{
    const int bayer_code = BayerToOpenCvCode(src_format);
    if (bayer_code < 0) {
        return common::errors::New(common::errors::imgproc_module::kUnsupportedBayerFormat, "unsupported bayer format");
    }

    cv::Mat src_mat;
    if (auto err = WrapMatView(src, ImageFormatToType(src_format), src_mat)) {
        return err;
    }

    cv::cvtColor(src_mat, out, bayer_code);
    return nullptr;
}

common::errors::ErrorPtr MakeDebayeredMat(
    const cv::Mat& src,
    ImageFormat src_format,
    cv::Mat& out)
{
    const int bayer_code = BayerToOpenCvCode(src_format);
    if (bayer_code < 0) {
        return common::errors::New(common::errors::imgproc_module::kUnsupportedBayerFormat, "unsupported bayer format");
    }
    if (auto err = ValidateMat(src)) {
        return err;
    }
    cv::cvtColor(src, out, bayer_code);
    return nullptr;
}

common::errors::ErrorPtr MakeEncodableMat(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    cv::Mat& mat_view,
    cv::Mat& temp_mat)
{
    if (IsBayerFormat(src_format)) {
        if (auto err = MakeDebayeredMat(src, src_format, temp_mat)) {
            return err;
        }
        mat_view = temp_mat;
        return nullptr;
    }

    cv::Mat src_mat;
    if (auto err = WrapMatView(src, ImageFormatToType(src_format), src_mat)) {
        return err;
    }

    switch (src_format) {
    case ImageFormat::GRAY8:
    case ImageFormat::GRAY16:
    case ImageFormat::MONO8:
    case ImageFormat::MONO10:
    case ImageFormat::MONO12:
    case ImageFormat::MONO14:
    case ImageFormat::MONO16:
    case ImageFormat::BGR24:
    case ImageFormat::BGRA32:
    case ImageFormat::BGR48:
    case ImageFormat::BGRA64:
        mat_view = src_mat;
        break;

    case ImageFormat::RGB24:
        cv::cvtColor(src_mat, temp_mat, cv::COLOR_RGB2BGR);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGBA32:
        cv::cvtColor(src_mat, temp_mat, cv::COLOR_RGBA2BGRA);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGB48:
        cv::cvtColor(src_mat, temp_mat, cv::COLOR_RGB2BGR);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGBA64:
        cv::cvtColor(src_mat, temp_mat, cv::COLOR_RGBA2BGRA);
        mat_view = temp_mat;
        break;

    default:
        return common::errors::New(
            common::errors::imgproc_module::kUnsupportedImageFormat,
            "unsupported image format for direct encoding");
    }

    if (options.format == EncodeFormat::JPG && mat_view.depth() != CV_8U) {
        return common::errors::New(
            common::errors::imgproc_module::kJpegOnlySupports8Bit,
            "jpeg encoding only supports 8-bit images");
    }
    return nullptr;
}

common::errors::ErrorPtr MakeEncodableMat(
    const cv::Mat& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    cv::Mat& mat_view,
    cv::Mat& temp_mat)
{
    if (auto err = ValidateMat(src)) {
        return err;
    }

    if (IsBayerFormat(src_format)) {
        if (auto err = MakeDebayeredMat(src, src_format, temp_mat)) {
            return err;
        }
        mat_view = temp_mat;
        return nullptr;
    }

    switch (src_format) {
    case ImageFormat::GRAY8:
    case ImageFormat::GRAY16:
    case ImageFormat::MONO8:
    case ImageFormat::MONO10:
    case ImageFormat::MONO12:
    case ImageFormat::MONO14:
    case ImageFormat::MONO16:
    case ImageFormat::BGR24:
    case ImageFormat::BGRA32:
    case ImageFormat::BGR48:
    case ImageFormat::BGRA64:
        mat_view = src;
        break;

    case ImageFormat::RGB24:
        cv::cvtColor(src, temp_mat, cv::COLOR_RGB2BGR);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGBA32:
        cv::cvtColor(src, temp_mat, cv::COLOR_RGBA2BGRA);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGB48:
        cv::cvtColor(src, temp_mat, cv::COLOR_RGB2BGR);
        mat_view = temp_mat;
        break;
    case ImageFormat::RGBA64:
        cv::cvtColor(src, temp_mat, cv::COLOR_RGBA2BGRA);
        mat_view = temp_mat;
        break;

    default:
        return common::errors::New(
            common::errors::imgproc_module::kUnsupportedImageFormat,
            "unsupported image format for direct encoding");
    }

    if (options.format == EncodeFormat::JPG && mat_view.depth() != CV_8U) {
        return common::errors::New(
            common::errors::imgproc_module::kJpegOnlySupports8Bit,
            "jpeg encoding only supports 8-bit images");
    }
    return nullptr;
}

}  // namespace

common::errors::ErrorPtr Debayer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    std::shared_ptr<common::algorithm::ImageBuffer>& dst)
{
    if (!IsBayerFormat(src_format)) {
        return common::errors::New(common::errors::imgproc_module::kInvalidArgument, "src_format is not a bayer format");
    }

    cv::Mat dst_mat;
    if (auto err = MakeDebayeredMat(src, src_format, dst_mat)) {
        return err;
    }

    // Zero-copy: let ImageBuffer share memory with dst_mat.
    auto mat_holder = std::make_shared<cv::Mat>(std::move(dst_mat));
    dst = std::make_shared<common::algorithm::ImageBuffer>();
    if (!dst || !mat_holder || mat_holder->empty()) {
        return common::errors::New(common::errors::imgproc_module::kBuildOutputFailed, "failed to build debayer output");
    }

    dst->width = mat_holder->cols;
    dst->height = mat_holder->rows;
    dst->channels = mat_holder->channels();
    dst->elem_size = static_cast<int>(mat_holder->elemSize1());
    dst->step = static_cast<int>(mat_holder->step);
    dst->data = std::shared_ptr<uint8_t[]>(
        mat_holder->data,
        [mat_holder](uint8_t*) mutable { mat_holder.reset(); });
    return nullptr;
}

common::errors::ErrorPtr EncodeToBuffer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    std::shared_ptr<EncodedBuffer>& out)
{
    cv::Mat mat_view;
    cv::Mat temp_mat;
    if (auto err = MakeEncodableMat(src, src_format, options, mat_view, temp_mat)) {
        return err;
    }

    auto bytes = std::make_shared<std::vector<uint8_t>>();
    if (!cv::imencode(ExtensionOf(options.format), mat_view, *bytes, BuildEncodeParams(options))) {
        return common::errors::New(common::errors::imgproc_module::kEncodeFailed, "cv::imencode failed");
    }

    out = std::make_shared<EncodedBuffer>();
    out->format = options.format;
    out->bytes = std::move(bytes);
    return nullptr;
}

common::errors::ErrorPtr Debayer(
    const std::shared_ptr<common::algorithm::ImageBuffer>& src,
    ImageFormat src_format,
    cv::Mat& dst)
{
    if (!IsBayerFormat(src_format)) {
        return common::errors::New(common::errors::imgproc_module::kInvalidArgument, "src_format is not a bayer format");
    }
    cv::Mat src_mat;
    if (auto err = WrapMatView(src, ImageFormatToType(src_format), src_mat)) {
        return err;
    }
    return MakeDebayeredMat(src_mat, src_format, dst);
}

common::errors::ErrorPtr Debayer(
    const cv::Mat& src,
    ImageFormat src_format,
    cv::Mat& dst)
{
    if (!IsBayerFormat(src_format)) {
        return common::errors::New(common::errors::imgproc_module::kInvalidArgument,
                                   "src_format is not a bayer format");
    }
    return MakeDebayeredMat(src, src_format, dst);
}

common::errors::ErrorPtr EncodeToBuffer(
    const cv::Mat& src,
    ImageFormat src_format,
    const EncodeOptions& options,
    std::shared_ptr<EncodedBuffer>& out)
{
    cv::Mat mat_view;
    cv::Mat temp_mat;
    if (auto err = MakeEncodableMat(src, src_format, options, mat_view, temp_mat)) {
        return err;
    }

    auto bytes = std::make_shared<std::vector<uint8_t>>();
    if (!cv::imencode(ExtensionOf(options.format), mat_view, *bytes, BuildEncodeParams(options))) {
        return common::errors::New(common::errors::imgproc_module::kEncodeFailed, "cv::imencode failed");
    }

    out = std::make_shared<EncodedBuffer>();
    out->format = options.format;
    out->bytes = std::move(bytes);
    return nullptr;
}

common::errors::ErrorPtr ConvertMat12To8(
    const cv::Mat& src,
    cv::Mat& dst)
{
    if (auto err = ValidateMat(src)) {
        return err;
    }
    if (src.depth() != CV_16U) {
        return common::errors::New(
            common::errors::imgproc_module::kConvertSourceTypeMismatch,
            "src must be a 16-bit unsigned cv::Mat");
    }

    const int dst_type =
        (src.channels() == 1) ? CV_8UC1 :
        (src.channels() == 3) ? CV_8UC3 :
        (src.channels() == 4) ? CV_8UC4 : -1;
    if (dst_type < 0) {
        return common::errors::New(
            common::errors::imgproc_module::kConvertSourceTypeMismatch,
            "unsupported channel count for 16-bit to 8-bit conversion");
    }

    double min_val = 0.0;
    double max_val = 65535.0;
    cv::minMaxLoc(src, &min_val, &max_val);

    if (max_val > min_val) {
        const double scale = 255.0 / (max_val - min_val);
        const double shift = -min_val * scale;
        src.convertTo(dst, dst_type, scale, shift);
    } else {
        src.convertTo(dst, dst_type, 0.0);
    }
    return nullptr;
}

common::errors::ErrorPtr Rotate(
    const cv::Mat& src,
    int rotate_code,
    cv::Mat& dst)
{
    if (auto err = ValidateMat(src)) {
        return err;
    }
    if (rotate_code != cv::ROTATE_90_CLOCKWISE
        && rotate_code != cv::ROTATE_180
        && rotate_code != cv::ROTATE_90_COUNTERCLOCKWISE) {
        return common::errors::New(common::errors::imgproc_module::kRotateCodeUnsupported, "unsupported rotate code");
    }
    cv::rotate(src, dst, rotate_code);
    return nullptr;
}

}  // namespace common::imgproc
