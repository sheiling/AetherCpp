#include "result_summary_steps.h"

#include <common/plugins/lightendetector/service/vision_problem_detect.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <QDateTime>
#include <QString>
#include <QVariantList>

#include <common/utility/file_tool/file_manager.hpp>
#include <common/utility/json_tool/reflector.h>
#include <common/utility/string_tool/time_format.hpp>
#include <extensionsystem/pluginmanager.h>
#include <interface/ilighten_inspection_history_service.h>
#include <common/include/algorithm_data/mes_flow_data.h>
#include <messagecenter/message_center.h>

#include <scheduler/step/step_factory.h>

/**
 * @file result_summary_steps.cpp
 * @brief ResultSummarySteps 的实现。
 *
 * 缺陷计数来源：
 *   KEY_PURE_DEAD/DARK/BRIGHT_*_OUT、KEY_COLOR_VAR_*_OUT、KEY_STRING_LIGHT_*_OUT、
 *   KEY_MIX_COLOR_*_OUT（均为 R/G/B）、KEY_BLACK_SCREEN_LIGHT_OUT、KEY_IS_BLACK_SCREEN_OUT。
 * 执行流程：清除本工步输出键 → 读取 KEY_RESULT_SUMMARY_STEP_CONFIG → 坐标转换（panelCoord，ChangeLedPoint）→ 收集缺陷计数
 *   → 读取/补全 Flash 结果 → 综合判级 → 线性/区块不良特殊判定
 *   → 写入 overallDetectResult → 写入检测时间/耗时 → 追加检测历史记录
 *   → 缺陷点去重写入 KEY_DEDUP_DEFECTS_OUT → 第三方路径坐标汇总 CSV
 *   → Notify EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED。
 * 线性/区块不良汇总写入 KEY_LINE_DEFECT_SUMMARY_OUT / KEY_BLOCK_DEFECT_SUMMARY_OUT（不参与点级去重）。
 *
 * 第三方坐标汇总 CSV（writeThirdPartyCoordSummaryCsv）：
 *   - 根路径：KEY_RESULT_SUMMARY_STEP_CONFIG.thirdPartySoftwarePath（系统配置「第三方软件路径」）
 *   - 文件：{root}/point/{年}/{月}/{YYYYMMDD}.csv，同天按行追加
 *   - 数据来源：KEY_DEDUP_DEFECTS_OUT（点数/坐标/分色计数）、线性/区块不良汇总键
 *   - 分色组 6 类（无麻点）：死灯/暗灯/亮灯/串色/混色/串灯，各 R,G,B 三列
 *   - 路径为空、根目录不可访问或文件无法打开时静默跳过，不影响工步结果
 */

namespace step {
namespace {

using json = nlohmann::json;

using common::algorithm::BlockProblemDetectParam;
using common::algorithm::BlockProblemDetect;
using common::algorithm::ColumnProblemDetectParam;
using common::algorithm::ColumnProblemDetect;
using common::algorithm::DetectResultStatus;
using common::algorithm::JudgeDefectCountStatus;
using common::algorithm::kLightenDefectOutKeys;
using common::algorithm::kPureColorChannelCount;
using common::algorithm::LightenDetectDefectCounts;
using common::algorithm::LightenDefectNotifyKind;
using common::algorithm::MergeDetectResultStatus;

using common::algorithm::kLightenDefectNotifyKindCount;
using common::algorithm::BuildDefectTypeMask;
using common::algorithm::LightenDedupDefectKind;
using common::algorithm::LightenDedupDefectPoint;
using common::algorithm::LightenLineDefectSummary;
using common::algorithm::LightenBlockDefectSummary;

using common::algorithm::MixColorDefectPoint;
using common::algorithm::PureColorDefectPoint;
using common::algorithm::ColorVariationDefectPoint;
using common::algorithm::JoinPathAndFileName;
using common::algorithm::StringLightDefectPoint;

constexpr int kLightenDefectRgbCount = kLightenInspectionDefectRgbCount;

// 清空本工步 execute 写入 FlowContext 的输出键，避免 Flow 复用时残留上次汇总结果。
// 不含 KEY_FLASH_DETECT_OUT（上游 Flash 工步输入，仅缺失时补写默认值）。
void ClearResultSummaryOutputs(scheduler::FlowContext& ctx)
{
    ctx.remove(KEY_LINE_DEFECT_SUMMARY_OUT, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_BLOCK_DEFECT_SUMMARY_OUT, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_MES_FAIL_REASON, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_DEDUP_DEFECTS_OUT, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_OVERALL_DETECT_RESULT_OUT, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_INSPECT_DURATION_MS, scheduler::VariableScope::FLOW);
    ctx.remove(KEY_INSPECTED_AT, scheduler::VariableScope::FLOW);
}

// =============================================================================
// 第三方坐标汇总 CSV 辅助（匿名命名空间）
// 写入时机：execute 步骤 8 去重完成后；计数与坐标均来自 KEY_DEDUP_DEFECTS_OUT。
// =============================================================================

// 坐标摘要最多展示点数（超出时追加「共 N 个 仅显示前 M 个」提示，与 lighten_detect 一致）。
constexpr size_t kCoordAggMaxPoints = 500;

// 去重缺陷 RGB 分色计数（死灯/暗灯/亮灯/串色/混色/串灯 × R/G/B；黑屏亮灯仅计入 coordAgg，不计入分色列）。
struct DedupDefectRgbCounts {
    std::array<int, kPureColorChannelCount> dead{};             ///< 死灯 R/G/B
    std::array<int, kPureColorChannelCount> dim{};              ///< 暗灯 R/G/B
    std::array<int, kPureColorChannelCount> bright{};           ///< 亮灯 R/G/B
    std::array<int, kPureColorChannelCount> color_variation{};  ///< 串色 R/G/B
    std::array<int, kPureColorChannelCount> mix_color{};        ///< 混色 R/G/B
    std::array<int, kPureColorChannelCount> series{};           ///< 串灯 R/G/B
};

// R/G/B 通道索引 → 坐标 CSV 颜色字段。
const char* ColorChannelLabel(int rgb)
{
    switch (rgb) {
    case 0:
        return "R";
    case 1:
        return "G";
    case 2:
        return "B";
    default:
        return "R";
    }
}

constexpr size_t kCoordCsvDefectGroupCount = 6; ///< CSV 表头分色组数（无麻点列）

// LightenDedupDefectKind → 坐标 CSV 缺陷类型字段（与表头分色组名称一致；ColorVariation 输出「串色」）。
const char* DedupDefectKindCsvLabel(LightenDedupDefectKind kind)
{
    switch (kind) {
    case LightenDedupDefectKind::BlackScreenLight:
        return "黑屏亮灯";
    case LightenDedupDefectKind::DeadLed:
        return "死灯";
    case LightenDedupDefectKind::DimLed:
        return "暗灯";
    case LightenDedupDefectKind::BrightLed:
        return "亮灯";
    case LightenDedupDefectKind::ColorVariation:
        return "串色";
    case LightenDedupDefectKind::MixedColor:
        return "混色";
    case LightenDedupDefectKind::SeriesLed:
        return "串灯";
    default:
        return "缺陷";
    }
}

// 统计 KEY_DEDUP_DEFECTS_OUT 中各缺陷类型 R/G/B 数量（供 CSV 分色列）。
DedupDefectRgbCounts CountDedupDefectsByKindRgb(
    const std::vector<LightenDedupDefectPoint>& deduped)
{
    DedupDefectRgbCounts counts;
    for (const LightenDedupDefectPoint& pt : deduped) {
        const size_t color_idx = static_cast<size_t>(std::clamp(pt.rgb, 0, 2));
        switch (pt.kind) {
        case LightenDedupDefectKind::DeadLed:
            ++counts.dead[color_idx];
            break;
        case LightenDedupDefectKind::DimLed:
            ++counts.dim[color_idx];
            break;
        case LightenDedupDefectKind::BrightLed:
            ++counts.bright[color_idx];
            break;
        case LightenDedupDefectKind::ColorVariation:
            ++counts.color_variation[color_idx];
            break;
        case LightenDedupDefectKind::MixedColor:
            ++counts.mix_color[color_idx];
            break;
        case LightenDedupDefectKind::SeriesLed:
            ++counts.series[color_idx];
            break;
        case LightenDedupDefectKind::BlackScreenLight:
            // 黑屏亮灯参与 coordAgg，不参与下方 6 组分色计数列。
            break;
        }
    }
    return counts;
}

// 拼接不良点坐标摘要：x-y-颜色-类型，最多 kCoordAggMaxPoints 条。
std::string BuildCoordAggFromDedupDefects(const std::vector<LightenDedupDefectPoint>& deduped)
{
    std::string coord_agg;
    size_t shown_points = 0;
    for (const LightenDedupDefectPoint& pt : deduped) {
        if (shown_points >= kCoordAggMaxPoints) {
            break;
        }
        std::ostringstream piece;
        piece << pt.x << '-' << pt.y << '-' << ColorChannelLabel(pt.rgb) << '-'
              << DedupDefectKindCsvLabel(pt.kind);
        if (!coord_agg.empty()) {
            coord_agg += " | ";
        }
        coord_agg += piece.str();
        ++shown_points;
    }
    if (shown_points < deduped.size()) {
        if (!coord_agg.empty()) {
            coord_agg += " | ";
        }
        coord_agg += "...(共" + std::to_string(deduped.size()) + "个 仅显示前"
            + std::to_string(shown_points) + "个)";
    }
    return coord_agg;
}

// 线性/区块不良等特殊项摘要（写入 CSV「特殊检测项」列；来源 KEY_*_DEFECT_SUMMARY_OUT）。
std::string BuildSpecialDetectAgg(const scheduler::FlowContext& ctx)
{
    std::string special_agg;
    const auto append_piece = [&special_agg](const char* piece) {
        if (piece == nullptr || piece[0] == '\0') {
            return;
        }
        if (!special_agg.empty()) {
            special_agg += " | ";
        }
        special_agg += piece;
    };

    const auto line_summary =
        ctx.tryGet<LightenLineDefectSummary>(KEY_LINE_DEFECT_SUMMARY_OUT)
            .value_or(LightenLineDefectSummary{});
    if (!line_summary.empty()) {
        append_piece("线性不良");
    }

    const auto block_summary =
        ctx.tryGet<LightenBlockDefectSummary>(KEY_BLOCK_DEFECT_SUMMARY_OUT)
            .value_or(LightenBlockDefectSummary{});
    if (!block_summary.empty()) {
        append_piece("块状不良");
    }

    return special_agg;
}

// 读取配方产品名（MES 电气参数工步配置 productName）。
std::string ProductNameFromContext(const scheduler::FlowContext& ctx)
{
    const auto para_opt = ctx.tryGet<json>(KEY_MES_ELECTRICAL_PARA_STEP_CONFIG);
    if (!para_opt || !para_opt->is_object()) {
        return {};
    }
    std::string product_name;
    fromJson(*para_opt, "productName", product_name);
    return product_name;
}

// 拼接第三方坐标 CSV 路径：{root}/point/年/月/YYYYMMDD.csv；根目录不存在时返回空串。
std::string BuildThirdPartyCoordCsvPath(const std::string& third_party_root)
{
    if (third_party_root.empty()) {
        return {};
    }

    const std::string normalized_root =
        common::utility::FileManager::normalizeWindowsPath(third_party_root);
    if (!common::utility::FileManager::folderExist(normalized_root)) {
        return {};
    }

    const std::string day_str = common::utility::TimeFormat::GetCurrentDayStr();
    if (day_str.size() < 8) {
        return {};
    }
    const std::string year_tag  = day_str.substr(0, 4);
    const std::string month_tag = day_str.substr(4, 2);

    std::string point_month_path = JoinPathAndFileName(normalized_root, "point");
    point_month_path             = JoinPathAndFileName(point_month_path, year_tag);
    point_month_path             = JoinPathAndFileName(point_month_path, month_tag);
    common::utility::FileManager::createDir(point_month_path);

    return JoinPathAndFileName(point_month_path, day_str + ".csv");
}

// =============================================================================
// 判级 / 去重 / 历史入库辅助
// =============================================================================

constexpr std::array<LightenDefectNotifyKind, kLightenDefectNotifyKindCount> kAllDefectKinds = {
    // 线性/区块不良检测遍历顺序（不含黑屏/黑屏亮灯/Flash）。
    LightenDefectNotifyKind::DeadLed,
    LightenDefectNotifyKind::DimLed,
    LightenDefectNotifyKind::BrightLed,
    LightenDefectNotifyKind::ColorVariation,
    LightenDefectNotifyKind::SeriesLed,
    LightenDefectNotifyKind::MixedColor,
};

// 将算法侧 DetectResultStatus 映射为历史入库用的 LightenInspectionResult（Ok/Ng/Rw）。
LightenInspectionResult ToInspectionResult(DetectResultStatus status)
{
    switch (status) {
    case DetectResultStatus::Ng:
        return LightenInspectionResult::Ng;
    case DetectResultStatus::Rw:
        return LightenInspectionResult::Rw;
    case DetectResultStatus::Ok:
    default:
        return LightenInspectionResult::Ok;
    }
}

// 与系统配置默认阈值一致（system_config_model.cpp）；JSON 未配置 defectCountLimits 时使用。
common::algorithm::DefectCountLimits DefaultDefectCountLimits()
{
    common::algorithm::DefectCountLimits limits{};
    limits.dead_count.lower = 0;
    limits.dead_count.upper = 3;
    limits.dim_count.lower = 0;
    limits.dim_count.upper = 3;
    limits.bright_count.lower = 0;
    limits.bright_count.upper = 2;
    limits.color_variation_count.lower = 0;
    limits.color_variation_count.upper = 2;
    limits.string_light_count.lower = 0;
    limits.string_light_count.upper = 1;
    limits.mix_color_count.lower = 0;
    limits.mix_color_count.upper = 1;
    limits.total_defect_count.lower = 0;
    limits.total_defect_count.upper = 5;
    return limits;
}

// 读取 FlowContext 中 vector<T> 的长度，键不存在时返回 0。
template<typename T>
int VectorSizeOrZero(const scheduler::FlowContext& ctx, const char* key)
{
    return static_cast<int>(ctx.tryGet<std::vector<T>>(key).value_or(std::vector<T>{}).size());
}

// 累加指定缺陷类型 R/G/B 三色输出键对应的 vector 长度。
template<typename T>
int VectorSizeSumByDefectKind(const scheduler::FlowContext& ctx, LightenDefectNotifyKind kind)
{
    int total = 0;
    const size_t kind_idx = static_cast<size_t>(kind);
    for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
        total += VectorSizeOrZero<T>(ctx, kLightenDefectOutKeys[kind_idx][color_idx]);
    }
    return total;
}

// 将 R/G/B 三色缺陷点汇总为 (x,y) 列表，供线性/区块不良检测使用。
template<typename T>
void AppendMergedDefectPoints(
    const scheduler::FlowContext& ctx,
    LightenDefectNotifyKind kind,
    std::vector<common::geometry::Point>& out)
{
    const size_t kind_idx = static_cast<size_t>(kind);
    for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
        const auto points = ctx.tryGet<std::vector<T>>(kLightenDefectOutKeys[kind_idx][color_idx])
            .value_or(std::vector<T>{});
        out.reserve(out.size() + points.size());
        for (const auto& pt : points) {
            out.push_back({pt.x, pt.y});
        }
    }
}

// 按缺陷类型将 R/G/B 三色输出合并为 (x,y) 点集；类型不匹配时返回空 vector。
std::vector<common::geometry::Point> CollectMergedDefectPoints(
    const scheduler::FlowContext& ctx,
    LightenDefectNotifyKind kind)
{
    std::vector<common::geometry::Point> merged;
    switch (kind) {
    case LightenDefectNotifyKind::DeadLed:
    case LightenDefectNotifyKind::DimLed:
    case LightenDefectNotifyKind::BrightLed:
        AppendMergedDefectPoints<PureColorDefectPoint>(ctx, kind, merged);
        break;
    case LightenDefectNotifyKind::ColorVariation:
        AppendMergedDefectPoints<ColorVariationDefectPoint>(ctx, kind, merged);
        break;
    case LightenDefectNotifyKind::SeriesLed:
        AppendMergedDefectPoints<StringLightDefectPoint>(ctx, kind, merged);
        break;
    case LightenDefectNotifyKind::MixedColor:
        AppendMergedDefectPoints<MixColorDefectPoint>(ctx, kind, merged);
        break;
    }
    return merged;
}

// 将算法输出的 (x,y) 转换至配方坐标系（移植自华星 lighten_detect::changePoint）。
// coord_system_index 0–7 与配方「点亮产品基本信息 → 坐标系」下拉选项一一对应：
//   0 左上-宽X高Y  1 右上-宽Y高X  2 左下-宽X高Y  3 右下-宽Y高X
//   4 左上-宽Y高X  5 右上-宽X高Y  6 左下-宽Y高X  7 右下-宽X高Y
void ChangeLedPoint(int& x, int& y, int width, int height, int coord_system_index)
{
    if (width <= 0 || height <= 0) {
        return;
    }
    switch (std::clamp(coord_system_index, 0, 7)) {
    case 0: // 左上角-宽X高Y
        ++x;
        ++y;
        break;
    case 1: // 右上角-宽Y高X
        std::swap(x, y);
        y = width - y;
        ++x;
        std::swap(x, y);
        break;
    case 2: // 左下角-宽X高Y
        std::swap(x, y);
        x = height - x;
        ++y;
        break;
    case 3: // 右下角-宽Y高X
        x = width - x;
        y = height - y;
        std::swap(x, y);
        break;
    case 4: // 左上角-宽Y高X
        ++x;
        ++y;
        std::swap(x, y);
        break;
    case 5: // 右上角-宽X高Y
        std::swap(x, y);
        y = width - y;
        ++x;
        break;
    case 6: // 左下角-宽Y高X
        std::swap(x, y);
        x = height - x;
        ++y;
        std::swap(x, y);
        break;
    case 7: // 右下角-宽X高Y
        x = width - x;
        y = height - y;
        break;
    default:
        break;
    }
}

// 读取 vector<T>，逐点转换 (x,y) 后写回 FlowContext。
template<typename T>
void TransformAndSetDefectPoints(
    scheduler::FlowContext& ctx,
    const char* key,
    int width,
    int height,
    int coord_system_index)
{
    auto points_opt = ctx.tryGet<std::vector<T>>(key);
    if (!points_opt || points_opt->empty()) {
        return;
    }
    std::vector<T> points = std::move(*points_opt);
    for (auto& pt : points) {
        ChangeLedPoint(pt.x, pt.y, width, height, coord_system_index);
    }
    ctx.set(key, std::move(points), scheduler::VariableScope::FLOW);
}

// 线性不良（列/行连续缺陷）检测，检出段写入 out_problems。
void CollectColumnProblems(
    const std::vector<common::geometry::Point>& points,
    int led_w,
    int led_h,
    int threshold,
    LightenLineDefectSummary& out_problems)
{
    if (points.empty()) {
        return;
    }
    ColumnProblemDetectParam param;
    param.led_w              = led_w;
    param.led_h              = led_h;
    param.width_detect_flag  = threshold;
    param.height_detect_flag = threshold;
    param.points             = points;
    ColumnProblemDetect(param);
    if (!param.column_problems.empty()) {
        out_problems.insert(out_problems.end(),
                            param.column_problems.begin(),
                            param.column_problems.end());
    }
}

// 区块不良检测，检出区块写入 out_problems。
void CollectBlockProblems(
    const std::vector<common::geometry::Point>& points,
    int led_w,
    int led_h,
    int threshold,
    LightenBlockDefectSummary& out_problems)
{
    if (points.empty()) {
        return;
    }
    BlockProblemDetectParam param;
    param.led_w       = led_w;
    param.led_h       = led_h;
    param.detect_flag = threshold;
    param.points      = points;
    BlockProblemDetect(param);
    if (!param.block_problems.empty()) {
        out_problems.insert(out_problems.end(),
                            param.block_problems.begin(),
                            param.block_problems.end());
    }
}

// (x,y,rgb) 去重键：高位 x、中位 y、低位 rgb。
uint64_t MakeDedupKey(int x, int y, int rgb)
{
    return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32)
         | (static_cast<uint64_t>(static_cast<uint16_t>(y)) << 16)
         | static_cast<uint64_t>(static_cast<uint16_t>(rgb));
}

// LightenDefectNotifyKind → 去重缺陷类型（6 类分色缺陷）。
LightenDedupDefectKind NotifyKindToDedupKind(LightenDefectNotifyKind kind)
{
    switch (kind) {
    case LightenDefectNotifyKind::DeadLed:
        return LightenDedupDefectKind::DeadLed;
    case LightenDefectNotifyKind::DimLed:
        return LightenDedupDefectKind::DimLed;
    case LightenDefectNotifyKind::BrightLed:
        return LightenDedupDefectKind::BrightLed;
    case LightenDefectNotifyKind::ColorVariation:
        return LightenDedupDefectKind::ColorVariation;
    case LightenDefectNotifyKind::MixedColor:
        return LightenDedupDefectKind::MixedColor;
    case LightenDefectNotifyKind::SeriesLed:
        return LightenDedupDefectKind::SeriesLed;
    default:
        return LightenDedupDefectKind::DeadLed;
    }
}

// 将指定缺陷类型的 R/G/B 输出点插入去重 map；同 (x,y,rgb) 仅保留先写入的类型（调用顺序即优先级）。
template<typename T>
void InsertDefectPointsForDedup(
    const scheduler::FlowContext& ctx,
    LightenDefectNotifyKind notify_kind,
    std::unordered_map<uint64_t, LightenDedupDefectPoint>& dedup_map)
{
    const LightenDedupDefectKind dedup_kind = NotifyKindToDedupKind(notify_kind);
    const size_t kind_idx                   = static_cast<size_t>(notify_kind);
    for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
        const auto points = ctx.tryGet<std::vector<T>>(kLightenDefectOutKeys[kind_idx][color_idx])
                                .value_or(std::vector<T>{});
        for (const T& pt : points) {
            const uint64_t key = MakeDedupKey(pt.x, pt.y, static_cast<int>(color_idx));
            if (dedup_map.find(key) == dedup_map.end()) {
                dedup_map.emplace(key,
                                  LightenDedupDefectPoint{pt.x, pt.y, static_cast<int>(color_idx), dedup_kind});
            }
        }
    }
}

// 按优先级（黑屏亮灯→死灯→…→串灯）对 (x,y,rgb) 去重，写入 KEY_DEDUP_DEFECTS_OUT。
void WriteDeduplicatedDefects(scheduler::FlowContext& ctx)
{
    std::unordered_map<uint64_t, LightenDedupDefectPoint> dedup_map;
    dedup_map.reserve(128);

    // 1. 黑屏亮灯（无分色通道，与 MES 一致记为 G 通道）
    const auto black_light_points =
        ctx.tryGet<std::vector<common::geometry::Point>>(KEY_BLACK_SCREEN_LIGHT_OUT)
            .value_or(std::vector<common::geometry::Point>{});
    for (const common::geometry::Point& pt : black_light_points) {
        const uint64_t key = MakeDedupKey(pt.x, pt.y, 1);
        if (dedup_map.find(key) == dedup_map.end()) {
            dedup_map.emplace(key,
                              LightenDedupDefectPoint{
                                  pt.x, pt.y, 1, LightenDedupDefectKind::BlackScreenLight});
        }
    }

    // 2–7. 六类分色缺陷（顺序即优先级）
    InsertDefectPointsForDedup<PureColorDefectPoint>(ctx, LightenDefectNotifyKind::DeadLed, dedup_map);
    InsertDefectPointsForDedup<PureColorDefectPoint>(ctx, LightenDefectNotifyKind::DimLed, dedup_map);
    InsertDefectPointsForDedup<PureColorDefectPoint>(ctx, LightenDefectNotifyKind::BrightLed, dedup_map);
    InsertDefectPointsForDedup<ColorVariationDefectPoint>(ctx, LightenDefectNotifyKind::ColorVariation, dedup_map);
    InsertDefectPointsForDedup<MixColorDefectPoint>(ctx, LightenDefectNotifyKind::MixedColor, dedup_map);
    InsertDefectPointsForDedup<StringLightDefectPoint>(ctx, LightenDefectNotifyKind::SeriesLed, dedup_map);

    std::vector<LightenDedupDefectPoint> deduped;
    deduped.reserve(dedup_map.size());
    for (const auto& entry : dedup_map) {
        deduped.push_back(entry.second);
    }
    ctx.set(KEY_DEDUP_DEFECTS_OUT, std::move(deduped), scheduler::VariableScope::FLOW);
}

// 收集 6 类缺陷 × R/G/B 共 18 项分色计数（顺序与 lighten_inspection_history / defectCounts 一致）。
// flat_idx 编码：kind_idx × 3 + color_idx（kind 顺序见 LightenDefectNotifyKind，color 顺序 R/G/B）。
std::array<int, kLightenDefectRgbCount> CollectDefectRgbCounts(const scheduler::FlowContext& ctx)
{
    std::array<int, kLightenDefectRgbCount> rgb_counts{};
    for (size_t kind_idx = 0; kind_idx < kLightenDefectNotifyKindCount; ++kind_idx) {
        for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
            const int flat_idx =
                static_cast<int>(kind_idx * kPureColorChannelCount + color_idx);
            switch (static_cast<LightenDefectNotifyKind>(kind_idx)) {
            case LightenDefectNotifyKind::DeadLed:
            case LightenDefectNotifyKind::DimLed:
            case LightenDefectNotifyKind::BrightLed:
                rgb_counts[flat_idx] = VectorSizeOrZero<common::algorithm::PureColorDefectPoint>(
                    ctx, kLightenDefectOutKeys[kind_idx][color_idx]);
                break;
            case LightenDefectNotifyKind::ColorVariation:
                rgb_counts[flat_idx] =
                    VectorSizeOrZero<common::algorithm::ColorVariationDefectPoint>(
                        ctx, kLightenDefectOutKeys[kind_idx][color_idx]);
                break;
            case LightenDefectNotifyKind::SeriesLed:
                rgb_counts[flat_idx] = VectorSizeOrZero<common::algorithm::StringLightDefectPoint>(
                    ctx, kLightenDefectOutKeys[kind_idx][color_idx]);
                break;
            case LightenDefectNotifyKind::MixedColor:
                rgb_counts[flat_idx] = VectorSizeOrZero<common::algorithm::MixColorDefectPoint>(
                    ctx, kLightenDefectOutKeys[kind_idx][color_idx]);
                break;
            }
        }
    }
    return rgb_counts;
}

// 生成当前本地时间字符串，格式 yyyy-MM-dd HH:mm:ss（写入 KEY_INSPECTED_AT / 历史 inspected_at）。
QString BuildInspectedAtNow()
{
    return QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
}

// 由 KEY_INSPECT_START_MS 与当前 steady_clock 差值计算单片检测耗时（毫秒）；键缺失时返回 0。
int ComputeInspectDurationMs(const scheduler::FlowContext& ctx)
{
    const int64_t end_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    const int64_t start_ms = ctx.tryGet<int64_t>(KEY_INSPECT_START_MS).value_or(end_ms);
    return static_cast<int>(std::max<int64_t>(0, end_ms - start_ms));
}

// 组装 lighten_inspection_history 单行入库记录。
// counts：6 类缺陷 RGB 汇总数量（判级与 defectTypeMask 来源）；
// defectCounts：18 项 R/G/B 分色计数（首页进度 / 历史明细，flat_idx = kind×3 + color）。
LightenInspectionHistoryRecord BuildHistoryRecord(
    const scheduler::FlowContext& ctx,
    const LightenDetectDefectCounts& counts,
    DetectResultStatus overall,
    DetectResultStatus flash_result,
    const QString& inspected_at,
    int inspect_duration_ms)
{
    LightenInspectionHistoryRecord record;
    record.model = QString::fromStdString(
        ctx.tryGet<std::string>(KEY_PRODUCT_MODEL).value_or(std::string{})).trimmed();
    record.serialNumber = QString::fromStdString(
        ctx.tryGet<std::string>(KEY_BOARD_SN).value_or(std::string{})).trimmed();
    record.result            = ToInspectionResult(overall);           // 综合判级 Ok/Ng/Rw
    record.inspectTimes      = 0;                                     // 复检次数，首检固定 0
    record.rwCount           = 0;                                     // RW 累计，首检固定 0
    record.inspectDurationMs = inspect_duration_ms;                   // 单片耗时（毫秒）
    record.blackScreenLightCount = counts.black_screen_light_count;     // 黑屏亮灯缺陷点数
    // 18 项分色计数：死灯/暗灯/亮灯/串色/串灯/混色 × R/G/B，顺序同 kLightenDefectOutKeys
    record.defectCounts      = CollectDefectRgbCounts(ctx);
    record.defectTypeMask    = BuildDefectTypeMask(counts);           // 缺陷类型位掩码（按汇总数量）
    record.flashResult       = static_cast<int>(flash_result);        // Flash 结果整型码 0=OK/1=NG/2=RW
    record.specialDetect.clear();                                     // 线性/区块不良明细，当前未写入
    record.inspectedAt       = inspected_at;                            // 检测完成时间
    return record;
}

// 经 ILightenInspectionHistoryService 追加历史记录；服务未就绪返回 0，失败返回 -1。
qint64 AppendInspectionHistoryRecord(const LightenInspectionHistoryRecord& record, QString* error_out)
{
    auto* service = ExtensionSystem::PluginManager::getInterface<ILightenInspectionHistoryService>();
    if (!service || !service->isReady())
        return 0;
    return service->appendRecord(record, error_out);
}

} // namespace

// =============================================================================
// ResultSummarySteps 配置解析（reflectFromJson / loadParamsFromJson）
// reflectFromJson 保留接口；运行时参数在 execute 入口从 KEY_RESULT_SUMMARY_STEP_CONFIG 加载。
// =============================================================================

void ResultSummarySteps::reflectFromJson(const json& /*j*/)
{
}

// 从运行时 JSON 加载 defectCountLimits / specialDetectRules / panelCoord / thirdPartySoftwarePath。
bool ResultSummarySteps::loadParamsFromJson(const json& root)
{
    if (!root.is_object()) {
        return false;
    }

    m_limits = DefaultDefectCountLimits();
    if (root.contains("defectCountLimits")) {
        fromJson(root, "defectCountLimits", m_limits);
    }

    m_special_detect_rules = {};
    if (root.contains("specialDetectRules")) {
        fromJson(root, "specialDetectRules", m_special_detect_rules);
    }

    m_panel_coord = {};
    if (root.contains("panelCoord")) {
        fromJson(root, "panelCoord", m_panel_coord);
    }

    m_third_party_path.clear();
    if (root.contains("thirdPartySoftwarePath")) {
        // 系统配置「第三方软件路径」，由 HomeView 写入；空则跳过 CSV 导出。
        fromJson(root, "thirdPartySoftwarePath", m_third_party_path);
    }
    return true;
}

// 将 FlowContext 中 6 类 × R/G/B 缺陷点坐标转换至配方坐标系（panelCoord）并写回。
void ResultSummarySteps::transformAllDefectCoordinates(scheduler::FlowContext& ctx) const
{
    // panelCoord 无效（宽高 ≤ 0）时跳过，保留算法原始坐标。
    const int width  = m_panel_coord.width;
    const int height = m_panel_coord.height;
    const int coord  = m_panel_coord.coord_system_index;
    if (width <= 0 || height <= 0) {
        return;
    }

    for (size_t kind_idx = 0; kind_idx < kLightenDefectNotifyKindCount; ++kind_idx) {
        for (size_t color_idx = 0; color_idx < kPureColorChannelCount; ++color_idx) {
            const char* key = kLightenDefectOutKeys[kind_idx][color_idx];
            switch (static_cast<LightenDefectNotifyKind>(kind_idx)) {
            case LightenDefectNotifyKind::DeadLed:
            case LightenDefectNotifyKind::DimLed:
            case LightenDefectNotifyKind::BrightLed:
                TransformAndSetDefectPoints<PureColorDefectPoint>(ctx, key, width, height, coord);
                break;
            case LightenDefectNotifyKind::ColorVariation:
                TransformAndSetDefectPoints<ColorVariationDefectPoint>(ctx, key, width, height, coord);
                break;
            case LightenDefectNotifyKind::SeriesLed:
                TransformAndSetDefectPoints<StringLightDefectPoint>(ctx, key, width, height, coord);
                break;
            case LightenDefectNotifyKind::MixedColor:
                TransformAndSetDefectPoints<MixColorDefectPoint>(ctx, key, width, height, coord);
                break;
            }
        }
    }
}

// 从 FlowContext 读取各缺陷点向量长度；total_defect_count 仅累加 6 类分色缺陷。
LightenDetectDefectCounts ResultSummarySteps::collectDefectCounts(
    scheduler::FlowContext& ctx) const
{
    LightenDetectDefectCounts counts{};

    counts.dead_count = VectorSizeSumByDefectKind<common::algorithm::PureColorDefectPoint>(
        ctx, LightenDefectNotifyKind::DeadLed);
    counts.dim_count = VectorSizeSumByDefectKind<common::algorithm::PureColorDefectPoint>(
        ctx, LightenDefectNotifyKind::DimLed);
    counts.bright_count = VectorSizeSumByDefectKind<common::algorithm::PureColorDefectPoint>(
        ctx, LightenDefectNotifyKind::BrightLed);
    counts.color_variation_count = VectorSizeSumByDefectKind<common::algorithm::ColorVariationDefectPoint>(
        ctx, LightenDefectNotifyKind::ColorVariation);
    counts.string_light_count = VectorSizeSumByDefectKind<common::algorithm::StringLightDefectPoint>(
        ctx, LightenDefectNotifyKind::SeriesLed);
    counts.mix_color_count = VectorSizeSumByDefectKind<common::algorithm::MixColorDefectPoint>(
        ctx, LightenDefectNotifyKind::MixedColor);
    counts.black_screen_light_count = VectorSizeOrZero<common::geometry::Point>(
        ctx, KEY_BLACK_SCREEN_LIGHT_OUT);
    counts.is_black_screen = ctx.tryGet<int>(KEY_IS_BLACK_SCREEN_OUT).value_or(0);

    counts.total_defect_count = counts.dead_count + counts.dim_count + counts.bright_count
        + counts.color_variation_count + counts.string_light_count + counts.mix_color_count;

    return counts;
}

// 按 m_limits 对各类缺陷数量逐项判级并合并；黑屏亮灯 / 黑屏 / Flash NG 时强制 NG。
DetectResultStatus ResultSummarySteps::judgeOverallStatus(
    const LightenDetectDefectCounts& counts,
    DetectResultStatus flash_result) const
{
    DetectResultStatus overall = DetectResultStatus::Ok;

    overall = MergeDetectResultStatus(
        overall, JudgeDefectCountStatus(counts.dead_count, m_limits.dead_count));
    overall = MergeDetectResultStatus(
        overall, JudgeDefectCountStatus(counts.dim_count, m_limits.dim_count));
    overall = MergeDetectResultStatus(
        overall, JudgeDefectCountStatus(counts.bright_count, m_limits.bright_count));
    overall = MergeDetectResultStatus(
        overall,
        JudgeDefectCountStatus(counts.color_variation_count, m_limits.color_variation_count));
    overall = MergeDetectResultStatus(
        overall, JudgeDefectCountStatus(counts.string_light_count, m_limits.string_light_count));
    overall = MergeDetectResultStatus(
        overall, JudgeDefectCountStatus(counts.mix_color_count, m_limits.mix_color_count));
    overall = MergeDetectResultStatus(
        overall,
        JudgeDefectCountStatus(counts.total_defect_count, m_limits.total_defect_count));

    if (counts.black_screen_light_count > 0) {
        overall = MergeDetectResultStatus(overall, DetectResultStatus::Ng);
    }

    if (counts.is_black_screen != 0) {
        overall = MergeDetectResultStatus(overall, DetectResultStatus::Ng);
    }

    overall = MergeDetectResultStatus(overall, flash_result);

    return overall;
}

// 线性不良：R/G/B 汇总后按 defect 类型依次检测；区块不良：同理。任一检出则 NG。
// 检出的线性/区块不良段汇总写入 FlowContext（不参与点级去重）。
DetectResultStatus ResultSummarySteps::judgeLineAndBlockDefects(
    scheduler::FlowContext& ctx) const
{
    const bool need_line  = m_special_detect_rules.line_defect.enabled;
    const bool need_block = m_special_detect_rules.block_defect.enabled;
    if (!need_line && !need_block) {
        return DetectResultStatus::Ok;
    }

    const int led_w = m_panel_coord.width;
    const int led_h = m_panel_coord.height;
    if (led_w <= 0 || led_h <= 0) {
        return DetectResultStatus::Ok;
    }

    const int line_threshold  = m_special_detect_rules.line_defect.lamp_count_threshold;
    const int block_threshold = m_special_detect_rules.block_defect.lamp_count_threshold;

    LightenLineDefectSummary line_summary;
    LightenBlockDefectSummary block_summary;
    bool has_line  = false;
    bool has_block = false;

    for (const auto kind : kAllDefectKinds) {
        const auto points = CollectMergedDefectPoints(ctx, kind);
        if (points.empty()) {
            continue;
        }
        if (need_line) {
            const size_t before = line_summary.size();
            CollectColumnProblems(points, led_w, led_h, line_threshold, line_summary);
            if (line_summary.size() > before) {
                has_line = true;
            }
        }
        if (need_block) {
            const size_t before = block_summary.size();
            CollectBlockProblems(points, led_w, led_h, block_threshold, block_summary);
            if (block_summary.size() > before) {
                has_block = true;
            }
        }
    }

    if (has_line) {
        ctx.set(KEY_LINE_DEFECT_SUMMARY_OUT, std::move(line_summary), scheduler::VariableScope::FLOW);
        ctx.set(KEY_MES_FAIL_REASON, std::string("线性不良"), scheduler::VariableScope::FLOW);
    }
    if (has_block) {
        ctx.set(KEY_BLOCK_DEFECT_SUMMARY_OUT, std::move(block_summary), scheduler::VariableScope::FLOW);
        if (!has_line) {
            ctx.set(KEY_MES_FAIL_REASON, std::string("区块不良"), scheduler::VariableScope::FLOW);
        }
    }

    if (has_line || has_block) {
        return DetectResultStatus::Ng;
    }
    return DetectResultStatus::Ok;
}

// =============================================================================
// 第三方坐标汇总 CSV（按天追加；须在 WriteDeduplicatedDefects 之后调用）
// =============================================================================

// 将单片检测结果追加写入第三方路径坐标汇总 CSV。
// 表头两行：基础列 + 6 组分色组名 + 特殊检测项/检测时间；第二行为各组 R,G,B 子列。
// 数据行：条码, 产品名, 不良点数, 不良点坐标, 检测结果(OK/NG),
//   死/暗/亮/串色/混/串灯各 R,G,B, 特殊检测项, 检测时间（yyyy/MM/dd HH:mm:ss）。
// RW 与 NG 均记为 NG；路径或 IO 失败时不抛错、不使工步失败。
void ResultSummarySteps::writeThirdPartyCoordSummaryCsv(
    const scheduler::FlowContext& ctx,
    DetectResultStatus overall) const
{
    if (m_third_party_path.empty()) {
        return;
    }

    const std::string coord_csv_path = BuildThirdPartyCoordCsvPath(m_third_party_path);
    if (coord_csv_path.empty()) {
        return;
    }

    const auto deduped =
        ctx.tryGet<std::vector<LightenDedupDefectPoint>>(KEY_DEDUP_DEFECTS_OUT)
            .value_or(std::vector<LightenDedupDefectPoint>{});
    const DedupDefectRgbCounts rgb_counts = CountDedupDefectsByKindRgb(deduped);
    const std::string coord_agg           = BuildCoordAggFromDedupDefects(deduped);
    const std::string special_agg         = BuildSpecialDetectAgg(ctx);

    // 条码缺省与 lighten_detect 一致；产品名优先 MES 电气参数，否则回退 KEY_PRODUCT_MODEL。
    std::string str_sn = ctx.tryGet<std::string>(KEY_BOARD_SN).value_or(std::string{});
    if (str_sn.empty()) {
        str_sn = "detectResult";
    }

    std::string product_name = ProductNameFromContext(ctx);
    if (product_name.empty()) {
        product_name = ctx.tryGet<std::string>(KEY_PRODUCT_MODEL).value_or(std::string{});
    }

    const std::string detect_result_str =
        (overall == DetectResultStatus::Ok) ? "OK" : "NG";
    const std::string current_time =
        common::utility::TimeFormat::getCurrentTime("%Y/%m/%d %H:%M:%S");

    // 文件不存在或大小为 0 时写入两行表头（与 lighten_detect 汇总 CSV 格式一致）。
    bool need_header = true;
    if (common::utility::FileManager::fileExist(coord_csv_path.c_str())) {
        std::ifstream size_check(coord_csv_path, std::ios::binary | std::ios::ate);
        need_header = !size_check.is_open() || size_check.tellg() == 0;
    }

    std::ofstream coord_file(coord_csv_path, std::ios::out | std::ios::app);
    if (!coord_file.is_open()) {
        return;
    }

    if (need_header) {
        coord_file << "条码" << "," << "产品名" << "," << "不良点数" << "," << "不良点坐标" << ","
                   << "检测结果";
        static const char* k_defect_group_names[] = {
            "死灯", "暗灯", "亮灯", "串色", "混色", "串灯"};
        coord_file << "," << "," << k_defect_group_names[0];
        for (size_t d = 1; d < kCoordCsvDefectGroupCount; ++d) {
            coord_file << ",,," << k_defect_group_names[d];
        }
        coord_file << "," << "," << "特殊检测项" << "," << "检测时间" << "\n";

        coord_file << ",,,,,";
        for (size_t d = 0; d < kCoordCsvDefectGroupCount; ++d) {
            if (d != 0) {
                coord_file << ",";
            }
            coord_file << "R,G,B";
        }
        coord_file << "," << "," << "\n";
    }

    coord_file << str_sn << "," << product_name << "," << deduped.size() << "," << coord_agg << ","
               << detect_result_str << "," << rgb_counts.dead[0] << "," << rgb_counts.dead[1] << ","
               << rgb_counts.dead[2] << "," << rgb_counts.dim[0] << "," << rgb_counts.dim[1] << ","
               << rgb_counts.dim[2] << "," << rgb_counts.bright[0] << "," << rgb_counts.bright[1] << ","
               << rgb_counts.bright[2] << "," << rgb_counts.color_variation[0] << ","
               << rgb_counts.color_variation[1] << "," << rgb_counts.color_variation[2] << ","
               << rgb_counts.mix_color[0] << "," << rgb_counts.mix_color[1] << ","
               << rgb_counts.mix_color[2] << "," << rgb_counts.series[0] << "," << rgb_counts.series[1]
               << "," << rgb_counts.series[2] << "," << special_agg << "," << current_time << "\n";
}

// =============================================================================
// 主执行入口 execute
// 步骤 1–7：配置加载 → 坐标转换 → 计数/判级 → 历史入库/KPI 通知
// 步骤 8–9：缺陷去重（KEY_DEDUP_DEFECTS_OUT）→ 第三方坐标 CSV
// =============================================================================

scheduler::StepResult ResultSummarySteps::execute(scheduler::FlowContext& ctx)
{
    try {
        ClearResultSummaryOutputs(ctx);
        // 1. 加载运行时配置（defectCountLimits / specialDetectRules / panelCoord / thirdPartySoftwarePath）
        const auto params_opt = ctx.tryGet<json>(KEY_RESULT_SUMMARY_STEP_CONFIG);
        if (!params_opt || !loadParamsFromJson(*params_opt)) {
            return scheduler::StepResult::fail("ResultSummarySteps: failed to load algo params", -20);
        }

        // 2. 缺陷点坐标转换至配方坐标系
        transformAllDefectCoordinates(ctx);

        // 3. 统计各类缺陷 RGB 汇总数量
        const LightenDetectDefectCounts counts = collectDefectCounts(ctx);

        // 4. 读取/补全 Flash 结果
        const auto flash_opt = ctx.tryGet<int>(KEY_FLASH_DETECT_OUT);
        if (!flash_opt.has_value()) {
            // Flash 检测工步未启用或未执行时，默认视为 OK。
            ctx.set(KEY_FLASH_DETECT_OUT,
                      static_cast<int>(DetectResultStatus::Ok),
                      scheduler::VariableScope::FLOW);
        }
        const DetectResultStatus flash_result = static_cast<DetectResultStatus>(
            flash_opt.value_or(static_cast<int>(DetectResultStatus::Ok)));

        // 5. 综合判级（数量阈值 + 线性/区块不良）
        const DetectResultStatus overall = MergeDetectResultStatus(
            judgeOverallStatus(counts, flash_result),
            judgeLineAndBlockDefects(ctx));

        // 6. 写回 FlowContext 输出键
        ctx.set(KEY_OVERALL_DETECT_RESULT_OUT, static_cast<int>(overall), scheduler::VariableScope::FLOW);

        const int inspect_duration_ms = ComputeInspectDurationMs(ctx);
        const QString inspected_at    = BuildInspectedAtNow();
        ctx.set(KEY_INSPECT_DURATION_MS, inspect_duration_ms, scheduler::VariableScope::FLOW);
        ctx.set(KEY_INSPECTED_AT, inspected_at.toStdString(), scheduler::VariableScope::FLOW);

        // 7. 追加检测历史并通知首页 KPI 刷新
        // model 与 serialNumber 均非空时才入库并刷新首页 KPI（避免空 SN 脏数据）。
        const LightenInspectionHistoryRecord history_record =
            BuildHistoryRecord(ctx, counts, overall, flash_result, inspected_at, inspect_duration_ms);
        if (!history_record.model.isEmpty() && !history_record.serialNumber.isEmpty()) {
            QString append_error;
            if (AppendInspectionHistoryRecord(history_record, &append_error) < 0) {
                return scheduler::StepResult::fail(
                    append_error.isEmpty()
                        ? "ResultSummarySteps: failed to append inspection history"
                        : append_error.toStdString(),
                    -30);
            }
            MessageCenter::Center::Notify(EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED, QVariantList{});
        }

        // 8. 缺陷点去重（大面积灭点/线性/区块不良不参与）并写入 FlowContext
        WriteDeduplicatedDefects(ctx);

        // 9. 第三方路径坐标汇总 CSV（point/年/月/YYYYMMDD.csv）
        writeThirdPartyCoordSummaryCsv(ctx, overall);

        return scheduler::StepResult::ok();
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::ResultSummarySteps, "lighten.result_summary")
