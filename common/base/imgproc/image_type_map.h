#pragma once

#include <cstdint>
#include <common/3rdparty/nlohmann/json.hpp>

namespace common
{
namespace imgproc
{

// 与 OpenCV Mat type 编码规则对齐：
// type = depth + ((channels - 1) << kDepthShift)
enum class PixelDepth : int
{
    UINT8   = 0,  // 对齐 CV_8U
    SINT8   = 1,  // 对齐 CV_8S
    UINT16  = 2,  // 对齐 CV_16U
    SINT16  = 3,  // 对齐 CV_16S
    SINT32  = 4,  // 对齐 CV_32S
    FLOAT32 = 5,  // 对齐 CV_32F
    FLOAT64 = 6,  // 对齐 CV_64F
    FLOAT16 = 7   // 对齐 CV_16F
};

constexpr int kDepthShift = 3;
constexpr int kDepthMax   = 1 << kDepthShift;
constexpr int kMaxChannels = 512;

constexpr int MakeImageType(PixelDepth depth, int channels)
{
    return static_cast<int>(depth) + ((channels - 1) << kDepthShift);
}

constexpr PixelDepth GetDepth(int imageType)
{
    return static_cast<PixelDepth>(imageType & (kDepthMax - 1));
}

constexpr int GetChannels(int imageType)
{
    return ((imageType >> kDepthShift) & (kMaxChannels - 1)) + 1;
}

constexpr bool IsValidImageType(int imageType)
{
    const int channels = GetChannels(imageType);
    const int depth = imageType & (kDepthMax - 1);
    return channels >= 1 && channels <= kMaxChannels && depth >= 0 && depth < kDepthMax;
}

constexpr int DepthBytes(PixelDepth depth)
{
    return (depth == PixelDepth::UINT8 || depth == PixelDepth::SINT8) ? 1 :
           (depth == PixelDepth::UINT16 || depth == PixelDepth::SINT16 || depth == PixelDepth::FLOAT16) ? 2 :
           (depth == PixelDepth::SINT32 || depth == PixelDepth::FLOAT32) ? 4 :
           (depth == PixelDepth::FLOAT64) ? 8 : 0;
}

constexpr int ElemBytes(int imageType)
{
    return DepthBytes(GetDepth(imageType)) * GetChannels(imageType);
}

// 常用类型常量（命名风格与 OpenCV 接近）
constexpr int IMG_8UC1  = MakeImageType(PixelDepth::UINT8,   1);
constexpr int IMG_8UC2  = MakeImageType(PixelDepth::UINT8,   2);
constexpr int IMG_8UC3  = MakeImageType(PixelDepth::UINT8,   3);
constexpr int IMG_8UC4  = MakeImageType(PixelDepth::UINT8,   4);
constexpr int IMG_8SC1  = MakeImageType(PixelDepth::SINT8,   1);
constexpr int IMG_16UC1 = MakeImageType(PixelDepth::UINT16,  1);
constexpr int IMG_16UC3 = MakeImageType(PixelDepth::UINT16,  3);
constexpr int IMG_16UC4 = MakeImageType(PixelDepth::UINT16,  4);
constexpr int IMG_16SC1 = MakeImageType(PixelDepth::SINT16,  1);
constexpr int IMG_32SC1 = MakeImageType(PixelDepth::SINT32,  1);
constexpr int IMG_32FC1 = MakeImageType(PixelDepth::FLOAT32, 1);
constexpr int IMG_32FC2 = MakeImageType(PixelDepth::FLOAT32, 2);
constexpr int IMG_32FC3 = MakeImageType(PixelDepth::FLOAT32, 3);
constexpr int IMG_64FC1 = MakeImageType(PixelDepth::FLOAT64, 1);

// 图像格式（语义层），与 imageType（存储层）分离。
enum class ImageFormat : int
{
    UNKNOWN = 0,

    // Mono / Gray
    MONO8,
    MONO10,
    MONO12,
    MONO14,
    MONO16,
    GRAY8,
    GRAY16,

    // Packed RGB/BGR (8-bit)
    BGR24,
    RGB24,
    BGRA32,
    RGBA32,
    ARGB32,
    ABGR32,

    // High bit-depth RGB/BGR
    BGR48,
    RGB48,
    BGRA64,
    RGBA64,

    // Bayer short naming (aligned with OpenCV BayerRG/BayerBG/BayerGB/BayerGR)
    BAYER_BG8,
    BAYER_RG8,
    BAYER_GB8,
    BAYER_GR8,
    BAYER_BG10,
    BAYER_RG10,
    BAYER_GB10,
    BAYER_GR10,
    BAYER_BG12,
    BAYER_RG12,
    BAYER_GB12,
    BAYER_GR12,
    BAYER_BG14,
    BAYER_RG14,
    BAYER_GB14,
    BAYER_GR14,
    BAYER_BG16,
    BAYER_RG16,
    BAYER_GB16,
    BAYER_GR16,

    // Compatibility aliases
    BAYER_BGGR8  = BAYER_BG8,
    BAYER_RGGB8  = BAYER_RG8,
    BAYER_GBRG8  = BAYER_GB8,
    BAYER_GRBG8  = BAYER_GR8,
    BAYER_BGGR10 = BAYER_BG10,
    BAYER_RGGB10 = BAYER_RG10,
    BAYER_GBRG10 = BAYER_GB10,
    BAYER_GRBG10 = BAYER_GR10,
    BAYER_BGGR12 = BAYER_BG12,
    BAYER_RGGB12 = BAYER_RG12,
    BAYER_GBRG12 = BAYER_GB12,
    BAYER_GRBG12 = BAYER_GR12,
    BAYER_BGGR14 = BAYER_BG14,
    BAYER_RGGB14 = BAYER_RG14,
    BAYER_GBRG14 = BAYER_GB14,
    BAYER_GRBG14 = BAYER_GR14,
    BAYER_BGGR16 = BAYER_BG16,
    BAYER_RGGB16 = BAYER_RG16,
    BAYER_GBRG16 = BAYER_GB16,
    BAYER_GRBG16 = BAYER_GR16,

    YUV420P,   // I420/YU12（3 平面）
    YV12,      // Y + V + U（3 平面）
    NV12,      // Y + UV
    NV21,      // Y + VU
    YUYV422,   // 打包 4:2:2
    UYVY422,   // 打包 4:2:2
    YVYU422,   // 打包 4:2:2
    VYUY422,   // 打包 4:2:2
    YUV422P,   // 3 平面 4:2:2
    YUV444P,   // 3 平面 4:4:4

    BINARY1,   // 逻辑二值图（通常仍以 8-bit 存储）
    LABEL32S   // 连通域标签图等
};

NLOHMANN_JSON_SERIALIZE_ENUM(ImageFormat, {
    {ImageFormat::UNKNOWN, "UNKNOWN"},
    {ImageFormat::MONO8, "MONO8"},
    {ImageFormat::MONO10, "MONO10"},
    {ImageFormat::MONO12, "MONO12"},
    {ImageFormat::MONO14, "MONO14"},
    {ImageFormat::MONO16, "MONO16"},
    {ImageFormat::GRAY8, "GRAY8"},
    {ImageFormat::GRAY16, "GRAY16"},
    {ImageFormat::BGR24, "BGR24"},
    {ImageFormat::RGB24, "RGB24"},
    {ImageFormat::BGRA32, "BGRA32"},
    {ImageFormat::RGBA32, "RGBA32"},
    {ImageFormat::ARGB32, "ARGB32"},
    {ImageFormat::ABGR32, "ABGR32"},
    {ImageFormat::BGR48, "BGR48"},
    {ImageFormat::RGB48, "RGB48"},
    {ImageFormat::BGRA64, "BGRA64"},
    {ImageFormat::RGBA64, "RGBA64"},
    {ImageFormat::BAYER_BG8, "BAYER_BG8"},
    {ImageFormat::BAYER_RG8, "BAYER_RG8"},
    {ImageFormat::BAYER_GB8, "BAYER_GB8"},
    {ImageFormat::BAYER_GR8, "BAYER_GR8"},
    {ImageFormat::BAYER_BG10, "BAYER_BG10"},
    {ImageFormat::BAYER_RG10, "BAYER_RG10"},
    {ImageFormat::BAYER_GB10, "BAYER_GB10"},
    {ImageFormat::BAYER_GR10, "BAYER_GR10"},
    {ImageFormat::BAYER_BG12, "BAYER_BG12"},
    {ImageFormat::BAYER_RG12, "BAYER_RG12"},
    {ImageFormat::BAYER_GB12, "BAYER_GB12"},
    {ImageFormat::BAYER_GR12, "BAYER_GR12"},
    {ImageFormat::BAYER_BG14, "BAYER_BG14"},
    {ImageFormat::BAYER_RG14, "BAYER_RG14"},
    {ImageFormat::BAYER_GB14, "BAYER_GB14"},
    {ImageFormat::BAYER_GR14, "BAYER_GR14"},
    {ImageFormat::BAYER_BG16, "BAYER_BG16"},
    {ImageFormat::BAYER_RG16, "BAYER_RG16"},
    {ImageFormat::BAYER_GB16, "BAYER_GB16"},
    {ImageFormat::BAYER_GR16, "BAYER_GR16"},
    {ImageFormat::YUV420P, "YUV420P"},
    {ImageFormat::YV12, "YV12"},
    {ImageFormat::NV12, "NV12"},
    {ImageFormat::NV21, "NV21"},
    {ImageFormat::YUYV422, "YUYV422"},
    {ImageFormat::UYVY422, "UYVY422"},
    {ImageFormat::YVYU422, "YVYU422"},
    {ImageFormat::VYUY422, "VYUY422"},
    {ImageFormat::YUV422P, "YUV422P"},
    {ImageFormat::YUV444P, "YUV444P"},
    {ImageFormat::BINARY1, "BINARY1"},
    {ImageFormat::LABEL32S, "LABEL32S"}
})

constexpr int ImageFormatToType(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::MONO8:
    case ImageFormat::GRAY8:      return IMG_8UC1;
    case ImageFormat::MONO10:
    case ImageFormat::MONO12:
    case ImageFormat::MONO14:
    case ImageFormat::MONO16:
    case ImageFormat::GRAY16:     return IMG_16UC1;
    case ImageFormat::BGR24:      return IMG_8UC3;
    case ImageFormat::RGB24:      return IMG_8UC3;
    case ImageFormat::BGRA32:     return IMG_8UC4;
    case ImageFormat::RGBA32:     return IMG_8UC4;
    case ImageFormat::ARGB32:     return IMG_8UC4;
    case ImageFormat::ABGR32:     return IMG_8UC4;
    case ImageFormat::BGR48:      return IMG_16UC3;
    case ImageFormat::RGB48:      return IMG_16UC3;
    case ImageFormat::BGRA64:     return IMG_16UC4;
    case ImageFormat::RGBA64:     return IMG_16UC4;
    case ImageFormat::BAYER_BG8:
    case ImageFormat::BAYER_RG8:
    case ImageFormat::BAYER_GB8:
    case ImageFormat::BAYER_GR8:
        return IMG_8UC1;
    case ImageFormat::BAYER_BG10:
    case ImageFormat::BAYER_RG10:
    case ImageFormat::BAYER_GB10:
    case ImageFormat::BAYER_GR10:
    case ImageFormat::BAYER_BG12:
    case ImageFormat::BAYER_RG12:
    case ImageFormat::BAYER_GB12:
    case ImageFormat::BAYER_GR12:
    case ImageFormat::BAYER_BG14:
    case ImageFormat::BAYER_RG14:
    case ImageFormat::BAYER_GB14:
    case ImageFormat::BAYER_GR14:
    case ImageFormat::BAYER_BG16:
    case ImageFormat::BAYER_RG16:
    case ImageFormat::BAYER_GB16:
    case ImageFormat::BAYER_GR16:
        return IMG_16UC1;
    case ImageFormat::YUV420P:    return IMG_8UC1;
    case ImageFormat::YV12:       return IMG_8UC1;
    case ImageFormat::NV12:       return IMG_8UC1;
    case ImageFormat::NV21:       return IMG_8UC1;
    case ImageFormat::YUYV422:    return IMG_8UC2;
    case ImageFormat::UYVY422:    return IMG_8UC2;
    case ImageFormat::YVYU422:    return IMG_8UC2;
    case ImageFormat::VYUY422:    return IMG_8UC2;
    case ImageFormat::YUV422P:    return IMG_8UC1;
    case ImageFormat::YUV444P:    return IMG_8UC1;
    case ImageFormat::BINARY1:    return IMG_8UC1;
    case ImageFormat::LABEL32S:   return IMG_32SC1;
    default:                      return -1;
    }
}

constexpr PixelDepth ImageFormatToPixelDepth(ImageFormat format)
{
    switch (format)
    {
    case ImageFormat::MONO8:
    case ImageFormat::GRAY8:
    case ImageFormat::BGR24:
    case ImageFormat::RGB24:
    case ImageFormat::BGRA32:
    case ImageFormat::RGBA32:
    case ImageFormat::ARGB32:
    case ImageFormat::ABGR32:
    case ImageFormat::BAYER_BG8:
    case ImageFormat::BAYER_RG8:
    case ImageFormat::BAYER_GB8:
    case ImageFormat::BAYER_GR8:
    case ImageFormat::YUV420P:
    case ImageFormat::YV12:
    case ImageFormat::NV12:
    case ImageFormat::NV21:
    case ImageFormat::YUYV422:
    case ImageFormat::UYVY422:
    case ImageFormat::YVYU422:
    case ImageFormat::VYUY422:
    case ImageFormat::YUV422P:
    case ImageFormat::YUV444P:
    case ImageFormat::BINARY1:
        return PixelDepth::UINT8;

    case ImageFormat::MONO10:
    case ImageFormat::MONO12:
    case ImageFormat::MONO14:
    case ImageFormat::MONO16:
    case ImageFormat::GRAY16:
    case ImageFormat::BGR48:
    case ImageFormat::RGB48:
    case ImageFormat::BGRA64:
    case ImageFormat::RGBA64:
    case ImageFormat::BAYER_BG10:
    case ImageFormat::BAYER_RG10:
    case ImageFormat::BAYER_GB10:
    case ImageFormat::BAYER_GR10:
    case ImageFormat::BAYER_BG12:
    case ImageFormat::BAYER_RG12:
    case ImageFormat::BAYER_GB12:
    case ImageFormat::BAYER_GR12:
    case ImageFormat::BAYER_BG14:
    case ImageFormat::BAYER_RG14:
    case ImageFormat::BAYER_GB14:
    case ImageFormat::BAYER_GR14:
    case ImageFormat::BAYER_BG16:
    case ImageFormat::BAYER_RG16:
    case ImageFormat::BAYER_GB16:
    case ImageFormat::BAYER_GR16:
        return PixelDepth::UINT16;

    case ImageFormat::LABEL32S:
        return PixelDepth::SINT32;

    default:
        return PixelDepth::UINT8;
    }
}

constexpr bool IsColorFormat(ImageFormat format)
{
    return format == ImageFormat::BGR24 ||
           format == ImageFormat::RGB24 ||
           format == ImageFormat::BGRA32 ||
           format == ImageFormat::RGBA32 ||
           format == ImageFormat::ARGB32 ||
           format == ImageFormat::ABGR32 ||
           format == ImageFormat::BGR48 ||
           format == ImageFormat::RGB48 ||
           format == ImageFormat::BGRA64 ||
           format == ImageFormat::RGBA64;
}

constexpr bool IsBayerFormat(ImageFormat format)
{
    return format == ImageFormat::BAYER_BG8 ||
           format == ImageFormat::BAYER_RG8 ||
           format == ImageFormat::BAYER_GB8 ||
           format == ImageFormat::BAYER_GR8 ||
           format == ImageFormat::BAYER_BG10 ||
           format == ImageFormat::BAYER_RG10 ||
           format == ImageFormat::BAYER_GB10 ||
           format == ImageFormat::BAYER_GR10 ||
           format == ImageFormat::BAYER_BG12 ||
           format == ImageFormat::BAYER_RG12 ||
           format == ImageFormat::BAYER_GB12 ||
           format == ImageFormat::BAYER_GR12 ||
           format == ImageFormat::BAYER_BG14 ||
           format == ImageFormat::BAYER_RG14 ||
           format == ImageFormat::BAYER_GB14 ||
           format == ImageFormat::BAYER_GR14 ||
           format == ImageFormat::BAYER_BG16 ||
           format == ImageFormat::BAYER_RG16 ||
           format == ImageFormat::BAYER_GB16 ||
           format == ImageFormat::BAYER_GR16;
}

constexpr bool IsYuvFormat(ImageFormat format)
{
    return format == ImageFormat::YUV420P ||
           format == ImageFormat::YV12 ||
           format == ImageFormat::NV12 ||
           format == ImageFormat::NV21 ||
           format == ImageFormat::YUYV422 ||
           format == ImageFormat::UYVY422 ||
           format == ImageFormat::YVYU422 ||
           format == ImageFormat::VYUY422 ||
           format == ImageFormat::YUV422P ||
           format == ImageFormat::YUV444P;
}

}  // namespace imgproc
}  // namespace common

