/**
 * imgproc_example/main.cpp
 *
 * 演示：
 *   1. 构造 Bayer 原始图（ImageBuffer）
 *   2. Debayer 到 BGR Mat
 *   3. 编码为 JPG/PNG 内存 buffer
 *   4. 从 .raw 文件直接读取并执行同样流程
 */

#include <common/base/imgproc/imgproc.h>

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include <opencv2/core.hpp>

using common::algorithm::ImageBuffer;
using common::imgproc::EncodeFormat;
using common::imgproc::EncodeOptions;
using common::imgproc::ImageFormat;

namespace {

// =============================================================================
// 场景 1：内存构造 Bayer 图 -> Debayer -> JPG/PNG buffer
// =============================================================================
bool RunInMemoryExample()
{
    auto raw = ImageBuffer::Create(8, 8, 1, 1);
    if (!raw || raw->Empty()) {
        std::cerr << "[InMemory] failed to allocate raw buffer\n";
        return false;
    }
    std::memset(raw->data.get(), 127, raw->ByteSize());

    cv::Mat bgr;
    if (auto err = common::imgproc::Debayer(raw, ImageFormat::BAYER_RG8, bgr)) {
        std::cerr << "[InMemory] Debayer failed: " << err->Message() << "\n";
        return false;
    }
    assert(!bgr.empty());
    assert(bgr.channels() == 3);
    std::cout << "[InMemory] Debayer ok: " << bgr.cols << "x" << bgr.rows
              << ", channels=" << bgr.channels() << "\n";

    EncodeOptions jpg_opt;
    jpg_opt.format = EncodeFormat::JPG;
    jpg_opt.jpeg_quality = 90;
    std::shared_ptr<common::imgproc::EncodedBuffer> jpg_buf;
    if (auto err = common::imgproc::EncodeToBuffer(bgr, ImageFormat::BGR24, jpg_opt, jpg_buf)) {
        std::cerr << "[InMemory] Encode JPG failed: " << err->Message() << "\n";
        return false;
    }
    assert(jpg_buf && !jpg_buf->Empty());
    std::cout << "[InMemory] JPG bytes: " << jpg_buf->Size() << "\n";

    EncodeOptions png_opt;
    png_opt.format = EncodeFormat::PNG;
    png_opt.png_compression = 3;
    std::shared_ptr<common::imgproc::EncodedBuffer> png_buf;
    if (auto err = common::imgproc::EncodeToBuffer(raw, ImageFormat::BAYER_RG8, png_opt, png_buf)) {
        std::cerr << "[InMemory] Encode PNG failed: " << err->Message() << "\n";
        return false;
    }
    assert(png_buf && !png_buf->Empty());
    std::cout << "[InMemory] PNG bytes: " << png_buf->Size() << "\n";
    return true;
}

// =============================================================================
// 场景 2：读取 RAW 文件 -> Debayer -> JPG buffer
// =============================================================================
bool RunRawFileExample()
{
    // 示例 RAW 配置：按实际文件修改
    // const std::string raw_path = "2026-04-27_17_57_08_370.raw";
    const std::string raw_path = "Image_20260506175427540_w5496_h3672_pBayerRG12.raw";
    constexpr int raw_width = 5496;
    constexpr int raw_height = 3672;

    std::ifstream ifs(raw_path, std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "[RAW] file not found, skip: " << raw_path << "\n";
        return true;
    }

    auto raw = ImageBuffer::Create(raw_width, raw_height, 1, 1);
    if (!raw || raw->Empty()) {
        std::cerr << "[RAW] failed to allocate raw buffer\n";
        return false;
    }

    ifs.read(reinterpret_cast<char*>(raw->data.get()),
             static_cast<std::streamsize>(raw->ByteSize()));
    if (ifs.gcount() != static_cast<std::streamsize>(raw->ByteSize())) {
        std::cerr << "[RAW] invalid file size, expected " << raw->ByteSize()
                  << " bytes, got " << ifs.gcount() << "\n";
        return false;
    }
    ifs.close();

    cv::Mat bgr;
    if (auto err = common::imgproc::Debayer(raw, ImageFormat::BAYER_RG8, bgr)) {
        std::cerr << "[RAW] Debayer failed: " << err->Message() << "\n";
        return false;
    }

    EncodeOptions jpg_opt;
    jpg_opt.format = EncodeFormat::JPG;
    jpg_opt.jpeg_quality = 90;
    std::shared_ptr<common::imgproc::EncodedBuffer> jpg_buf;
    if (auto err = common::imgproc::EncodeToBuffer(bgr, ImageFormat::BGR24, jpg_opt, jpg_buf)) {
        std::cerr << "[RAW] Encode JPG failed: " << err->Message() << "\n";
        return false;
    }
    assert(jpg_buf && !jpg_buf->Empty());

    const std::string jpg_output_path = "Image_20260506175427540_w5496_h3672_pBayerRG12.jpg";
    std::ofstream ofs(jpg_output_path, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "[RAW] failed to open output file: " << jpg_output_path << "\n";
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(jpg_buf->Data()),
              static_cast<std::streamsize>(jpg_buf->Size()));
    ofs.close();

    std::cout << "[RAW] success, JPG bytes: " << jpg_buf->Size()
              << ", saved: " << jpg_output_path << "\n";
    return true;
}

// =============================================================================
// 场景 3：读取 16-bit RAW 文件 -> Debayer -> PNG 写盘
// =============================================================================
bool RunRaw16FileExample()
{
    // 示例 16-bit RAW 配置：按实际文件修改
    const std::string raw_path = "D:\\00_Aether\\image_20260701082608_BG12_w9344_h7000_pBayerBG12.raw";
    constexpr int raw_width = 9344;
    constexpr int raw_height = 7000;
    constexpr int raw_bits = 12;  // BayerRG12: 有效位深 12-bit（存储为 uint16）

    std::ifstream ifs(raw_path, std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "[RAW16] file not found, skip: " << raw_path << "\n";
        return true;
    }

    auto raw = ImageBuffer::Create(raw_width, raw_height, 1, 2);
    if (!raw || raw->Empty()) {
        std::cerr << "[RAW16] failed to allocate raw buffer\n";
        return false;
    }

    ifs.read(reinterpret_cast<char*>(raw->data.get()),
             static_cast<std::streamsize>(raw->ByteSize()));
    if (ifs.gcount() != static_cast<std::streamsize>(raw->ByteSize())) {
        std::cerr << "[RAW16] invalid file size, expected " << raw->ByteSize()
                  << " bytes, got " << ifs.gcount() << "\n";
        return false;
    }
    ifs.close();

    cv::Mat bgr16;
    if (auto err = common::imgproc::Debayer(raw, ImageFormat::BAYER_RG12, bgr16)) {
        std::cerr << "[RAW16] Debayer failed: " << err->Message() << "\n";
        return false;
    }

    // 注意：12-bit 数据直接保存为 16-bit PNG 时，查看器通常按 [0,65535] 映射显示，
    // 实际像素只用到 [0,4095]，所以看起来会偏黑。
    // 这里把有效位深左移到 16-bit 全量程，便于常见查看器正常显示。
    cv::Mat bgr16_full;
    const double scale = 1u << (16 - raw_bits);  // 12->16: *16
    bgr16.convertTo(bgr16_full, bgr16.type(), scale);

    // 16-bit 数据用 PNG 保存，避免 JPG 的 8-bit 有损压缩导致数据截断。
    EncodeOptions png_opt;
    png_opt.format = EncodeFormat::PNG;
    png_opt.png_compression = 3;
    std::shared_ptr<common::imgproc::EncodedBuffer> png_buf;
    if (auto err = common::imgproc::EncodeToBuffer(bgr16_full, ImageFormat::BGR48, png_opt, png_buf)) {
        std::cerr << "[RAW16] Encode PNG failed: " << err->Message() << "\n";
        return false;
    }
    assert(png_buf && !png_buf->Empty());

    const std::string png_output_path = "D:\\00_Aether\\image_20260701082608_BG12_w9344_h7000_pBayerBG12.png";
    std::ofstream ofs(png_output_path, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "[RAW16] failed to open output file: " << png_output_path << "\n";
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(png_buf->Data()),
              static_cast<std::streamsize>(png_buf->Size()));
    ofs.close();

    std::cout << "[RAW16] success, PNG bytes: " << png_buf->Size()
              << ", saved: " << png_output_path << "\n";

    // 同时输出 8-bit 预览图：更适合肉眼快速确认效果（带有压缩/量化，仅用于预览）。
    // cv::Mat bgr8_preview;
    // if (auto err = common::imgproc::ConvertMat12To8(bgr16, bgr8_preview)) {
    //     std::cerr << "[RAW16] ConvertMat12To8 failed: " << err->Message() << "\n";
    //     return false;
    // }
    // EncodeOptions jpg_opt;
    // jpg_opt.format = EncodeFormat::JPG;
    // jpg_opt.jpeg_quality = 90;
    // std::shared_ptr<common::imgproc::EncodedBuffer> jpg_buf;
    // if (auto err = common::imgproc::EncodeToBuffer(bgr8_preview, ImageFormat::BGR24, jpg_opt, jpg_buf)) {
    //     std::cerr << "[RAW16] Encode JPG preview failed: " << err->Message() << "\n";
    //     return false;
    // }
    // const std::string jpg_output_path = "Image_20260506175438684_w5496_h3672_pBayerRG12.jpg";
    // std::ofstream jpg_ofs(jpg_output_path, std::ios::binary);
    // if (!jpg_ofs.is_open()) {
    //     std::cerr << "[RAW16] failed to open preview output file: " << jpg_output_path << "\n";
    //     return false;
    // }
    // jpg_ofs.write(reinterpret_cast<const char*>(jpg_buf->Data()),
    //               static_cast<std::streamsize>(jpg_buf->Size()));
    // jpg_ofs.close();
    // std::cout << "[RAW16] preview saved: " << jpg_output_path << "\n";

    return true;
}

// =============================================================================
// 场景 4：读取 RAW 文件 -> Debayer -> 旋转 -> JPG 写盘
// =============================================================================
bool RunRawRotateExample()
{
    // 示例 RAW 配置：按实际文件修改
    const std::string raw_path = "2026-04-27_17_57_08_370.raw";
    constexpr int raw_width = 5496;
    constexpr int raw_height = 3672;

    std::ifstream ifs(raw_path, std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "[RAW-Rotate] file not found, skip: " << raw_path << "\n";
        return true;
    }

    auto raw = ImageBuffer::Create(raw_width, raw_height, 1, 1);
    if (!raw || raw->Empty()) {
        std::cerr << "[RAW-Rotate] failed to allocate raw buffer\n";
        return false;
    }

    ifs.read(reinterpret_cast<char*>(raw->data.get()),
             static_cast<std::streamsize>(raw->ByteSize()));
    if (ifs.gcount() != static_cast<std::streamsize>(raw->ByteSize())) {
        std::cerr << "[RAW-Rotate] invalid file size, expected " << raw->ByteSize()
                  << " bytes, got " << ifs.gcount() << "\n";
        return false;
    }
    ifs.close();

    cv::Mat bgr;
    if (auto err = common::imgproc::Debayer(raw, ImageFormat::BAYER_RG8, bgr)) {
        std::cerr << "[RAW-Rotate] Debayer failed: " << err->Message() << "\n";
        return false;
    }

    cv::Mat rotated;
    if (auto err = common::imgproc::Rotate(bgr, cv::ROTATE_90_CLOCKWISE, rotated)) {
        std::cerr << "[RAW-Rotate] Rotate failed: " << err->Message() << "\n";
        return false;
    }

    EncodeOptions jpg_opt;
    jpg_opt.format = EncodeFormat::JPG;
    jpg_opt.jpeg_quality = 90;
    std::shared_ptr<common::imgproc::EncodedBuffer> jpg_buf;
    if (auto err = common::imgproc::EncodeToBuffer(rotated, ImageFormat::BGR24, jpg_opt, jpg_buf)) {
        std::cerr << "[RAW-Rotate] Encode JPG failed: " << err->Message() << "\n";
        return false;
    }
    assert(jpg_buf && !jpg_buf->Empty());

    const std::string jpg_output_path = "demo_rg8_5496x3672_rot90cw.jpg";
    std::ofstream ofs(jpg_output_path, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "[RAW-Rotate] failed to open output file: " << jpg_output_path << "\n";
        return false;
    }
    ofs.write(reinterpret_cast<const char*>(jpg_buf->Data()),
              static_cast<std::streamsize>(jpg_buf->Size()));
    ofs.close();

    std::cout << "[RAW-Rotate] success, JPG bytes: " << jpg_buf->Size()
              << ", saved: " << jpg_output_path << "\n";
    return true;
}

}  // namespace

// =============================================================================
// main：演示完整调用流程
// =============================================================================
int main()
{
    std::cout << "imgproc example start.\n";

    // -------------------------------------------------------------------------
    // 场景 1：内存构造 Bayer 图并完成 Debayer/编码
    // -------------------------------------------------------------------------
    // if (!RunInMemoryExample()) {
    //     return 1;
    // }

    // -------------------------------------------------------------------------
    // 场景 2：直接读取 .raw 文件并完成 Debayer/编码
    // -------------------------------------------------------------------------
    // if (!RunRawFileExample()) {
    //     return 1;
    // }

    // -------------------------------------------------------------------------
    // 场景 3：直接读取 16-bit .raw 文件并完成 Debayer/编码
    // -------------------------------------------------------------------------
    if (!RunRaw16FileExample()) {
        return 1;
    }

    // -------------------------------------------------------------------------
    // 场景 4：直接读取 .raw 文件，旋转后编码并写盘
    // -------------------------------------------------------------------------
    // if (!RunRawRotateExample()) {
    //     return 1;
    // }

    std::cout << "imgproc example done.\n";
    return 0;
}
