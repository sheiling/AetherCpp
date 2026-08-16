/**
 * algorithm_demo.cpp
 *
 * 演示：
 *   1. 继承 AlgorithmBase 实现具体算法
 *   2. 使用 REGISTER_ALGORITHM 宏完成工厂注册
 *   3. 通过 AlgorithmPacket 传递图像与标量参数（零拷贝）
 *   4. 通过工厂按名称创建并执行算法
 */

#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <opencv_4.10.0/include/opencv2/opencv.hpp>
#include <common/base/algorithm/algorithm.h>

using namespace common::algorithm;
using common::errors::ErrorPtr;

// =============================================================================
// 示例算法 1：亮度统计（演示标量参数传递）
// =============================================================================
class BrightnessStatAlgorithm : public AlgorithmBase
{
public:
    std::string Name()    const override { return "BrightnessStat"; }
    std::string Version() const override { return "1.0.0"; }

    ErrorPtr Initialize() override
    {
        std::cout << "[BrightnessStat] Initialize\n";
        return nullptr;
    }

    void Uninitialize() override
    {
        std::cout << "[BrightnessStat] Uninitialize\n";
    }

    ErrorPtr Execute(AlgorithmExecuteData& data) override
    {
        // 从输入包取图像（零拷贝）
        auto img = data.inputs.GetImage("src");
        if (!img || img->Empty())
            return common::errors::New(1, "input image 'src' is missing or empty");

        // 计算平均亮度（仅演示，实际可用 OpenCV mean()）
        const uint8_t* pixels = img->data.get();
        size_t total  = img->ByteSize();
        double sum    = 0.0;
        for (size_t i = 0; i < total; ++i)
            sum += pixels[i];
        double mean = sum / static_cast<double>(total);

        // 将结果写入输出包
        data.outputs.Set("mean_brightness", mean);
        data.outputs.Set("pixel_count",     static_cast<int64_t>(total));
        return nullptr;
    }
};

// 工厂注册（静态初始化，DLL 加载时自动触发）
REGISTER_ALGORITHM("BrightnessStat", BrightnessStatAlgorithm)


// =============================================================================
// 示例算法 2：图像翻转（演示图像零拷贝输出）
// =============================================================================
class FlipAlgorithm : public AlgorithmBase
{
public:
    std::string Name() const override { return "Flip"; }

    ErrorPtr Execute(AlgorithmExecuteData& data) override
    {
        auto src = data.inputs.GetImage("src");
        if (!src || src->Empty())
            return common::errors::New(1, "input image 'src' is missing or empty");

        // 读取翻转方向参数（默认水平翻转）
        bool horizontal = true;
        if (const bool* flag = data.inputs.Get<bool>("horizontal"))
            horizontal = *flag;

        // 分配输出图像（与输入同尺寸）
        auto dst = ImageBuffer::Create(
            src->width, src->height, src->channels, src->elem_size, src->step);

        // 简单逐行翻转演示（生产代码替换为 cv::flip）
        int row_bytes = src->step;
        for (int y = 0; y < src->height; ++y)
        {
            const uint8_t* src_row = src->data.get() + static_cast<size_t>(y) * row_bytes;
            uint8_t*       dst_row = dst->data.get() + static_cast<size_t>(y) * row_bytes;
            if (horizontal)
            {
                for (int x = 0; x < src->width; ++x)
                {
                    int px_bytes = src->channels * src->elem_size;
                    std::memcpy(
                        dst_row + x * px_bytes,
                        src_row + (src->width - 1 - x) * px_bytes,
                        px_bytes);
                }
            }
            else
            {
                std::memcpy(dst_row, src_row, row_bytes);
            }
        }

        // 将结果图像写入输出包（shared_ptr 传递，无拷贝）
        data.outputs.SetImage("dst", std::move(dst));
        return nullptr;
    }
};

REGISTER_ALGORITHM("Flip", FlipAlgorithm)

// =============================================================================
// 示例算法 3：RAW Bayer -> Debayer -> JPEG
// =============================================================================
class RawDebayerToJpegAlgorithm : public AlgorithmBase
{
public:
    std::string Name() const override { return "RawDebayerToJpeg"; }

    ErrorPtr Execute(AlgorithmExecuteData& data) override
    {
        auto raw = data.inputs.GetImage("raw_bayer");
        if (!raw || raw->Empty() || raw->channels != 1 || raw->elem_size != 1)
            return common::errors::New(1, "invalid raw_bayer image, expect 8-bit single channel");

        int bayer_code = cv::COLOR_BayerRG2BGR;
        if (const int* code = data.inputs.Get<int>("bayer_code"))
            bayer_code = *code;

        int quality = 95;
        if (const int* q = data.inputs.Get<int>("jpg_quality"))
            quality = *q;
        if (quality < 1) quality = 1;
        if (quality > 100) quality = 100;

        const std::string* output_path = data.inputs.Get<std::string>("jpg_output_path");
        if (!output_path || output_path->empty())
            return common::errors::New(1, "jpg_output_path is required");

        cv::Mat bayer(raw->height, raw->width, CV_8UC1, raw->data.get(), raw->step);
        cv::Mat bgr;
        cv::cvtColor(bayer, bgr, bayer_code);

        std::vector<unsigned char> jpeg_buf;
        const std::vector<int> encode_params = {cv::IMWRITE_JPEG_QUALITY, quality};
        if (!cv::imencode(".jpg", bgr, jpeg_buf, encode_params))
            return common::errors::New(1, "cv::imencode(.jpg) failed");

        std::ofstream ofs(*output_path, std::ios::binary);
        if (!ofs.is_open())
            return common::errors::Newf(1, "failed to open output jpg file: %s", output_path->c_str());
        ofs.write(reinterpret_cast<const char*>(jpeg_buf.data()),
                  static_cast<std::streamsize>(jpeg_buf.size()));
        ofs.close();

        data.outputs.Set("jpg_size", static_cast<int>(jpeg_buf.size()));
        data.outputs.Set("jpg_path", *output_path);
        return nullptr;
    }
};

REGISTER_ALGORITHM("RawDebayerToJpeg", RawDebayerToJpegAlgorithm)


// =============================================================================
// main：演示完整调用流程
// =============================================================================
int main()
{
    auto& factory = AlgorithmFactory::Instance();

    // 查看已注册的算法列表
    std::cout << "Registered algorithms:\n";
    for (const auto& name : factory.List())
        std::cout << "  - " << name << "\n";

    // -------------------------------------------------------------------------
    // 场景 1：亮度统计
    // -------------------------------------------------------------------------
    {
        auto algo = factory.Create("BrightnessStat");
        assert(algo != nullptr);

        if (auto err = algo->Initialize(); err)
        {
            std::cerr << "Initialize failed: " << err->Message() << "\n";
            return 1;
        }

        // 构造 4x4 单通道灰度图（全填充 128）
        auto img = ImageBuffer::Create(4, 4, 1);
        std::memset(img->data.get(), 128, img->ByteSize());

        AlgorithmExecuteData data;
        data.inputs.SetImage("src", img);   // 零拷贝传递

        if (auto err = algo->Execute(data); err)
        {
            std::cerr << "Execute failed: " << err->Message() << "\n";
            return 1;
        }

        double* mean = data.outputs.Get<double>("mean_brightness");
        assert(mean && *mean == 128.0);
        std::cout << "BrightnessStat mean = " << *mean << "\n";

        algo->Uninitialize();
    }

    // -------------------------------------------------------------------------
    // 场景 2：图像翻转（水平）
    // -------------------------------------------------------------------------
    {
        auto algo = factory.Create("Flip");
        assert(algo != nullptr);

        // 构造 1x3 单通道图，像素值：[10, 20, 30]
        auto img = ImageBuffer::Create(3, 1, 1);
        img->data.get()[0] = 10;
        img->data.get()[1] = 20;
        img->data.get()[2] = 30;

        AlgorithmExecuteData data;
        data.inputs.SetImage("src", img);
        data.inputs.Set("horizontal", true);

        if (auto err = algo->Execute(data); err)
        {
            std::cerr << "Flip failed: " << err->Message() << "\n";
            return 1;
        }

        auto dst = data.outputs.GetImage("dst");
        assert(dst && !dst->Empty());
        // 翻转后应为 [30, 20, 10]
        assert(dst->data.get()[0] == 30);
        assert(dst->data.get()[1] == 20);
        assert(dst->data.get()[2] == 10);
        std::cout << "Flip result: ["
                  << (int)dst->data.get()[0] << ", "
                  << (int)dst->data.get()[1] << ", "
                  << (int)dst->data.get()[2] << "]\n";
    }

    // -------------------------------------------------------------------------
    // 场景 3：RAW 文件 -> Bayer 解码 -> JPEG
    // -------------------------------------------------------------------------
    {
        auto algo = factory.Create("RawDebayerToJpeg");
        assert(algo != nullptr);

        const int width = 5496;
        const int height = 3672;
        const std::string raw_path = "2026-04-27_17_57_08_370.raw";
        const std::string jpg_path = "2026-04-27_17_57_08_370.jpg";

        // 先生成一份 demo RAW 数据并写盘（Bayer RG pattern）
        // std::vector<unsigned char> demo_raw(static_cast<size_t>(width * height));
        // for (int y = 0; y < height; ++y)
        // {
        //     for (int x = 0; x < width; ++x)
        //     {
        //         demo_raw[static_cast<size_t>(y * width + x)] =
        //             static_cast<unsigned char>((x * 8 + y * 8) & 0xFF);
        //     }
        // }
        // {
        //     std::ofstream raw_out(raw_path, std::ios::binary);
        //     raw_out.write(reinterpret_cast<const char*>(demo_raw.data()),
        //                   static_cast<std::streamsize>(demo_raw.size()));
        // }

        // 1) 从 raw 文件读取到 unsigned char* 数据中
        std::ifstream raw_in(raw_path, std::ios::binary | std::ios::ate);
        assert(raw_in.is_open());
        const auto file_size = raw_in.tellg();
        raw_in.seekg(0, std::ios::beg);
        assert(file_size == static_cast<std::streamoff>(width * height));

        std::unique_ptr<unsigned char[]> raw_data(new unsigned char[static_cast<size_t>(file_size)]);
        raw_in.read(reinterpret_cast<char*>(raw_data.get()), file_size);
        raw_in.close();

        // 2) 构建参数，把数据内容传入算法实现类
        auto raw_img = ImageBuffer::Wrap(raw_data.get(), width, height, 1, 1, width);
        AlgorithmExecuteData data;
        data.inputs.SetImage("raw_bayer", raw_img);
        data.inputs.Set("bayer_code", cv::COLOR_BayerRG2BGR);
        data.inputs.Set("jpg_quality", 90);
        data.inputs.Set("jpg_output_path", jpg_path);

        // 3) + 4) + 5) 在算法内执行：debayer、jpg 压缩、写文件
        if (auto err = algo->Execute(data); err)
        {
            std::cerr << "RawDebayerToJpeg failed: " << err->Message() << "\n";
            return 1;
        }

        const int* jpg_size = data.outputs.Get<int>("jpg_size");
        const std::string* out_path = data.outputs.Get<std::string>("jpg_path");
        assert(jpg_size && *jpg_size > 0);
        assert(out_path && *out_path == jpg_path);
        std::cout << "RawDebayerToJpeg done: " << *out_path
                  << ", size=" << *jpg_size << " bytes\n";
    }

    std::cout << "All demos passed.\n";
    return 0;
}
