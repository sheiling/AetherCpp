#pragma once

/**
 * @file lighten_steps.h
 * @brief 点亮检测综合调度步骤（typeName: "lighten.light_detect"）声明。
 *
 * LightenDetectStep 在一次 execute 中按需串联：
 *   黑屏判定、黑屏亮灯、纯色/串色、混色、串灯检测。
 * 共享主相机黑/白参考图与各阶段轮廓文件，结果写入 FlowContext。
 * 实现细节见 lighten_steps.cpp。
 */

#include <map>
#include <memory>
#include <string>

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/include/algorithm_data/lightendetector_data.h>

#include <scheduler/step/i_step.h>

namespace common {
namespace device {
class DeviceManager;
} // namespace device
} // namespace common

namespace step {

// 点亮检测综合调度步骤（typeName: "lighten.light_detect"）。
// 根据 JSON 配置按需执行：黑屏判定、黑屏亮灯、纯色/串色检测、混色检测、串灯检测；
// 共享黑/白参考图采集，并将各阶段结果写入 FlowContext。
class LightenDetectStep final : public scheduler::IStep {
public:
    // 从步骤 JSON 解析软屏键。
    void reflectFromJson(const nlohmann::json& j) override;
    // 按顺序执行已启用的检测子阶段，结果写入 ctx。
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "lighten.light_detect"; }

private:
    // 从运行时 JSON 加载 lightDelayMs、featureFlags 及各相机算法参数（execute 入口从 KEY_LIGHT_DETECT_STEP_CONFIG 读取）。
    bool loadParamsFromJson(const nlohmann::json& root);

    // --- 全局输出路径 ---
    // 轮廓/图片：m_image_path / cam_path / filename；CSV：m_data_path / filename
    std::string cameraOutputPath(const std::string& cam_path, const std::string& filename) const;
    std::string sessionDir(const std::string& basePath, std::string& subPath) const;

    // --- 共享参考图 / 轮廓路径（execute 前置判定与采集） ---
    std::string resolveReferenceCameraName() const;
    bool needsWhiteReferenceImage() const;
    bool needsBlackReferenceImage() const;
    const common::algorithm::LightenDetectAlgoParams* FindAlgoParams(const std::string& camera_key) const;
    const common::algorithm::LightenDetectAlgoParams& mainAlgoParams() const;
    std::string outputDataRoot() const;
    void assignWhiteContourOutputPaths(
        const std::string& cam_path,
        std::string* red_out,
        std::string* green_out,
        std::string* blue_out) const;
    common::algorithm::WhiteContourGenParams buildWhiteContourGenParams(
        const std::shared_ptr<common::algorithm::ImageBuffer>& white_image) const;
    common::algorithm::PureColorDetectParams preparePureColorParams(
        const common::algorithm::PureColorDetectParams& base,
        const std::shared_ptr<common::algorithm::ImageBuffer>& image,
        bool is_main_camera,
        const std::string& cam_path,
        size_t color_idx) const;
    common::algorithm::MixColorVariationDetectParams prepareMixColorParams(
        const common::algorithm::MixColorVariationDetectParams& base,
        const std::shared_ptr<common::algorithm::ImageBuffer>& image,
        const std::string& cam_path) const;
    common::algorithm::StringLightDetectParams prepareStringLightParams(
        const common::algorithm::StringLightDetectParams& base,
        std::shared_ptr<common::algorithm::ImageBuffer> left_image,
        std::shared_ptr<common::algorithm::ImageBuffer> right_image,
        const std::string& cam_path,
        size_t color_idx) const;
    common::algorithm::BlackScreenLightDetectParams prepareBlackLightParams(
        const common::algorithm::BlackScreenLightDetectParams& base,
        std::shared_ptr<common::algorithm::ImageBuffer> bg_image,
        const std::string& cam_path) const;
    scheduler::StepResult runBlackScreenDetectSync(
        const std::string& camera_key,
        const std::shared_ptr<common::algorithm::ImageBuffer>& white_image,
        const std::shared_ptr<common::algorithm::ImageBuffer>& black_image,
        int& is_black_screen_out) const;

    // --- 各算法保存选项（execute 各子阶段调用） ---
    void applyBlackScreenSaveOptions(common::algorithm::BlackScreenDetectParams& p) const;
    void applyBlackLightSaveOptions(common::algorithm::BlackScreenLightDetectParams& p, const std::string& cam_path) const;
    void applyWhiteContourSaveOptions(common::algorithm::WhiteContourGenParams& p, const std::string& cam_path) const;
    void applyPureColorSaveOptions(common::algorithm::PureColorDetectParams& p, const std::string& cam_path) const;
    void applyMixColorSaveOptions(common::algorithm::MixColorVariationDetectParams& p, const std::string& cam_path) const;
    void applyStringLightSaveOptions(common::algorithm::StringLightDetectParams& p,
                                     const std::string& cam_path,
                                     size_t color_idx) const;

    // --- 软屏显示 ---
    // DeviceManager 中软屏设备的配置键（JSON: softScreenKey）。
    std::string m_soft_screen_key;

    // 点亮延时（毫秒）；execute 入口在加载参数后、执行检测前先等待（JSON: lightDelayMs；由 KEY_LIGHT_DETECT_STEP_CONFIG 加载）。
    int m_lighten_delay_ms{ 0 };

    // 子功能启用标志，决定本工步执行哪些检测子阶段（由 KEY_LIGHT_DETECT_STEP_CONFIG 的 featureFlags 加载）。
    common::algorithm::LightenDetectFeatureFlags m_feature_flags;

    // 当前工件序列号（SN），由 execute 从 FlowContext KEY_BOARD_SN 读取；缺失时用当前时间戳，作用于整个工步。
    std::string m_sn;

    // 主相机名称：loadParamsFromJson 取 KEY_LIGHT_DETECT_STEP_CONFIG.cameras 的首个 key。
    std::string m_main_camera_name;

    // 各相机算法参数（key 为 camera_name；由 KEY_LIGHT_DETECT_STEP_CONFIG 的 cameras 加载）。
    std::map<std::string, common::algorithm::LightenDetectAlgoParams> m_algo_params_by_camera;

    // --- 运行时输出目录（execute 内按 m_sn + 时间戳生成） ---
    std::string m_data_path;
    std::string m_image_path;
};

} // namespace step
