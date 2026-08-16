#include "lighten_steps.h"

#include <array>
#include <algorithm>
#include <chrono>
#include <exception>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <thread>
#include <utility>
#include <vector>

#include <common/base/algorithm/algorithm_factory.h>
#include <common/core/container/application.h>
#include <common/core/messagecenter/message_center.h>
#include <common/core/device/displayControl/software_screen/software_screen_base.h>
#include <common/utility/errors/include/error.h>
#include <common/utility/json_tool/reflector.h>
#include <common/utility/string_tool/time_format.hpp>
#include <common/utility/file_tool/file_manager.hpp>
#include <common/plugins/lightendetector/algorithm/pure_color_detect_algorithm.h>
#include <common/plugins/lightendetector/algorithm/mix_color_variation_detect_algorithm.h>
#include <common/plugins/lightendetector/service/shot_meter.h>
#include <common/plugins/lightendetector/algorithm/string_light_detect_algorithm.h>

#include "lighten_device_ops.h"

#include <scheduler/step/step_factory.h>

/**
 * @file lighten_steps.cpp
 * @brief 点亮检测综合调度步骤（LightenDetectStep）实现。
 *
 * 本模块是 LightenDetector 插件的核心调度层，串联软屏、相机与多种缺陷检测算法，
 * 在一次工步 execute 中按需完成以下子阶段（均可通过 JSON 独立启用/禁用）：
 *
 *   | 子阶段       | 算法名                  | FlowContext 输出键            |
 *   |-------------|-------------------------|-------------------------------|
 *   | 纯色检测     | PureColorDetect         | KEY_PURE_DEAD/DARK/BRIGHT_*_OUT（R/G/B） |
 *   | 串色检测     | PureColorDetect（内置）  | KEY_COLOR_VAR_*_OUT（R/G/B）             |
 *   | 混色检测     | MixColorVariationDetect | KEY_MIX_COLOR_*_OUT（R/G/B）             |
 *   | 串灯检测     | StringLightDetect       | KEY_STRING_LIGHT_*_OUT（R/G/B）          |
 *   | 黑屏亮灯     | BlackScreenLightDetect  | KEY_BLACK_SCREEN_LIGHT_OUT    |
 *   | 黑屏判定     | BlackScreenDetect       | KEY_IS_BLACK_SCREEN_OUT       |
 *
 * 共享资源（单次 execute 内采集一次、多算法复用）：
 *   - main_black_image  主相机黑场参考图（黑屏亮灯、黑屏判定）
 *   - main_white_image  主相机白场参考图（混色、黑屏判定；纯色检测不依赖白场）
 *   - 轮廓文件     白场 RGB 轮廓（混色/黑屏亮灯复用）；纯色 RGB 轮廓（串色/串灯复用）
 *   - 轮廓策略     纯色/白屏强制生成；串色复用纯色轮廓；串灯按纯色开关复用轮廓或 ROI；混色/黑屏亮灯复用白屏轮廓
 *
 * 并发策略：
 *   - 相机采图：打屏后在主线程同步 GrabRgbBuffer（与串灯检测一致），确保采图时序与当前打屏画面严格对应
 *   - 算法检测池 pure/mix_algo_futures：采图完成后异步提交算法，与后续打屏/采图并行
 *   - 延后检测池 black/string_light_futures：依赖轮廓就绪后再执行
 */
namespace step {
namespace {

using json = nlohmann::json;

using common::algorithm::kPureColorChannelCount;
using common::algorithm::kLightenDefectNotifyKindCount;
using common::algorithm::kLightenDefectOutKeys;
using common::algorithm::LightenDefectNotifyKind;
using common::algorithm::kLightenDetectProgressColorNone;
using common::algorithm::LightenDetectProgressKind;
using common::algorithm::LightenDetectProgressStatus;
using common::algorithm::LightenDetectFeatureFlags;

// =============================================================================
// 错误处理
// 统一将 common::errors::ErrorPtr 转为可读文本或 scheduler::StepResult。
// =============================================================================

// 将 ErrorPtr 格式化为紧凑文本，空指针返回空字符串。
std::string ErrText(const common::errors::ErrorPtr& e)
{
    return e ? e->Format(common::errors::FormatStyle::kCompact) : std::string{};
}

// 将 ErrorPtr 转换为 StepResult::fail，便于在步骤中统一返回。
scheduler::StepResult FailByErr(const common::errors::ErrorPtr& e, int fallback_code = -1)
{
    return scheduler::StepResult::fail(ErrText(e), e ? e->Code() : fallback_code);
}

// =============================================================================
// 算法参数构建（execute 阶段调用，将本次采图绑定到算法参数）
// =============================================================================

// 基于步骤配置与正常/黑底图像构建黑屏判定参数。
common::algorithm::BlackScreenDetectParams BuildBlackScreenParams(
    const common::algorithm::BlackScreenDetectParams& base,
    std::shared_ptr<common::algorithm::ImageBuffer> normal_image,
    std::shared_ptr<common::algorithm::ImageBuffer> black_image)
{
    auto p = base;
    p.normal_image = std::move(normal_image);
    p.black_image  = std::move(black_image);
    return p;
}

// 基于步骤配置与共享黑底参考图构建黑屏亮灯检测参数。
common::algorithm::BlackScreenLightDetectParams BuildBlackLightParams(const common::algorithm::BlackScreenLightDetectParams& base, std::shared_ptr<common::algorithm::ImageBuffer> bg_image)
{
    auto p = base;
    p.bg_image = std::move(bg_image);
    return p;
}

// 将 R/G/B 分色缺陷点写入 FlowContext，并在 ctx.set 后通知首页刷新对应单元格。
// execute 入口已调用 ClearRgbDefectOutputs，同次工步内仅合并多相机结果。
template<typename T>
void AppendDefectOutput(
    scheduler::FlowContext& ctx,
    LightenDefectNotifyKind defect_kind,
    size_t color_idx,
    const std::vector<T>& data)
{
    const char* key = LIGHTEN_DEFECT_OUT_KEY(defect_kind, color_idx);
    int count = 0;
    if (data.empty()) {
        if (ctx.has(key)) {
            return;
        }
        ctx.set(key, std::vector<T>{}, scheduler::VariableScope::FLOW);
    } else {
        auto merged = ctx.tryGet<std::vector<T>>(key).value_or(std::vector<T>{});
        merged.insert(merged.end(), data.begin(), data.end());
        count = static_cast<int>(merged.size());
        ctx.set(key, std::move(merged), scheduler::VariableScope::FLOW);
    }
    MessageCenter::Center::Notify(
        EVENT_LIGHTEN_DEFECT_COUNT_RESULT,
        LIGHTEN_DEFECT_COUNT_NOTIFY_ARGS(defect_kind, color_idx, count));
}

// 清空本工步 execute 写入 FlowContext 的全部输出键，避免 Flow 复用时残留上次检测数据。
void ClearRgbDefectOutputs(scheduler::FlowContext& ctx)
{
    for (size_t kind_idx = 0; kind_idx < kLightenDefectNotifyKindCount; ++kind_idx) {
        for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
            ctx.remove(kLightenDefectOutKeys[kind_idx][color_idx], scheduler::VariableScope::FLOW);
        }
    }
    ctx.remove(KEY_IS_BLACK_SCREEN_OUT, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_BLACK_SCREEN_LIGHT_OUT, scheduler::VariableScope::FLOW);
}

// =============================================================================
// 异步算法执行与线程池辅助
// Run*Algorithm 在 std::async 线程中调用 AlgorithmFactory，返回统一 Result 结构。
// =============================================================================

// 将算法 PureDetectColor 映射为软屏颜色（White 无对应打屏色，不参与 kDetectColorSequence）。
common::device::SoftScreenColor ToSoftScreenColor(common::algorithm::PureDetectColor c)
{
    switch (c) {
    case common::algorithm::PureDetectColor::Green: return common::device::COLOR_GREEN;
    case common::algorithm::PureDetectColor::Blue: return common::device::COLOR_BLUE;
    case common::algorithm::PureDetectColor::White: return common::device::COLOR_WHITE;
    default: return common::device::COLOR_RED;
    }
}

// 纯色/混色/串灯检测按 R → G → B 依次打屏并采图。
const std::array<common::algorithm::PureDetectColor, 3> kDetectColorSequence{
    common::algorithm::PureDetectColor::Red,
    common::algorithm::PureDetectColor::Green,
    common::algorithm::PureDetectColor::Blue,
};

// 将检测颜色映射为 kDetectColorSequence 下标（R=0, G=1, B=2）。
size_t ColorIndex(common::algorithm::PureDetectColor color)
{
    switch (color) {
    case common::algorithm::PureDetectColor::Green: return 1;
    case common::algorithm::PureDetectColor::Blue: return 2;
    default: return 0;
    }
}

void NotifyDetectProgress(LightenDetectProgressKind kind, int colorIdx, LightenDetectProgressStatus status)
{
    MessageCenter::Center::Notify(
        EVENT_LIGHTEN_DETECT_PROGRESS,
        LIGHTEN_DETECT_PROGRESS_NOTIFY_ARGS(kind, colorIdx, status));
}

void NotifyDetectProgressSimple(LightenDetectProgressKind kind, LightenDetectProgressStatus status)
{
    NotifyDetectProgress(kind, kLightenDetectProgressColorNone, status);
}

// 异步纯色/串色算法任务的统一返回结构（供 pure_algo_futures_by_color 使用）。
struct PureColorAlgoResult {
    common::errors::ErrorPtr err;
    std::vector<common::algorithm::PureColorDefectPoint> deads;           // 死灯
    std::vector<common::algorithm::PureColorDefectPoint> darks;           // 暗灯
    std::vector<common::algorithm::PureColorDefectPoint> brights;         // 亮灯
    std::vector<common::algorithm::ColorVariationDefectPoint> color_variations; // 串色
};

// 异步混色算法任务的统一返回结构（供 mix_algo_futures 使用）。
struct MixColorAlgoResult {
    common::errors::ErrorPtr err;
    std::vector<common::algorithm::MixColorDefectPoint> mix_colors;
};

// 延后检测池（黑屏亮灯）任务的统一返回结构。
struct BlackLightAlgoResult {
    common::errors::ErrorPtr err;
    std::vector<common::algorithm::BlackScreenLightDefectPoint> points;
};

// 延后检测池（串灯）任务的统一返回结构。
struct StringLightAlgoResult {
    common::errors::ErrorPtr err;
    std::vector<common::algorithm::StringLightDefectPoint> points;
};

// 在独立线程中执行 PureColorDetect 算法。
// 通过 AlgorithmFactory 创建算法实例，输入 params，输出 deads/darks/brights/color_variations。
PureColorAlgoResult RunPureColorAlgorithm(const common::algorithm::PureColorDetectParams& params)
{
    PureColorAlgoResult out{};
    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", params);
    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("PureColorDetect");
    if (!algo) {
        out.err = common::errors::New(-10, "Algorithm PureColorDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }
    auto deads = data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("deads");
    auto darks = data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("darks");
    auto brights = data.outputs.Get<std::vector<common::algorithm::PureColorDefectPoint>>("brights");
    auto color_variations =
        data.outputs.Get<std::vector<common::algorithm::ColorVariationDefectPoint>>("color_variations");
    out.deads = deads ? *deads : std::vector<common::algorithm::PureColorDefectPoint>{};
    out.darks = darks ? *darks : std::vector<common::algorithm::PureColorDefectPoint>{};
    out.brights = brights ? *brights : std::vector<common::algorithm::PureColorDefectPoint>{};
    out.color_variations = color_variations ? *color_variations
                                            : std::vector<common::algorithm::ColorVariationDefectPoint>{};
    return out;
}

// 在独立线程中执行 MixColorVariationDetect 算法。
MixColorAlgoResult RunMixColorAlgorithm(const common::algorithm::MixColorVariationDetectParams& params)
{
    MixColorAlgoResult out{};
    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", params);
    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("MixColorVariationDetect");
    if (!algo) {
        out.err = common::errors::New(-10, "Algorithm MixColorVariationDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }
    auto mix_colors = data.outputs.Get<std::vector<common::algorithm::MixColorDefectPoint>>("mix_colors");
    out.mix_colors = mix_colors ? *mix_colors : std::vector<common::algorithm::MixColorDefectPoint>{};
    return out;
}

// 在独立线程中执行 BlackScreenLightDetect 算法。
BlackLightAlgoResult RunBlackLightAlgorithm(common::algorithm::BlackScreenLightDetectParams params)
{
    BlackLightAlgoResult out{};
    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", std::move(params));
    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("BlackScreenLightDetect");
    if (!algo) {
        out.err = common::errors::New(-10, "Algorithm BlackScreenLightDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }
    const auto* result =
        data.outputs.Get<std::vector<common::algorithm::BlackScreenLightDefectPoint>>("black_screen_lights");
    out.points = result ? *result : std::vector<common::algorithm::BlackScreenLightDefectPoint>{};
    return out;
}

// 在独立线程中执行 StringLightDetect 算法。
StringLightAlgoResult RunStringLightAlgorithm(common::algorithm::StringLightDetectParams params)
{
    StringLightAlgoResult out{};
    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", std::move(params));
    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("StringLightDetect");
    if (!algo) {
        out.err = common::errors::New(-10, "Algorithm StringLightDetect not registered");
        return out;
    }
    if (auto err = algo->Execute(data)) {
        out.err = err;
        return out;
    }
    const auto* result = data.outputs.Get<std::vector<common::algorithm::StringLightDefectPoint>>("string_lights");
    out.points = result ? *result : std::vector<common::algorithm::StringLightDefectPoint>{};
    return out;
}

// 等待 future 列表完成，成功后将各任务结果通过 merge_fn 回调合并到 ctx。
template<typename TResult, typename MergeFn>
scheduler::StepResult WaitFutureVectorAndMerge(std::vector<std::future<TResult>>& futures, MergeFn merge_fn)
{
    for (auto& future : futures) {
        TResult result = future.get();
        if (result.err) {
            return FailByErr(result.err);
        }
        merge_fn(result);
    }
    futures.clear();
    return scheduler::StepResult::ok();
}

} // namespace

// =============================================================================
// LightenDetectStep 配置解析（reflectFromJson / loadParamsFromJson）
// reflectFromJson 解析 softScreenKey；运行时参数在 execute 入口从 KEY_LIGHT_DETECT_STEP_CONFIG 加载。
// =============================================================================

// 从运行时 JSON 加载 lightDelayMs、featureFlags 及各相机算法参数。
bool LightenDetectStep::loadParamsFromJson(const json& root)
{
    if (!root.is_object()) {
        return false;
    }

    m_lighten_delay_ms = 0;
    fromJson(root, "lightDelayMs", m_lighten_delay_ms);
    if (m_lighten_delay_ms < 0) {
        m_lighten_delay_ms = 0;
    }

    if (!root.contains("featureFlags")) {
        return false;
    }

    m_feature_flags = {};
    fromJson(root, "featureFlags", m_feature_flags);

    if (!root.contains("cameras") || !root["cameras"].is_object()) {
        return false;
    }
    const json& cameras_params = root["cameras"];
    if (cameras_params.empty()) {
        return false;
    }

    m_algo_params_by_camera.clear();
    m_main_camera_name.clear();
    for (auto it = cameras_params.begin(); it != cameras_params.end(); ++it) {
        if (m_main_camera_name.empty()) {
            m_main_camera_name = it.key();
        }
        common::algorithm::LightenDetectAlgoParams params;
        fromJson(cameras_params, it.key(), params);
        m_algo_params_by_camera.emplace(it.key(), std::move(params));
    }
    return true;
}

// 从步骤 JSON 解析软屏键。
void LightenDetectStep::reflectFromJson(const json& j)
{
    // DeviceManager 中软屏设备的配置键（JSON: softScreenKey）。
    fromJson(j, "softScreenKey", m_soft_screen_key);
}

// =============================================================================
// 全局输出路径
// 轮廓/图片：m_image_path / cam_path / filename；CSV：m_data_path / filename
// =============================================================================

// 拼接图像输出完整路径：m_image_path / [cam_path /] filename。
std::string LightenDetectStep::cameraOutputPath(const std::string& cam_path, const std::string& filename) const
{
    const std::string base = cam_path.empty()
        ? m_image_path
        : common::algorithm::JoinPathAndFileName(m_image_path, cam_path);
    return common::algorithm::JoinPathAndFileName(base, filename);
}

// 拼接并创建会话子目录（basePath + subPath），目录不存在时自动创建。
std::string LightenDetectStep::sessionDir(const std::string& basePath, std::string& subPath) const
{
    std::string path;
    if (basePath.empty()) {
        path = subPath;
    } else if (subPath.empty()) {
        path = basePath;
    } else if (basePath.back() == '/' || basePath.back() == '\\') {
        path = basePath + subPath;
    } else {
        path = basePath + "/" + subPath;
    }
    if (!path.empty()) {
        common::utility::FileManager::createDir(path);
    }
    return path;
}

// 配置黑屏判定算法参数（此处关闭子结构体原图开关）。
void LightenDetectStep::applyBlackScreenSaveOptions(
    common::algorithm::BlackScreenDetectParams& p) const
{
    p.adjustFileName();
    p.save_original_image_flag = false;
    p.black_image_file_name.clear();
    p.white_image_file_name.clear();
}

// 为白场 RGB 三色轮廓文件设置输出路径（混色/黑屏亮灯算法复用，供点定位）。
void LightenDetectStep::assignWhiteContourOutputPaths(
    const std::string& cam_path,
    std::string* red_out,
    std::string* green_out,
    std::string* blue_out) const
{
    common::algorithm::WhiteContourGenParams names{};
    names.adjustFileName();
    if (red_out) {
        *red_out = cameraOutputPath(cam_path, names.white_red_contour_file_name);
    }
    if (green_out) {
        *green_out = cameraOutputPath(cam_path, names.white_green_contour_file_name);
    }
    if (blue_out) {
        *blue_out = cameraOutputPath(cam_path, names.white_blue_contour_file_name);
    }
}

// 配置黑屏亮灯检测的保存路径；复用 white_green.contour 供点定位。
void LightenDetectStep::applyBlackLightSaveOptions(
    common::algorithm::BlackScreenLightDetectParams& p, const std::string& cam_path) const
{
    p.adjustFileName();
    p.save_original_image_flag = false;
    assignWhiteContourOutputPaths(cam_path, nullptr, &p.white_green_contour_file_name, nullptr);
    p.black_image_file_name.clear();
    p.black_screen_light_png_file_name = p.save_result_image_flag
        ? cameraOutputPath(cam_path, p.black_screen_light_png_file_name)
        : std::string{};
}

// 配置白场参考图及 RGB 轮廓文件的保存路径（ShotMeter::RunWhite 异步生成）。
void LightenDetectStep::applyWhiteContourSaveOptions(
    common::algorithm::WhiteContourGenParams& p, const std::string& cam_path) const
{
    p.adjustFileName();
    p.white_image_file_name = p.save_original_image_flag
        ? cameraOutputPath(cam_path, p.white_image_file_name)
        : std::string{};
    assignWhiteContourOutputPaths(
        cam_path,
        &p.white_red_contour_file_name,
        &p.white_green_contour_file_name,
        &p.white_blue_contour_file_name);
}

// =============================================================================
// 共享参考图辅助
// 参考图在主相机上采集一次，在 Build*Params 时注入各算法参数。
// =============================================================================

// 解析主相机名称：loadParamsFromJson 取 cameras 对象的首个 key。
std::string LightenDetectStep::resolveReferenceCameraName() const
{
    return m_main_camera_name;
}

// 按相机名查找已加载的算法参数，未找到返回 nullptr。
const common::algorithm::LightenDetectAlgoParams* LightenDetectStep::FindAlgoParams(const std::string& camera_key) const
{
    const auto it = m_algo_params_by_camera.find(camera_key);
    return it != m_algo_params_by_camera.end() ? &it->second : nullptr;
}

// 返回主相机的算法参数；loadParamsFromJson 成功后保证存在。
const common::algorithm::LightenDetectAlgoParams& LightenDetectStep::mainAlgoParams() const
{
    return m_algo_params_by_camera.at(resolveReferenceCameraName());
}

// 返回 CSV 等数据输出根目录；优先 output_data_root，为空时回退 output_image_root。
std::string LightenDetectStep::outputDataRoot() const
{
    const auto& p = mainAlgoParams();
    return p.output_data_root.empty() ? p.output_image_root : p.output_data_root;
}

// 是否需要采集白场参考图并强制生成白场轮廓（混色/黑屏亮灯复用；纯色检测不需要）。
bool LightenDetectStep::needsWhiteReferenceImage() const
{
    return m_feature_flags.anyMixColor() || m_feature_flags.black_screen || m_feature_flags.black_light;
}

// 是否需要采集黑场参考图（黑屏判定 / 黑屏亮灯打黑屏；不考虑去背景）。
bool LightenDetectStep::needsBlackReferenceImage() const
{
    return m_feature_flags.black_screen || m_feature_flags.black_light;
}

// =============================================================================
// 轮廓路径与参考图辅助
// =============================================================================

// 构造白场轮廓生成（ShotMeter::RunWhite）所需的算法参数。
// 由 white_contour_future 异步线程调用，生成 white_red/green/blue.contour 供后续复用。
common::algorithm::WhiteContourGenParams LightenDetectStep::buildWhiteContourGenParams(
    const std::shared_ptr<common::algorithm::ImageBuffer>& white_image) const
{
    const auto* algo_params = FindAlgoParams(resolveReferenceCameraName());
    auto p = algo_params ? algo_params->white : common::algorithm::WhiteContourGenParams{};
    if (algo_params) {
        const auto& capture = algo_params->black_screen;
        if (p.led_w <= 0) {
            p.led_w = capture.led_w;
        }
        if (p.led_h <= 0) {
            p.led_h = capture.led_h;
        }
        if (p.cam_exposure <= 0) {
            p.cam_exposure = capture.white_cam_exposure;
        }
    }
    p.image = white_image;
    applyWhiteContourSaveOptions(p, algo_params ? algo_params->cameraPath : std::string{});
    return p;
}

// 组装纯色检测参数：仅主相机启用串色检查；轮廓路径由 applyPureColorSaveOptions 注入，副相机清空以避免重复落盘。
// image 以 shared_ptr 浅拷贝传入，可与混色检测并行共享同一帧像素数据（算法侧只读）。
common::algorithm::PureColorDetectParams LightenDetectStep::preparePureColorParams(
    const common::algorithm::PureColorDetectParams& base,
    const std::shared_ptr<common::algorithm::ImageBuffer>& image,
    bool is_main_camera,
    const std::string& cam_path,
    size_t color_idx) const
{
    auto p = base;
    p.image = image;
    p.dead_check = m_feature_flags.dead_light[color_idx];
    p.dark_check = m_feature_flags.dark_light[color_idx];
    p.bright_check = m_feature_flags.bright_light[color_idx];
    p.color_check = is_main_camera && m_feature_flags.color_variation[color_idx];
    applyPureColorSaveOptions(p, cam_path);
    if (!is_main_camera) {
        p.pure_contour_file_name.clear();
    }
    return p;
}

// 组装混色检测参数；白场轮廓路径由 applyMixColorSaveOptions 注入，算法内等待 white_contour_future。
// image 以 shared_ptr 浅拷贝传入，可与纯色检测并行共享同一帧像素数据（算法侧只读）。
common::algorithm::MixColorVariationDetectParams LightenDetectStep::prepareMixColorParams(
    const common::algorithm::MixColorVariationDetectParams& base,
    const std::shared_ptr<common::algorithm::ImageBuffer>& image,
    const std::string& cam_path) const
{
    auto p = base;
    p.image = image;
    applyMixColorSaveOptions(p, cam_path);
    return p;
}

// 组装串灯检测参数（奇/偶棋盘格左右分屏图）；纯色开启时复用 contour，否则用 ROI 点定位。
common::algorithm::StringLightDetectParams LightenDetectStep::prepareStringLightParams(
    const common::algorithm::StringLightDetectParams& base,
    std::shared_ptr<common::algorithm::ImageBuffer> left_image,
    std::shared_ptr<common::algorithm::ImageBuffer> right_image,
    const std::string& cam_path,
    size_t color_idx) const
{
    auto p = base;
    p.left_image  = std::move(left_image);
    p.right_image = std::move(right_image);
    applyStringLightSaveOptions(p, cam_path, color_idx);
    return p;
}

// 组装黑屏亮灯检测参数，注入主相机黑场参考图（步骤 2 采集的 main_black_image）。
common::algorithm::BlackScreenLightDetectParams LightenDetectStep::prepareBlackLightParams(
    const common::algorithm::BlackScreenLightDetectParams& base,
    std::shared_ptr<common::algorithm::ImageBuffer> bg_image,
    const std::string& cam_path) const
{
    auto p = BuildBlackLightParams(base, std::move(bg_image));
    applyBlackLightSaveOptions(p, cam_path);
    return p;
}

// 黑屏判定：同步执行（不进入异步池），直接使用主相机已采集的 main_black_image / main_white_image。
// 输出 is_black_screen（0/1）到 KEY_IS_BLACK_SCREEN_OUT。
scheduler::StepResult LightenDetectStep::runBlackScreenDetectSync(
    const std::string& camera_key,
    const std::shared_ptr<common::algorithm::ImageBuffer>& white_image,
    const std::shared_ptr<common::algorithm::ImageBuffer>& black_image,
    int& is_black_screen_out) const
{
    is_black_screen_out = 0;
    if (!black_image || !white_image) {
        return scheduler::StepResult::fail("LightenDetectStep: shared bg/white images not captured", -21);
    }

    const auto* algo_params = FindAlgoParams(camera_key);
    if (!algo_params) {
        return scheduler::StepResult::fail("LightenDetectStep: algo params not found for camera", -20);
    }

    auto algo = common::algorithm::AlgorithmFactory::Instance().Create("BlackScreenDetect");
    if (!algo) {
        return scheduler::StepResult::fail("Algorithm BlackScreenDetect not registered", -10);
    }
    auto params = BuildBlackScreenParams(algo_params->black_screen, white_image, black_image);
    applyBlackScreenSaveOptions(params);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs.Set("params", std::move(params));
    if (auto err = algo->Execute(data)) {
        return FailByErr(err);
    }
    const auto* out = data.outputs.Get<int>("is_black_screen");
    is_black_screen_out = out ? *out : 0;
    return scheduler::StepResult::ok();
}

// 配置纯色/串色检测的输出路径：原图、结果 PNG、过程 CSV、纯色轮廓文件。
void LightenDetectStep::applyPureColorSaveOptions(
    common::algorithm::PureColorDetectParams& p,
    const std::string& cam_path) const
{
    p.adjustFileName();

    p.normal_image_file = p.save_original_image_flag
        ? cameraOutputPath(cam_path, p.normal_image_file)
        : std::string{};

    // 各缺陷类型独立控制是否保存结果图（通过输出路径是否为空表达）。
    const bool save_dead   = p.save_result_image_flag && p.dead_check;
    const bool save_dark   = p.save_result_image_flag && p.dark_check;
    const bool save_bright = p.save_result_image_flag && p.bright_check;
    const bool save_color  = p.save_result_image_flag && p.color_check;
    p.dead_png_file = save_dead ? cameraOutputPath(cam_path, p.dead_png_file) : std::string{};
    p.dark_png_file = save_dark ? cameraOutputPath(cam_path, p.dark_png_file) : std::string{};
    p.bright_png_file = save_bright ? cameraOutputPath(cam_path, p.bright_png_file) : std::string{};
    p.color_variation_png_file_name = save_color
        ? cameraOutputPath(cam_path, p.color_variation_png_file_name)
        : std::string{};

    p.csv_pure_tempdata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_pure_tempdata_file);
    p.csv_pure_origindata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_pure_origindata_file);
    p.csv_pure_comparedata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_pure_comparedata_file);
    p.csv_color_red_comparedata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_color_red_comparedata_file);
    p.csv_color_green_comparedata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_color_green_comparedata_file);
    p.csv_color_blue_comparedata_file = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_color_blue_comparedata_file);

    p.pure_contour_file_name = cameraOutputPath(cam_path, p.pure_contour_file_name);
}

// 配置混色检测结果图与白场 RGB 轮廓路径（算法内读取轮廓做点定位）。
void LightenDetectStep::applyMixColorSaveOptions(
    common::algorithm::MixColorVariationDetectParams& p,
    const std::string& cam_path) const
{
    p.adjustFileName();

    p.mix_color_png_file_name = p.save_result_image_flag
        ? cameraOutputPath(cam_path, p.mix_color_png_file_name)
        : std::string{};
    assignWhiteContourOutputPaths(
        cam_path,
        &p.white_red_contour_file_name,
        &p.white_green_contour_file_name,
        &p.white_blue_contour_file_name);
}

// 配置串灯检测的左右原图、活灯/死灯结果图；纯色开启时注入 contour 路径，否则清空以走 ROI。
void LightenDetectStep::applyStringLightSaveOptions(
    common::algorithm::StringLightDetectParams& p,
    const std::string& cam_path,
    size_t color_idx) const
{
    const bool use_pure_contour = m_feature_flags.runPureAlgoForColor(color_idx);
    p.adjustFileName();

    p.normal_left_image_file_name = p.save_original_image_flag
        ? cameraOutputPath(cam_path, p.normal_left_image_file_name)
        : std::string{};
    p.normal_right_image_file_name = p.save_original_image_flag
        ? cameraOutputPath(cam_path, p.normal_right_image_file_name)
        : std::string{};

    p.string_live_file_name = p.save_result_image_flag
        ? cameraOutputPath(cam_path, p.string_live_file_name)
        : std::string{};
    p.string_dead_file_name = p.save_result_image_flag
        ? cameraOutputPath(cam_path, p.string_dead_file_name)
        : std::string{};
    p.pure_contour_file_name = use_pure_contour
        ? cameraOutputPath(cam_path, p.pure_contour_file_name)
        : std::string{};

    p.csv_left_comparedata_file_name = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_left_comparedata_file_name);
    p.csv_right_comparedata_file_name = common::algorithm::JoinPathAndFileName(m_data_path, p.csv_right_comparedata_file_name);
}

// =============================================================================
// 主执行入口 execute
//
// 整体调度分为 8 个阶段，通过两类异步任务池并行提速：
//   pure/mix_algo_futures  — 算法检测池：主线程采图完成后异步提交纯色/串色/混色算法
//   black/string_light_futures — 延后检测池：黑屏亮灯与串灯（串灯纯色开启时等待轮廓就绪）
//
// 执行顺序：
//   1) 白场参考：打白屏 + 主相机采图；异步 white_contour_future 生成 white_red/green/blue.contour
//   2) 黑场参考：打黑屏 + 主相机采图 → m_bg_image
//   3) 黑屏判定：同步
//   4) 黑屏亮灯：提交延后池；若同时启用混色/黑屏判定（已打白屏）则复用 white_green.contour，否则点定位
//   5) 纯色/混色：R/G/B 三色 × 各相机循环；主线程打屏后立即同步采图，纯色/混色算法异步提交
//      主相机：纯色+串色+混色；副相机：仅纯色（混色依赖白场轮廓，不在副相机执行）
//   6) 串灯：R/G/B 三色循环；左/右棋盘格主线程采图 → 提交延后池；
//      纯色开启时等待算法检测池对应色任务并复用 contour_r/g/b，否则用串灯 ROI 点定位
//   7) 等待延后检测池全部完成
//   8) 合并延后池结果；任一失败则工步失败
// =============================================================================

scheduler::StepResult LightenDetectStep::execute(scheduler::FlowContext& ctx)
{
    try {
        const auto params_opt = ctx.tryGet<json>(KEY_LIGHT_DETECT_STEP_CONFIG);
        if (!params_opt || !loadParamsFromJson(*params_opt)) {
            return scheduler::StepResult::fail("LightenDetectStep: failed to load algo params", -20);
        }
        const std::string main_camera = resolveReferenceCameraName();
        if (m_lighten_delay_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(m_lighten_delay_ms));
        }
        ClearRgbDefectOutputs(ctx);

        m_sn = ctx.tryGet<std::string>(KEY_BOARD_SN)
                   .value_or(common::utility::TimeFormat::GetCurrentTimeStr());
        const auto& main_algo = mainAlgoParams();

        // 生成本次 execute 的数据输出目录与图像输出目录。
        std::string date = common::utility::TimeFormat::GetCurrentDayStr();
        std::string subPath = m_sn + "_" + common::utility::TimeFormat::GetCurrentTimeStr();
        const std::string dataDatePath  = sessionDir(outputDataRoot(), date);
        m_data_path  = sessionDir(dataDatePath, subPath);
        const std::string imageDatePath = sessionDir(main_algo.output_image_root, date);
        m_image_path = sessionDir(imageDatePath, subPath);

        // 主相机白/黑参考图，供黑屏判定、黑屏亮灯、混色等算法复用。
        std::shared_ptr<common::algorithm::ImageBuffer> main_white_image;
        std::shared_ptr<common::algorithm::ImageBuffer> main_black_image;

        // 根据 featureFlags 推断本次需执行的子阶段集合。
        const auto& f = m_feature_flags;
        const bool need_color_cycle = f.needColorCycle();
        if (!f.black_screen && !f.black_light && !f.anyPureAlgo() && !f.anyMixColor() && !f.anyStringLight()) {
            return scheduler::StepResult::fail("LightenDetectStep: no detection configured", -20);
        }
        auto dm = CAPP->make<common::device::DeviceManager>();

        // 按需决定是否采集白/黑参考图（见 needsWhiteReferenceImage / needsBlackReferenceImage）。
        const bool need_white = needsWhiteReferenceImage();
        const bool need_black = needsBlackReferenceImage();
        if ((f.anyMixColor() || f.black_light) && !need_white) {
            return scheduler::StepResult::fail("LightenDetectStep: mix/black light requires white contour", -20);
        }

        const int main_panel_w = main_algo.pure_color[0].led_w;
        const int main_panel_h = main_algo.pure_color[0].led_h;

        std::shared_future<common::errors::ErrorPtr> white_contour_future;
        // 步骤 1/2：白/黑参考图仅在主相机采集（采图参数统一取自 BlackScreenDetectParams）。
        if (need_white || need_black) {
            const auto* algo_params = FindAlgoParams(main_camera);
            if (!algo_params) {
                return scheduler::StepResult::fail("LightenDetectStep: algo params not found for main camera", -20);
            }

            const auto& capture_cfg = algo_params->black_screen;

            if (need_white) {
                const int white_panel_w = capture_cfg.led_w > 0 ? capture_cfg.led_w : main_panel_w;
                const int white_panel_h = capture_cfg.led_h > 0 ? capture_cfg.led_h : main_panel_h;

                auto white_display = MakeBaseSoftDisplay(m_soft_screen_key, white_panel_w, white_panel_h, capture_cfg.disp_delay_ms);
                white_display.color      = common::device::COLOR_WHITE;
                white_display.brightness = capture_cfg.white_disp_brightness;
                if (auto err = SoftDisplay(dm, white_display)) {
                    return FailByErr(err);
                }
                if (auto err = GrabRgbBuffer(dm, main_camera, capture_cfg.white_cam_exposure, &main_white_image)) {
                    return FailByErr(err);
                }
            }

            if (need_black) {
                const int black_panel_w = capture_cfg.led_w > 0 ? capture_cfg.led_w : main_panel_w;
                const int black_panel_h = capture_cfg.led_h > 0 ? capture_cfg.led_h : main_panel_h;

                auto black_display = MakeBaseSoftDisplay(m_soft_screen_key, black_panel_w, black_panel_h, capture_cfg.disp_delay_ms);
                black_display.color      = common::device::COLOR_BLACK;
                black_display.brightness = capture_cfg.black_disp_brightness;
                if (auto err = SoftDisplay(dm, black_display)) {
                    return FailByErr(err);
                }
                if (auto err = GrabRgbBuffer(dm, main_camera, capture_cfg.black_cam_exposure, &main_black_image)) {
                    return FailByErr(err);
                }
            }
        }

        if (need_white && !main_white_image) {
            return scheduler::StepResult::fail("LightenDetectStep: main camera white reference missing", -21);
        }

        // 白场轮廓异步生成，与后续纯色/混色/黑屏亮灯并行；依赖方通过 white_contour_future.get() 等待。
        if (need_white && main_white_image) {
            white_contour_future = std::async(std::launch::async, [this, main_white_image]() {
                auto wp = buildWhiteContourGenParams(main_white_image);
                common::algorithm::ShotMeter shot_meter;
                common::algorithm::RgbMeterResult rgb{};
                return shot_meter.RunWhite(wp, &rgb);
            }).share();
        }

        // --- 两类异步任务池（相机采图不在此池，见步骤 5 主线程同步采图） ---
        // algo_pool_mutex 保护 pure/mix_algo_futures 向量的并发 push。
        // pure_algo_futures_by_color：纯色算法按 R/G/B 分桶，供串灯阶段按色等待并复用对应 contour。
        // mix_algo_futures_by_color：混色算法按 R/G/B 分桶（仅主相机提交）；混色任务内部等待 white_contour_future 就绪。
        // black_light_futures / string_light_futures_by_color：延后检测池，与采图/算法检测并行执行。
        std::array<std::vector<std::future<PureColorAlgoResult>>, kPureColorChannelCount> pure_algo_futures_by_color;
        std::array<std::vector<std::future<MixColorAlgoResult>>, kPureColorChannelCount> mix_algo_futures_by_color;
        std::vector<std::future<BlackLightAlgoResult>> black_light_futures;
        std::array<std::vector<std::future<StringLightAlgoResult>>, kPureColorChannelCount> string_light_futures_by_color;
        std::mutex algo_pool_mutex;   // 保护 pure/mix_algo_futures 向量的并发 push
        std::mutex ctx_merge_mutex;   // 保护串灯阶段 wait_pure_algo_for_color 合并结果到 ctx

        // 按 featureFlags 与当前颜色下标写入对应 R/G/B 输出键并通知 UI。
        auto merge_pure_algo_result = [&](size_t color_idx, const PureColorAlgoResult& result) {
            if (f.dead_light[color_idx]) {
                AppendDefectOutput(ctx, LightenDefectNotifyKind::DeadLed, color_idx, result.deads);
            }
            if (f.dark_light[color_idx]) {
                AppendDefectOutput(ctx, LightenDefectNotifyKind::DimLed, color_idx, result.darks);
            }
            if (f.bright_light[color_idx]) {
                AppendDefectOutput(ctx, LightenDefectNotifyKind::BrightLed, color_idx, result.brights);
            }
            if (f.color_variation[color_idx]) {
                AppendDefectOutput(ctx, LightenDefectNotifyKind::ColorVariation, color_idx, result.color_variations);
            }
        };

        auto wait_pure_algo_futures = [&]() -> scheduler::StepResult {
            for (size_t color_idx = 0; color_idx < pure_algo_futures_by_color.size(); ++color_idx) {
                auto& bucket = pure_algo_futures_by_color[color_idx];
                for (auto& future : bucket) {
                    PureColorAlgoResult result = future.get();
                    if (result.err) {
                        return FailByErr(result.err);
                    }
                    merge_pure_algo_result(color_idx, result);
                }
                bucket.clear();
            }
            return scheduler::StepResult::ok();
        };

        auto wait_mix_algo_futures = [&](std::array<std::vector<std::future<MixColorAlgoResult>>, kPureColorChannelCount>& futures_by_color)
            -> scheduler::StepResult {
            for (size_t color_idx = 0; color_idx < futures_by_color.size(); ++color_idx) {
                if (auto result = WaitFutureVectorAndMerge<MixColorAlgoResult>(
                        futures_by_color[color_idx],
                        [&](const MixColorAlgoResult& mix_result) {
                            AppendDefectOutput(
                                ctx, LightenDefectNotifyKind::MixedColor, color_idx, mix_result.mix_colors);
                        });
                    !result.isOk()) {
                    return result;
                }
            }
            return scheduler::StepResult::ok();
        };

        // 取出并等待算法检测池 B 中全部纯色/混色任务，合并结果到 ctx。
        auto drain_pure_algo_pool = [&]() -> scheduler::StepResult {
            if (auto result = wait_pure_algo_futures(); !result.isOk()) {
                return result;
            }
            std::array<std::vector<std::future<MixColorAlgoResult>>, kPureColorChannelCount> pending_mix{};
            {
                std::lock_guard<std::mutex> lock(algo_pool_mutex);
                for (size_t color_idx = 0; color_idx < pending_mix.size(); ++color_idx) {
                    pending_mix[color_idx].swap(mix_algo_futures_by_color[color_idx]);
                }
            }
            return wait_mix_algo_futures(pending_mix);
        };

        // 串灯延后池任务：等待当前颜色在算法检测池中的纯色任务，合并结果并确保 contour 已落盘。
        auto wait_pure_algo_for_color = [&](common::algorithm::PureDetectColor color) -> StringLightAlgoResult {
            StringLightAlgoResult out{};
            const size_t color_idx = ColorIndex(color);
            std::vector<std::future<PureColorAlgoResult>> pending;
            {
                std::lock_guard<std::mutex> lock(algo_pool_mutex);
                pending.swap(pure_algo_futures_by_color[color_idx]);
            }
            for (auto& future : pending) {
                PureColorAlgoResult result = future.get();
                if (result.err) {
                    out.err = result.err;
                    return out;
                }
                std::lock_guard<std::mutex> lock(ctx_merge_mutex);
                merge_pure_algo_result(color_idx, result);
            }
            return out;
        };

        auto notify_item_color = [&](LightenDetectProgressKind kind, bool enabled, int colorIdx,
                                     LightenDetectProgressStatus status) {
            if (enabled) {
                NotifyDetectProgress(kind, colorIdx, status);
            }
        };

        auto notify_color_cycle_items_for_color = [&](size_t color_idx, LightenDetectProgressStatus status) {
            const int idx = static_cast<int>(color_idx);
            notify_item_color(LightenDetectProgressKind::DeadLed, f.dead_light[color_idx], idx, status);
            notify_item_color(LightenDetectProgressKind::DimLed, f.dark_light[color_idx], idx, status);
            notify_item_color(LightenDetectProgressKind::BrightLed, f.bright_light[color_idx], idx, status);
            notify_item_color(
                LightenDetectProgressKind::ColorVariation, f.color_variation[color_idx], idx, status);
            notify_item_color(LightenDetectProgressKind::MixedColor, f.mix_color[color_idx], idx, status);
        };

        // 步骤 3：黑屏判定（同步，仅主相机）。
        if (f.black_screen) {
            NotifyDetectProgressSimple(LightenDetectProgressKind::BlackScreen,
                                     LightenDetectProgressStatus::InProgress);
            if (!main_white_image || !main_black_image) {
                return scheduler::StepResult::fail("LightenDetectStep: black screen reference images missing", -21);
            }
            int is_black_screen = 0;
            if (auto result = runBlackScreenDetectSync(
                    main_camera, main_white_image, main_black_image, is_black_screen);
                !result.isOk()) {
                return result;
            }
            ctx.set(KEY_IS_BLACK_SCREEN_OUT, is_black_screen, scheduler::VariableScope::FLOW);
            MessageCenter::Center::Notify(EVENT_BLACK_SCREEN_DETECT_RESULT, is_black_screen == 0);
            NotifyDetectProgressSimple(LightenDetectProgressKind::BlackScreen,
                                     LightenDetectProgressStatus::Completed);
        }

        // 步骤 4：黑屏亮灯检测 → 延后池；等待白场轮廓就绪后复用 white_green.contour（仅主相机）。
        if (f.black_light) {
            NotifyDetectProgressSimple(LightenDetectProgressKind::BlackScreenOn,
                                     LightenDetectProgressStatus::InProgress);
            const auto* algo_params = FindAlgoParams(main_camera);
            if (!algo_params || !main_black_image) {
                return scheduler::StepResult::fail("LightenDetectStep: black light reference image missing", -21);
            }
            const common::algorithm::LightenDetectAlgoParams camera_algo_params = *algo_params;
            black_light_futures.push_back(std::async(
                std::launch::async,
                [this,
                 camera_algo_params,
                 main_black_image,
                 white_contour_future]() {
                    if (auto err = white_contour_future.get()) {
                        BlackLightAlgoResult out{};
                        out.err = err;
                        return out;
                    }
                    auto black_light_params = prepareBlackLightParams(
                        camera_algo_params.black_light, main_black_image, camera_algo_params.cameraPath);
                    if (black_light_params.cam_exposure <= 0) {
                        black_light_params.cam_exposure = camera_algo_params.black_screen.black_cam_exposure;
                    }
                    return RunBlackLightAlgorithm(std::move(black_light_params));
                }));
        }

        // 步骤 5：纯色/混色检测，按 R → G → B 三色循环打屏采图。
        // 外层按颜色、内层按相机遍历；每台相机各自打屏并采图（参数取自 pure_color[color]）。
        // 采图必须在主线程同步完成：异步采图会导致打屏切换后才拍照，画面与曝光时序错位。
        // 算法计算耗时较长，采图完成后立即提交到 pure/mix_algo_futures，与后续打屏/采图并行。
        if (need_color_cycle) {
            for (const auto detect_color : kDetectColorSequence) {
                const size_t color_idx = ColorIndex(detect_color);
                const bool color_needs_pure = f.runPureAlgoForColor(color_idx);
                const bool color_needs_mix = f.mix_color[color_idx];
                if (!color_needs_pure && !color_needs_mix) {
                    continue;
                }

                notify_color_cycle_items_for_color(color_idx, LightenDetectProgressStatus::InProgress);

                for (const auto& entry : m_algo_params_by_camera) {
                    const std::string camera_key = entry.first;
                    const auto* algo_params = &entry.second;
                    if (!algo_params) {
                        return scheduler::StepResult::fail("LightenDetectStep: algo params not found for camera", -20);
                    }
                    const bool is_main_camera = camera_key == main_camera;
                    const common::algorithm::LightenDetectAlgoParams camera_algo_params = *algo_params;
                    const auto& pure_cfg = camera_algo_params.pure_color[color_idx];
                    const int capture_exposure = pure_cfg.cam_exposure;
                    const int capture_brightness = pure_cfg.disp_brightness;
                    // 混色检测仅主相机执行（依赖白场轮廓，副相机不参与）。
                    const bool run_mix_algo = color_needs_mix && is_main_camera;

                    // 打屏：settle_delay 等待面板稳定后再采图。
                    auto color_display = MakeBaseSoftDisplay(m_soft_screen_key, pure_cfg.led_w, pure_cfg.led_h, pure_cfg.disp_delay_ms);
                    color_display.color      = ToSoftScreenColor(detect_color);
                    color_display.brightness = capture_brightness;
                    if (auto err = SoftDisplay(dm, color_display)) {
                        return FailByErr(err);
                    }

                    // 主线程同步采图（不可放入 async，否则与下一帧打屏时序冲突）。
                    std::shared_ptr<common::algorithm::ImageBuffer> image;
                    if (auto err = GrabRgbBuffer(dm, camera_key, capture_exposure, &image)) {
                        return FailByErr(err);
                    }

                    // 纯色/串色与混色可能同时提交到异步池；shared_ptr 浅拷贝共享像素，
                    // 算法（Shot）只读 image->data，无需深拷贝整帧图像。
                    if (color_needs_pure) {
                        auto pure_params = preparePureColorParams(
                            camera_algo_params.pure_color[color_idx],
                            image,
                            is_main_camera,
                            camera_algo_params.cameraPath,
                            color_idx);
                        std::lock_guard<std::mutex> lock(algo_pool_mutex);
                        pure_algo_futures_by_color[color_idx].push_back(std::async(
                            std::launch::async,
                            [pure_params = std::move(pure_params)]() { return RunPureColorAlgorithm(pure_params); }));
                    }

                    // 混色算法：仅主相机；任务内阻塞等待 white_contour_future，与轮廓生成并行。
                    if (run_mix_algo) {
                        auto mix_params = prepareMixColorParams(
                            camera_algo_params.mix_color[color_idx],
                            image,
                            camera_algo_params.cameraPath);
                        if (mix_params.cam_exposure <= 0) {
                            mix_params.cam_exposure = pure_cfg.cam_exposure;
                        }
                        std::lock_guard<std::mutex> lock(algo_pool_mutex);
                        mix_algo_futures_by_color[color_idx].push_back(std::async(
                            std::launch::async,
                            [mix_params = std::move(mix_params),
                             white_contour_future]() mutable {
                                if (auto err = white_contour_future.get()) {
                                    MixColorAlgoResult out{};
                                    out.err = err;
                                    return out;
                                }
                                return RunMixColorAlgorithm(mix_params);
                            }));
                    }
                }

                if (color_needs_pure || color_needs_mix) {
                    notify_color_cycle_items_for_color(color_idx, LightenDetectProgressStatus::Completed);
                }
            }

            // 无串灯检测时，三色循环结束后直接排空算法检测池。
            if (!f.anyStringLight()) {
                if (auto result = drain_pure_algo_pool(); !result.isOk()) {
                    return result;
                }
            }
        }

        // 步骤 6：串灯检测，按 R/G/B 三色循环 × 相机遍历（与步骤 5 同色序）。
        // 奇/偶棋盘格分别采左右分屏图；纯色已开启时按色等待轮廓就绪后复用，否则用 ROI 点定位。
        if (f.anyStringLight()) {
            for (const auto detect_color : kDetectColorSequence) {
                const size_t color_idx = ColorIndex(detect_color);
                if (!f.string_light[color_idx]) {
                    continue;
                }

                NotifyDetectProgress(LightenDetectProgressKind::SeriesLed,
                                     static_cast<int>(color_idx),
                                     LightenDetectProgressStatus::InProgress);

                if (f.runPureAlgoForColor(color_idx)) {
                    if (auto wait_result = wait_pure_algo_for_color(detect_color); wait_result.err) {
                        return FailByErr(wait_result.err);
                    }
                }

                for (const auto& entry : m_algo_params_by_camera) {
                    const std::string camera_key = entry.first;
                    const auto* algo_params = &entry.second;
                    if (!algo_params) {
                        return scheduler::StepResult::fail("LightenDetectStep: algo params not found for camera", -20);
                    }
                    const auto& string_cfg = algo_params->string_light[color_idx];

                    // 奇数行棋盘格 → 左半屏；偶数行棋盘格 → 右半屏。
                    auto left_display = MakeBaseSoftDisplay(m_soft_screen_key, string_cfg.led_w, string_cfg.led_h, string_cfg.disp_delay_ms);
                    left_display.color      = ToSoftScreenColor(detect_color);
                    left_display.mode       = common::device::ODD_MODE;
                    left_display.brightness = string_cfg.disp_brightness;
                    if (auto err = SoftDisplay(dm, left_display)) {
                        return FailByErr(err);
                    }

                    std::shared_ptr<common::algorithm::ImageBuffer> left_image;
                    if (auto err = GrabRgbBuffer(dm, camera_key, string_cfg.cam_exposure, &left_image)) {
                        return FailByErr(err);
                    }

                    auto right_display = left_display;
                    right_display.mode = common::device::EVEN_MODE;
                    if (auto err = SoftDisplay(dm, right_display)) {
                        return FailByErr(err);
                    }

                    std::shared_ptr<common::algorithm::ImageBuffer> right_image;
                    if (auto err = GrabRgbBuffer(dm, camera_key, string_cfg.cam_exposure, &right_image)) {
                        return FailByErr(err);
                    }

                    string_light_futures_by_color[color_idx].push_back(std::async(
                        std::launch::async,
                        [this,
                         string_cfg,
                         cam_path = algo_params->cameraPath,
                         color_idx,
                         left_image = std::move(left_image),
                         right_image = std::move(right_image)]() {
                            return RunStringLightAlgorithm(prepareStringLightParams(
                                string_cfg,
                                std::move(left_image),
                                std::move(right_image),
                                cam_path,
                                color_idx));
                        }));
                }

                NotifyDetectProgress(LightenDetectProgressKind::SeriesLed,
                                     static_cast<int>(color_idx),
                                     LightenDetectProgressStatus::Completed);
            }
        }

        // 步骤 7/8：等待延后检测池（黑屏亮灯 + 串灯），合并结果到 ctx；任一失败则工步失败。
        // 注意：纯色/混色结果已在 drain_pure_algo_pool / wait_pure_algo_futures 中写入 ctx。
        int black_light_count = 0;
        if (auto result = WaitFutureVectorAndMerge<BlackLightAlgoResult>(
                black_light_futures,
                [&](const BlackLightAlgoResult& black_result) {
                    ctx.set(KEY_BLACK_SCREEN_LIGHT_OUT,
                            black_result.points,
                            scheduler::VariableScope::FLOW);
                    black_light_count = static_cast<int>(black_result.points.size());
                });
            !result.isOk()) {
            return result;
        }
        if (f.black_light) {
            NotifyDetectProgressSimple(LightenDetectProgressKind::BlackScreenOn,
                                     LightenDetectProgressStatus::Completed);
            MessageCenter::Center::Notify(EVENT_BLACK_SCREEN_LIGHT_DETECT_RESULT, black_light_count);
        }

        for (size_t color_idx = 0; color_idx < string_light_futures_by_color.size(); ++color_idx) {
            if (auto result = WaitFutureVectorAndMerge<StringLightAlgoResult>(
                    string_light_futures_by_color[color_idx],
                    [&](const StringLightAlgoResult& string_result) {
                        AppendDefectOutput(
                            ctx, LightenDefectNotifyKind::SeriesLed, color_idx, string_result.points);
                    });
                !result.isOk()) {
                return result;
            }
        }

        // 串灯阶段纯色结果已按色合并；此处排空算法检测池中剩余混色任务。
        if (f.anyStringLight()) {
            std::array<std::vector<std::future<MixColorAlgoResult>>, kPureColorChannelCount> pending_mix{};
            {
                std::lock_guard<std::mutex> lock(algo_pool_mutex);
                for (size_t color_idx = 0; color_idx < pending_mix.size(); ++color_idx) {
                    pending_mix[color_idx].swap(mix_algo_futures_by_color[color_idx]);
                }
            }
            if (auto result = wait_mix_algo_futures(pending_mix); !result.isOk()) {
                return result;
            }
        }

        return scheduler::StepResult::ok();
    } catch (const std::exception& e) {
        // 捕获 std::async 或算法内部未处理的异常，避免崩溃整个调度器。
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

// 注册步骤类型，供调度器通过 JSON typeName "lighten.light_detect" 创建实例。
SCHEDULER_REGISTER_STEP_CLASS(step::LightenDetectStep, "lighten.light_detect")
