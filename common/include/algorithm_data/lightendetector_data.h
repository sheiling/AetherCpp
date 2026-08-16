#pragma once

/**
 * @file lighten_detector_data.h
 * @brief 点亮检测（LightenDetector）各子算法的参数结构与 FlowContext 输出键定义。
 *
 * 参数分层说明（LightenDetectAlgoParams）：
 *   - white               白场轮廓生成参数，由 LightenDetectStep 统一采集
 *   - black_screen        黑屏判定算法参数（含黑/白场采图曝光/亮度/延迟；输入黑/白场参考图）
 *   - black_light         黑屏亮灯检测算法参数（输入黑场参考图 + 白场轮廓）
 *   - pure_color          R/G/B 纯色检测（内置串色），含采图与轮廓落盘
 *   - mix_color           R/G/B 混色检测（复用白场轮廓）
 *   - string_light        R/G/B 串灯检测（复用纯色轮廓或 ROI 点定位）
 *
 * FlowContext 运行时键：
 *   - KEY_BOARD_SN              板卡序列号（FLOW scope；motion.scan_code 或流程启动方写入）
 *   - KEY_SCAN_STEP_CONFIG           扫码工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }）
 *   - KEY_UPSTREAM_DOCK_STEP_CONFIG   上游对接工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }）
 *   - KEY_DOWNSTREAM_DOCK_STEP_CONFIG 下游对接工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }）
 *   - KEY_PRODUCT_MODEL         产品型号/默认配方 ID（GLOBAL scope；HomeView「初始化」按钮写入）
 *   - KEY_INSPECTED_AT          检测时间 yyyy-MM-dd HH:mm:ss（FLOW scope，result_summary 写入）
 *   - KEY_INSPECT_DURATION_MS   单片检测耗时毫秒（FLOW scope，result_summary 写入）
 *   - KEY_DEDUP_DEFECTS_OUT     去重后缺陷点（FLOW scope，result_summary 写入，MES 读取）
 *   - KEY_LINE_DEFECT_SUMMARY_OUT / KEY_BLOCK_DEFECT_SUMMARY_OUT  线性/区块不良汇总（FLOW scope）
 *   - EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED  生产 KPI 表更新（MessageCenter Notify；首页刷新 KPI 条）
 */

#include <common/base/algorithm/algorithm_packet.h>
#include <common/3rdparty/cisdk/measurement/include/ci_data.h>
#include <common/include/geometry.h>
#include <common/utility/json_tool/reflector.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace common {
namespace algorithm {

inline void fromJsonAlgoPoint(const json& j, const std::string& key, AlgoPoint& p)
{
    if (!j.contains(key) || !j[key].is_object()) {
        return;
    }
    const auto& o = j[key];
    fromJson(o, "x", p.x);
    fromJson(o, "y", p.y);
}

inline void fromJsonAlgoSize(const json& j, const std::string& key, AlgoSize& s)
{
    if (!j.contains(key) || !j[key].is_object()) {
        return;
    }
    const auto& o = j[key];
    fromJson(o, "width", s.width);
    fromJson(o, "height", s.height);
}

inline void toJsonAlgoPoint(json& j, const std::string& key, const AlgoPoint& p)
{
    json o = json::object();
    TO_JSON_KEY(o, "x", p.x);
    TO_JSON_KEY(o, "y", p.y);
    j[key] = std::move(o);
}

inline void toJsonAlgoSize(json& j, const std::string& key, const AlgoSize& s)
{
    json o = json::object();
    TO_JSON_KEY(o, "width", s.width);
    TO_JSON_KEY(o, "height", s.height);
    j[key] = std::move(o);
}

//纯色检测结果（死灯/暗灯/亮灯），按 R/G/B 打屏色分离
#define  KEY_PURE_DEAD_R_OUT                "key-pure-dead-r-out"
#define  KEY_PURE_DEAD_G_OUT                "key-pure-dead-g-out"
#define  KEY_PURE_DEAD_B_OUT                "key-pure-dead-b-out"
#define  KEY_PURE_DARK_R_OUT                "key-pure-dark-r-out"
#define  KEY_PURE_DARK_G_OUT                "key-pure-dark-g-out"
#define  KEY_PURE_DARK_B_OUT                "key-pure-dark-b-out"
#define  KEY_PURE_BRIGHT_R_OUT              "key-pure-bright-r-out"
#define  KEY_PURE_BRIGHT_G_OUT              "key-pure-bright-g-out"
#define  KEY_PURE_BRIGHT_B_OUT              "key-pure-bright-b-out"

//串色结果（R/G/B）
#define  KEY_COLOR_VAR_R_OUT                "key-color-var-r-out"
#define  KEY_COLOR_VAR_G_OUT                "key-color-var-g-out"
#define  KEY_COLOR_VAR_B_OUT                "key-color-var-b-out"

//混色结果（R/G/B）
#define  KEY_MIX_COLOR_R_OUT                "key-mix-color-r-out"
#define  KEY_MIX_COLOR_G_OUT                "key-mix-color-g-out"
#define  KEY_MIX_COLOR_B_OUT                "key-mix-color-b-out"

//串灯结果（R/G/B）
#define  KEY_STRING_LIGHT_R_OUT             "key-string-light-r-out"
#define  KEY_STRING_LIGHT_G_OUT             "key-string-light-g-out"
#define  KEY_STRING_LIGHT_B_OUT             "key-string-light-b-out"

// 缺陷类型（与首页 DetectItemKind 前 6 项顺序一致：死灯/暗灯/亮灯/串色/串灯/混色）
enum class LightenDefectNotifyKind : int {
    DeadLed = 0,
    DimLed = 1,
    BrightLed = 2,
    ColorVariation = 3,
    SeriesLed = 4,
    MixedColor = 5,
};

constexpr size_t kLightenDefectNotifyKindCount = 6;

// R/G/B 缺陷数量通知（MessageCenter::Center::Notify）
// 参数：QVariantList[0]=LightenDefectNotifyKind, [1]=R/G/B(0/1/2), [2]=缺陷数量(0=OK,>0=NG)
#define  EVENT_LIGHTEN_DEFECT_COUNT_RESULT  "event-lighten-defect-count-result"
#define  LIGHTEN_DEFECT_COUNT_NOTIFY_ARGS(defect_kind, color_idx, count) \
    static_cast<int>(defect_kind), static_cast<int>(color_idx), static_cast<int>(count)
											
//黑屏结果                                  
#define  KEY_IS_BLACK_SCREEN_OUT            "key-is-black-screen-out"
											
//黑屏亮灯结果                              
#define  KEY_BLACK_SCREEN_LIGHT_OUT         "key-black-screen-light-out"

//Flash 检测工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }）
#define  KEY_FLASH_STEP_CONFIG              "key-flash-step-config"

//点亮检测工步运行时配置（GLOBAL scope；JSON 同 light_detect config().params：lightDelayMs / featureFlags / cameras）
#define  KEY_LIGHT_DETECT_STEP_CONFIG       "key-light-detect-step-config"

//结果汇总工步运行时配置（GLOBAL scope；JSON 同 result_summary config().params：
//  defectCountLimits / specialDetectRules / panelCoord { width, height, coordSystemIndex }）
#define  KEY_RESULT_SUMMARY_STEP_CONFIG     "key-result-summary-step-config"

// 扫码工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }，对应系统配置「是否扫码」）
#define  KEY_SCAN_STEP_CONFIG               "key-scan-step-config"

// 上游对接工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }，对应系统配置「上游对接」）
#define  KEY_UPSTREAM_DOCK_STEP_CONFIG      "key-upstream-dock-step-config"

// 下游对接工步运行时配置（GLOBAL scope；JSON: { "enabled": bool }，对应系统配置「下游对接」）
#define  KEY_DOWNSTREAM_DOCK_STEP_CONFIG    "key-downstream-dock-step-config"

// 当前工件板卡序列号（FLOW scope；motion.scan_code 写入，下游检测/入库读取）
#define  KEY_BOARD_SN                       "sn"

// 当前检测产品型号（GLOBAL scope；默认配方 ID，由 HomeView 写入，历史入库等消费方读取）
#define  KEY_PRODUCT_MODEL                  "product-model"

// 检测时间（FLOW scope；yyyy-MM-dd HH:mm:ss，result_summary 写入，历史入库 inspected_at 来源）
#define  KEY_INSPECTED_AT                   "inspected-at"

// 单片检测耗时（FLOW scope；毫秒，result_summary 写入，历史入库 inspect_duration_ms 来源）
#define  KEY_INSPECT_DURATION_MS            "inspect-duration-ms"

// 检测起始时刻（FLOW scope；steady_clock 毫秒，由流程启动方写入，result_summary 计算耗时）
#define  KEY_INSPECT_START_MS               "inspect-start-ms"

//Flash检测结果（DetectResultStatus 整型值：0=OK，1=NG，2=RW）
#define  KEY_FLASH_DETECT_OUT               "key-flash-detect-out"

// Flash 检测结果通知（MessageCenter::Center::Notify）；参数：QVariantList[0] = bool（true=OK，false=NG）
#define  EVENT_FLASH_DETECT_RESULT          "event-flash-detect-result"

// 黑屏检测结果通知（MessageCenter::Center::Notify）；参数：QVariantList[0] = bool（true=OK，false=NG）
#define  EVENT_BLACK_SCREEN_DETECT_RESULT   "event-black-screen-detect-result"

// 黑屏亮灯检测结果通知（MessageCenter::Center::Notify）；参数：QVariantList[0] = 缺陷数量（int，0=OK，>0=NG）
#define  EVENT_BLACK_SCREEN_LIGHT_DETECT_RESULT "event-black-screen-light-detect-result"

// 检测项进度通知（MessageCenter::Center::Notify）
// 参数：QVariantList[0]=LightenDetectProgressKind
//       [1]=颜色通道（0=红，1=绿，2=蓝；无分色项填 kLightenDetectProgressColorNone）
//       [2]=LightenDetectProgressStatus
#define  EVENT_LIGHTEN_DETECT_PROGRESS          "event-lighten-detect-progress"
#define  LIGHTEN_DETECT_PROGRESS_NOTIFY_ARGS(item_kind, color_idx, status) \
    static_cast<int>(item_kind), static_cast<int>(color_idx), static_cast<int>(status)

/** 无 RGB 分色的检测项（黑屏检测 / 黑屏亮灯 / Flash）进度通知时颜色参数填此值 */
constexpr int kLightenDetectProgressColorNone = -1;

// 检测项进度状态（首页：未开始 / 进行中 / 已完成）
enum class LightenDetectProgressStatus : int {
    NotStarted = 0,
    InProgress = 1,
    Completed  = 2,
};

// 检测项进度种类（与首页 DetectItemKind 顺序一致）
enum class LightenDetectProgressKind : int {
    DeadLed = 0,
    DimLed = 1,
    BrightLed = 2,
    ColorVariation = 3,
    SeriesLed = 4,
    MixedColor = 5,
    BlackScreen = 6,
    BlackScreenOn = 7,
    Flash = 8,
};

// 综合判定结果（DetectResultStatus 整型值：0=OK, 1=NG, 2=RW）
#define  KEY_OVERALL_DETECT_RESULT_OUT      "overallDetectResult"

// 去重后缺陷点列表（FLOW scope；result_summary 写入，MES report_detection 读取）
#define  KEY_DEDUP_DEFECTS_OUT              "key-dedup-defects-out"

// 线性/区块不良汇总（FLOW scope；result_summary 写入；不参与点级去重）
#define  KEY_LINE_DEFECT_SUMMARY_OUT        "key-line-defect-summary-out"
#define  KEY_BLOCK_DEFECT_SUMMARY_OUT       "key-block-defect-summary-out"

// 去重缺陷类型（优先级从高到低：黑屏亮灯→死灯→暗灯→亮灯→串色→混色→串灯）
enum class LightenDedupDefectKind : int {
    BlackScreenLight = 0,
    DeadLed          = 1,
    DimLed           = 2,
    BrightLed        = 3,
    ColorVariation   = 4,
    MixedColor       = 5,
    SeriesLed        = 6,
};

constexpr size_t kLightenDedupDefectKindCount = 7;

// 去重后单条缺陷：坐标 (x,y)、芯片颜色通道 rgb（0=R/1=G/2=B）、缺陷类型
struct LightenDedupDefectPoint {
    int x    = 0;
    int y    = 0;
    int rgb  = 0; ///< 0=R，1=G，2=B
    LightenDedupDefectKind kind = LightenDedupDefectKind::DeadLed;
};

// 线性/区块不良汇总段：每段为同一问题内的缺陷点坐标列表
using LightenLineDefectSummary  = std::vector<std::vector<geometry::Point>>;
using LightenBlockDefectSummary = std::vector<std::vector<geometry::Point>>;

// 生产 KPI 统计表（lighten_production_kpi）更新通知（MessageCenter::Center::Notify）；无参数；首页监听后 loadKpi 刷新 KPI 条
#define  EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED "event-lighten-production-kpi-updated"

// 缺陷数量判定上下限（单项）
struct DefectCountLimitRange {
    int lower = 0;
    int upper = 0;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, lower);
        FROM_JSON_AUTO_SAME_NAME(j, upper);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, lower);
        TO_JSON_AUTO_SAME_NAME(j, upper);
    }
};

// 缺陷数量判定上下限（死灯 / 暗灯 / 亮灯 / 串色 / 串灯 / 混色 / 缺陷总数）
struct DefectCountLimits {
    DefectCountLimitRange dead_count{};             // 死灯数量
    DefectCountLimitRange dim_count{};              // 暗灯数量
    DefectCountLimitRange bright_count{};           // 亮灯数量
    DefectCountLimitRange color_variation_count{};  // 串色数量
    DefectCountLimitRange string_light_count{};     // 串灯数量
    DefectCountLimitRange mix_color_count{};        // 混色数量
    DefectCountLimitRange total_defect_count{};     // 缺陷总数

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, dead_count);
        FROM_JSON_AUTO_SAME_NAME(j, dim_count);
        FROM_JSON_AUTO_SAME_NAME(j, bright_count);
        FROM_JSON_AUTO_SAME_NAME(j, color_variation_count);
        FROM_JSON_AUTO_SAME_NAME(j, string_light_count);
        FROM_JSON_AUTO_SAME_NAME(j, mix_color_count);
        FROM_JSON_AUTO_SAME_NAME(j, total_defect_count);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, dead_count);
        TO_JSON_AUTO_SAME_NAME(j, dim_count);
        TO_JSON_AUTO_SAME_NAME(j, bright_count);
        TO_JSON_AUTO_SAME_NAME(j, color_variation_count);
        TO_JSON_AUTO_SAME_NAME(j, string_light_count);
        TO_JSON_AUTO_SAME_NAME(j, mix_color_count);
        TO_JSON_AUTO_SAME_NAME(j, total_defect_count);
    }
};

// 检测结果状态（OK / NG / RW）
enum class DetectResultStatus : int {
    Ok = 0,
    Ng = 1,
    Rw = 2,
};

// 处理方式（自动下料 / 报警）
enum class HandleMode : int {
    AutoUnload = 0,  ///< 自动下料
    Alarm = 1,         ///< 报警
};

inline void fromJsonHandleMode(const json& j, const std::string& key, HandleMode& mode)
{
    if (!j.contains(key) || j[key].is_null()) {
        return;
    }
    mode = static_cast<HandleMode>(j[key].get<int>());
}

inline void toJsonHandleMode(json& j, const std::string& key, HandleMode mode)
{
    j[key] = static_cast<int>(mode);
}

// 特殊检测规则（开启标志 / 灯点个数阈值 / 处理方式）
struct SpecialDetectRule {
    bool enabled = true;
    int lamp_count_threshold = 0;
    HandleMode handle_mode = HandleMode::Alarm;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, enabled);
        FROM_JSON_AUTO_SAME_NAME(j, lamp_count_threshold);
        fromJsonHandleMode(j, "handle_mode", handle_mode);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, enabled);
        TO_JSON_AUTO_SAME_NAME(j, lamp_count_threshold);
        toJsonHandleMode(j, "handle_mode", handle_mode);
    }
};

// 结果汇总工步：灯板分辨率与坐标系（对应配方 basic.width / basic.height / basic.coordSystemIndex）
struct ResultSummaryPanelCoord {
    int width             = 0; ///< 灯板宽度（像素）
    int height            = 0; ///< 灯板高度（像素）
    int coord_system_index = 0; ///< 0–7，与配方坐标系下拉一致

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, width);
        FROM_JSON_AUTO_SAME_NAME(j, height);
        fromJson(j, "coordSystemIndex", coord_system_index);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, width);
        TO_JSON_AUTO_SAME_NAME(j, height);
        TO_JSON_KEY(j, "coordSystemIndex", coord_system_index);
    }
};

// 特殊检测规则集合（大面积灭灯 / 非边缘缺陷 / 线不良 / 区块不良）
struct SpecialDetectRules {
    SpecialDetectRule large_area_dead_lamp{};  // 大面积灭灯
    SpecialDetectRule non_edge_defect{};       // 非边缘缺陷
    SpecialDetectRule line_defect{};             // 线不良
    SpecialDetectRule block_defect{};            // 区块不良

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, large_area_dead_lamp);
        FROM_JSON_AUTO_SAME_NAME(j, non_edge_defect);
        FROM_JSON_AUTO_SAME_NAME(j, line_defect);
        FROM_JSON_AUTO_SAME_NAME(j, block_defect);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, large_area_dead_lamp);
        TO_JSON_AUTO_SAME_NAME(j, non_edge_defect);
        TO_JSON_AUTO_SAME_NAME(j, line_defect);
        TO_JSON_AUTO_SAME_NAME(j, block_defect);
    }
};

// 按缺陷数量与上下限判定单项结果：count<=lower→OK，lower<count<=upper→RW，count>upper→NG。
inline DetectResultStatus JudgeDefectCountStatus(int count, const DefectCountLimitRange& limit)
{
    if (count <= limit.lower) {
        return DetectResultStatus::Ok;
    }
    if (count <= limit.upper) {
        return DetectResultStatus::Rw;
    }
    return DetectResultStatus::Ng;
}

// 合并多项判定，NG 优先于 RW，RW 优先于 OK。
inline DetectResultStatus MergeDetectResultStatus(DetectResultStatus a, DetectResultStatus b)
{
    if (a == DetectResultStatus::Ng || b == DetectResultStatus::Ng) {
        return DetectResultStatus::Ng;
    }
    if (a == DetectResultStatus::Rw || b == DetectResultStatus::Rw) {
        return DetectResultStatus::Rw;
    }
    return DetectResultStatus::Ok;
}

// 各检测项缺陷数量统计（由结果汇总工步从 FlowContext 读取并用于综合判级）。
struct LightenDetectDefectCounts {
    int dead_count             = 0;
    int dim_count              = 0;
    int bright_count           = 0;
    int color_variation_count  = 0;
    int string_light_count     = 0;
    int mix_color_count        = 0;
    int black_screen_light_count = 0;
    int is_black_screen        = 0; ///< 0=正常，1=黑屏
    int total_defect_count     = 0;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, dead_count);
        FROM_JSON_AUTO_SAME_NAME(j, dim_count);
        FROM_JSON_AUTO_SAME_NAME(j, bright_count);
        FROM_JSON_AUTO_SAME_NAME(j, color_variation_count);
        FROM_JSON_AUTO_SAME_NAME(j, string_light_count);
        FROM_JSON_AUTO_SAME_NAME(j, mix_color_count);
        FROM_JSON_AUTO_SAME_NAME(j, black_screen_light_count);
        FROM_JSON_AUTO_SAME_NAME(j, is_black_screen);
        FROM_JSON_AUTO_SAME_NAME(j, total_defect_count);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, dead_count);
        TO_JSON_AUTO_SAME_NAME(j, dim_count);
        TO_JSON_AUTO_SAME_NAME(j, bright_count);
        TO_JSON_AUTO_SAME_NAME(j, color_variation_count);
        TO_JSON_AUTO_SAME_NAME(j, string_light_count);
        TO_JSON_AUTO_SAME_NAME(j, mix_color_count);
        TO_JSON_AUTO_SAME_NAME(j, black_screen_light_count);
        TO_JSON_AUTO_SAME_NAME(j, is_black_screen);
        TO_JSON_AUTO_SAME_NAME(j, total_defect_count);
    }
};

/** 缺陷类型位掩码（与 lighten_inspection_history.defect_type 一致） */
constexpr int kLightenDefectTypeBitDeadLed           = 1 << 0;
constexpr int kLightenDefectTypeBitDimLed            = 1 << 1;
constexpr int kLightenDefectTypeBitBrightLed         = 1 << 2;
constexpr int kLightenDefectTypeBitColorVariation    = 1 << 3;
constexpr int kLightenDefectTypeBitSeriesLed         = 1 << 4;
constexpr int kLightenDefectTypeBitMixedColor        = 1 << 5;
constexpr int kLightenDefectTypeBitBlackScreenLight  = 1 << 6;
constexpr int kLightenDefectTypeBitSpecialDetect     = 1 << 7;

/** 由各类缺陷 RGB 汇总数量生成 defect_type 位掩码 */
inline int BuildDefectTypeMask(const LightenDetectDefectCounts& counts)
{
    int mask = 0;
    if (counts.dead_count > 0)
        mask |= kLightenDefectTypeBitDeadLed;
    if (counts.dim_count > 0)
        mask |= kLightenDefectTypeBitDimLed;
    if (counts.bright_count > 0)
        mask |= kLightenDefectTypeBitBrightLed;
    if (counts.color_variation_count > 0)
        mask |= kLightenDefectTypeBitColorVariation;
    if (counts.string_light_count > 0)
        mask |= kLightenDefectTypeBitSeriesLed;
    if (counts.mix_color_count > 0)
        mask |= kLightenDefectTypeBitMixedColor;
    if (counts.black_screen_light_count > 0)
        mask |= kLightenDefectTypeBitBlackScreenLight;
    return mask;
}

// 纯色检测目标颜色
enum class PureDetectColor : int {
    Red = 0,
    Green = 1,
    Blue = 2,
    Black = 3,
    White = 4,
};

inline void fromJsonPureDetectColor(const json& j, const std::string& key, PureDetectColor& color)
{
    if (!j.contains(key) || j[key].is_null()) {
        return;
    }
    color = static_cast<PureDetectColor>(j[key].get<int>());
}

inline void toJsonPureDetectColor(json& j, const std::string& key, PureDetectColor color)
{
    j[key] = static_cast<int>(color);
}

// 纯色检测 R/G/B 三色参数数量
constexpr size_t kPureColorChannelCount = 3;

// R/G/B 打屏色对应的 FlowContext 输出键（defect_kind × color_idx）
inline constexpr const char* kLightenDefectOutKeys[kLightenDefectNotifyKindCount][kPureColorChannelCount] = {
    {KEY_PURE_DEAD_R_OUT, KEY_PURE_DEAD_G_OUT, KEY_PURE_DEAD_B_OUT},
    {KEY_PURE_DARK_R_OUT, KEY_PURE_DARK_G_OUT, KEY_PURE_DARK_B_OUT},
    {KEY_PURE_BRIGHT_R_OUT, KEY_PURE_BRIGHT_G_OUT, KEY_PURE_BRIGHT_B_OUT},
    {KEY_COLOR_VAR_R_OUT, KEY_COLOR_VAR_G_OUT, KEY_COLOR_VAR_B_OUT},
    {KEY_STRING_LIGHT_R_OUT, KEY_STRING_LIGHT_G_OUT, KEY_STRING_LIGHT_B_OUT},
    {KEY_MIX_COLOR_R_OUT, KEY_MIX_COLOR_G_OUT, KEY_MIX_COLOR_B_OUT},
};

#define LIGHTEN_DEFECT_OUT_KEY(defect_kind, color_idx) \
    (common::algorithm::kLightenDefectOutKeys[static_cast<size_t>(defect_kind)][static_cast<size_t>(color_idx)])

// 将检测颜色枚举转为小写英文标识，用于输出文件名前缀（如 red / green / blue / white）
inline const char* PureDetectColorToString(PureDetectColor c)
{
    switch (c) {
    case PureDetectColor::Red:   return "red";
    case PureDetectColor::Green: return "green";
    case PureDetectColor::Blue:  return "blue";
    case PureDetectColor::Black: return "black";
    case PureDetectColor::White: return "white";
    default:                     return "unknown";
    }
}

// 拼接目录路径与文件名；path 或 fileName 为空时返回非空一方；自动处理末尾分隔符
inline std::string JoinPathAndFileName(const std::string& path, const std::string& fileName)
{
    if (path.empty()) {
        return fileName;
    }
    if (fileName.empty()) {
        return path;
    }
    if (path.back() == '/' || path.back() == '\\') {
        return path + fileName;
    }
    return path + "/" + fileName;
}

// 纯色检测阈值（死灯 / 暗灯 / 亮灯）
struct PureColorDetectThreshold {
    float thr_dead_R = 0.f;
    float thr_dead_G = 0.f;
    float thr_dead_B = 0.f;

    float thr_dim_min_R = 0.f;
    float thr_dim_max_R = 0.f;
    float thr_dim_min_G = 0.f;
    float thr_dim_max_G = 0.f;
    float thr_dim_min_B = 0.f;
    float thr_dim_max_B = 0.f;

    float thr_bright_R = 0.f;
    float thr_bright_G = 0.f;
    float thr_bright_B = 0.f;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_dead_R);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dead_G);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dead_B);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_min_R);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_max_R);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_min_G);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_max_G);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_min_B);
        FROM_JSON_AUTO_SAME_NAME(j, thr_dim_max_B);
        FROM_JSON_AUTO_SAME_NAME(j, thr_bright_R);
        FROM_JSON_AUTO_SAME_NAME(j, thr_bright_G);
        FROM_JSON_AUTO_SAME_NAME(j, thr_bright_B);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_dead_R);
        TO_JSON_AUTO_SAME_NAME(j, thr_dead_G);
        TO_JSON_AUTO_SAME_NAME(j, thr_dead_B);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_min_R);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_max_R);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_min_G);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_max_G);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_min_B);
        TO_JSON_AUTO_SAME_NAME(j, thr_dim_max_B);
        TO_JSON_AUTO_SAME_NAME(j, thr_bright_R);
        TO_JSON_AUTO_SAME_NAME(j, thr_bright_G);
        TO_JSON_AUTO_SAME_NAME(j, thr_bright_B);
    }
};

// 串色检测阈值，按红 / 绿 / 蓝三通道分别配置
struct ColorVariationDetectThreshold {
    float thr_color_variation_red_r = 0.f;
    float thr_color_variation_red_g = 0.f;
    float thr_color_variation_red_b = 0.f;

    float thr_color_variation_green_r = 0.f;
    float thr_color_variation_green_g = 0.f;
    float thr_color_variation_green_b = 0.f;

    float thr_color_variation_blue_r = 0.f;
    float thr_color_variation_blue_g = 0.f;
    float thr_color_variation_blue_b = 0.f;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_r);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_g);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_b);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_r);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_g);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_b);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_r);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_g);
        FROM_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_b);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_r);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_g);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_red_b);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_r);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_g);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_green_b);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_r);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_g);
        TO_JSON_AUTO_SAME_NAME(j, thr_color_variation_blue_b);
    }
};

// 纯色检测缺陷点
struct PureColorDefectPoint {
    int x = 0;
    int y = 0;

    float tempdata = 0.f;
    float origindata = 0.f;
    float comparedata = 0.f;

    float dead_thread = 0.f;
    float dark_thread = 0.f;
    float bright_thread = 0.f;
};

// 纯色检测与串色检测的输入参数
struct PureColorDetectParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> image;    // 待检测纯色图

    // --- 检测配置 ---
    PureDetectColor color = PureDetectColor::Red;
    bool virtual_flag = false;
    PureColorDetectThreshold detect_threshold{};
    ColorVariationDetectThreshold color_variation_detect_threshold{};

    // --- 检测项开关 ---
    bool dead_check = false;
    bool dark_check = false;
    bool bright_check = false;
    bool color_check = false;

    // --- 灯板与曝光 ---
    int led_w = 0;
    int led_h = 0;
    int cam_exposure = 0;
    int disp_brightness{ 255 };
    int disp_delay_ms{ 0 };

    // --- 相机 / Shot 公共参数 ---
    bool is_color_camera = true;
    float fixing_defective_thr = 3.f;

    // --- ROI ---
    bool use_auto_roi = true;
    AlgoPoint left_top{};
    AlgoPoint right_top{};
    AlgoPoint left_bottom{};
    AlgoPoint right_bottom{};
    AlgoSize image_size{};

    // --- 轮廓（纯色检测强制落盘 pure_contour；副相机由步骤层清空路径） ---
    bool is_side = false;
    int angle = 0;
    std::string pure_contour_file_name;

    // --- 原图输出 ---
    bool save_original_image_flag = false;
    std::string normal_image_file;

    // --- 结果图输出（纯色 → 串色） ---
    bool save_result_image_flag = false;
    std::string dead_png_file;
    std::string dark_png_file;
    std::string bright_png_file;
    std::string color_variation_png_file_name;

    // --- CSV 导出（纯色） ---
    std::string csv_pure_tempdata_file;
    std::string csv_pure_origindata_file;
    std::string csv_pure_comparedata_file;

    // --- CSV (串色) ---
    std::string csv_color_red_comparedata_file;
    std::string csv_color_green_comparedata_file;
    std::string csv_color_blue_comparedata_file;

    // 根据 color 生成各输出文件的基础文件名（不含目录路径，如 red / green / blue）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        const std::string prefix = PureDetectColorToString(color);

        // 轮廓
        pure_contour_file_name = prefix + ".contour";

        // 原图
        normal_image_file = prefix + ".jpg";

        // 结果图（死灯 / 暗灯 / 亮灯 / 串色）
        dead_png_file = prefix + "_dead.jpg";
        dark_png_file = prefix + "_dark.jpg";
        bright_png_file = prefix + "_bright.jpg";
        color_variation_png_file_name = prefix + "_color.jpg";

        // CSV（纯色）
        csv_pure_tempdata_file = prefix + "_tempdata.csv";
        csv_pure_origindata_file = prefix + "_origindata.csv";
        csv_pure_comparedata_file = prefix + "_comparedata.csv";

        // CSV（串色，按 R / G / B 通道）
        csv_color_red_comparedata_file = prefix + "_rcomparedata.csv";
        csv_color_green_comparedata_file = prefix + "_gcomparedata.csv";
        csv_color_blue_comparedata_file = prefix + "_bcomparedata.csv";
    }

    void reflectFromJson(const json& j)
    {
        fromJsonPureDetectColor(j, "color", color);
        FROM_JSON_AUTO_SAME_NAME(j, virtual_flag);
        FROM_JSON_AUTO_SAME_NAME(j, detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, color_variation_detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, dead_check);
        FROM_JSON_AUTO_SAME_NAME(j, dark_check);
        FROM_JSON_AUTO_SAME_NAME(j, bright_check);
        FROM_JSON_AUTO_SAME_NAME(j, color_check);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, disp_brightness);
        FROM_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        FROM_JSON_AUTO_SAME_NAME(j, is_color_camera);
        FROM_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
        FROM_JSON_AUTO_SAME_NAME(j, is_side);
        FROM_JSON_AUTO_SAME_NAME(j, angle);
    }

    void reflectToJson(json& j) const
    {
        toJsonPureDetectColor(j, "color", color);
        TO_JSON_AUTO_SAME_NAME(j, virtual_flag);
        TO_JSON_AUTO_SAME_NAME(j, detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, color_variation_detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, dead_check);
        TO_JSON_AUTO_SAME_NAME(j, dark_check);
        TO_JSON_AUTO_SAME_NAME(j, bright_check);
        TO_JSON_AUTO_SAME_NAME(j, color_check);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, disp_brightness);
        TO_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        TO_JSON_AUTO_SAME_NAME(j, is_color_camera);
        TO_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
        TO_JSON_AUTO_SAME_NAME(j, is_side);
        TO_JSON_AUTO_SAME_NAME(j, angle);
    }
};

// 混色检测阈值
struct MixColorDetectThreshold {
    float thr_mix_color_pre_red = 0.f;
    float thr_mix_color_red = 0.f;

    float thr_mix_color_pre_green = 0.f;
    float thr_mix_color_green = 0.f;

    float thr_mix_color_pre_blue = 0.f;
    float thr_mix_color_blue = 0.f;

    int mix_color_block_size = 5;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_red);
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_red);
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_green);
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_green);
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_blue);
        FROM_JSON_AUTO_SAME_NAME(j, thr_mix_color_blue);
        FROM_JSON_AUTO_SAME_NAME(j, mix_color_block_size);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_red);
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_red);
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_green);
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_green);
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_pre_blue);
        TO_JSON_AUTO_SAME_NAME(j, thr_mix_color_blue);
        TO_JSON_AUTO_SAME_NAME(j, mix_color_block_size);
    }
};

// 串色检测缺陷点
struct ColorVariationDefectPoint {
    int x = 0;
    int y = 0;

    float r_compare_data = 0.f;
    float g_compare_data = 0.f;
    float b_compare_data = 0.f;

    float color_r_threshold = 0.f;
    float color_g_threshold = 0.f;
    float color_b_threshold = 0.f;
};

using MixColorDefectPoint = common::geometry::Point;

// 混色检测输入参数（串色相关参数已迁移至 PureColorDetectParams）
struct MixColorVariationDetectParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> image;

    // --- 检测配置 ---
    PureDetectColor color = PureDetectColor::Red;
    bool virtual_flag = false;
    PureColorDetectThreshold pure_detect_threshold{};
    MixColorDetectThreshold mix_color_detect_threshold{};

    // --- 灯板（输入图像复用纯色检测采图，轮廓复用白场 white_*.contour） ---
    int led_w = 0;
    int led_h = 0;
    int cam_exposure = 0;

    // --- 相机 / Shot 公共参数 ---
    bool is_color_camera = true;
    float fixing_defective_thr = 3.f;

    // --- 轮廓（强制复用白场 white_*.contour 完整路径） ---
    std::string white_red_contour_file_name;
    std::string white_green_contour_file_name;
    std::string white_blue_contour_file_name;

    // --- 结果图输出 ---
    bool save_result_image_flag = false;
    std::string mix_color_png_file_name;

    // 根据 color 生成各输出文件的基础文件名（不含目录路径）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        // 白场轮廓（固定文件名，与 color 无关）
        white_red_contour_file_name = "white_red.contour";
        white_green_contour_file_name = "white_green.contour";
        white_blue_contour_file_name = "white_blue.contour";

        // 混色结果图
        const std::string prefix = PureDetectColorToString(color);
        mix_color_png_file_name = prefix + "_mixcolor.jpg";
    }

    void reflectFromJson(const json& j)
    {
        fromJsonPureDetectColor(j, "color", color);
        FROM_JSON_AUTO_SAME_NAME(j, virtual_flag);
        FROM_JSON_AUTO_SAME_NAME(j, pure_detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, mix_color_detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, is_color_camera);
        FROM_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }

    void reflectToJson(json& j) const
    {
        toJsonPureDetectColor(j, "color", color);
        TO_JSON_AUTO_SAME_NAME(j, virtual_flag);
        TO_JSON_AUTO_SAME_NAME(j, pure_detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, mix_color_detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, is_color_camera);
        TO_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }
};

// 串灯检测阈值
struct StringLightDetectThreshold {
    float thr_string_light_live_r = 0.f;
    float thr_string_light_live_g = 0.f;
    float thr_string_light_live_b = 0.f;

    float thr_string_light_dead_r = 0.f;
    float thr_string_light_dead_g = 0.f;
    float thr_string_light_dead_b = 0.f;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_live_r);
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_live_g);
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_live_b);
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_r);
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_g);
        FROM_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_b);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_live_r);
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_live_g);
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_live_b);
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_r);
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_g);
        TO_JSON_AUTO_SAME_NAME(j, thr_string_light_dead_b);
    }
};

// 串灯检测缺陷点
struct StringLightDefectPoint {
    int x = 0;
    int y = 0;

    float left_compare_data = 0.f;
    float right_compare_data = 0.f;

    float map_live_threshold = 0.f;
    float map_dead_threshold = 0.f;

    bool is_map_live = false;
};

// 串灯检测输入参数
struct StringLightDetectParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> left_image;
    std::shared_ptr<ImageBuffer> right_image;

    // --- 检测配置 ---
    PureDetectColor color = PureDetectColor::Red;
    StringLightDetectThreshold detect_threshold{};   

    // --- 灯板与曝光 ---
    int led_w = 0;
    int led_h = 0;
    int cam_exposure = 0;
    int disp_brightness{ 255 };
    int disp_delay_ms{ 0 };

    // --- 相机 / Shot 公共参数 ---
    bool is_color_camera = true;
    float fixing_defective_thr = 3.f;

    // --- ROI（纯色检测未开启时用于点定位，不保存 contour） ---
    bool use_auto_roi = true;
    AlgoPoint left_top{};
    AlgoPoint right_top{};
    AlgoPoint left_bottom{};
    AlgoPoint right_bottom{};
    AlgoSize image_size{};

    // --- 轮廓（纯色检测开启时复用纯色 contour 文件路径） ---
    std::string pure_contour_file_name;

    // --- 原图输出（黑场原图由 BlackScreenDetect / BlackScreenLightDetect 统一保存） ---
    bool save_original_image_flag = false;
    std::string normal_left_image_file_name;
    std::string normal_right_image_file_name;

    // --- 结果图输出（map_live → map_dead） ---
    bool save_result_image_flag = false;
    std::string string_dead_file_name;
    std::string string_live_file_name;

    // --- CSV 导出 ---
    std::string csv_left_comparedata_file_name;
    std::string csv_right_comparedata_file_name;

    // 根据 color 生成各输出文件的基础文件名（不含目录路径）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        const std::string prefix = PureDetectColorToString(color);

        // 轮廓（纯色检测开启时复用）
        pure_contour_file_name = prefix + ".contour";

        // 原图（左 / 右）
        normal_left_image_file_name = prefix + "_left.jpg";
        normal_right_image_file_name = prefix + "_right.jpg";

        // 结果图（串灯 live / dead）
        string_live_file_name = prefix + "_string_live.jpg";
        string_dead_file_name = prefix + "_string_dead.jpg";

        // CSV（左 / 右 comparedata）
        csv_left_comparedata_file_name = prefix + "_leftcomparedata.csv";
        csv_right_comparedata_file_name = prefix + "_rightcomparedata.csv";
    }

    void reflectFromJson(const json& j)
    {
        fromJsonPureDetectColor(j, "color", color);
        FROM_JSON_AUTO_SAME_NAME(j, detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, disp_brightness);
        FROM_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        FROM_JSON_AUTO_SAME_NAME(j, is_color_camera);
        FROM_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }

    void reflectToJson(json& j) const
    {
        toJsonPureDetectColor(j, "color", color);
        TO_JSON_AUTO_SAME_NAME(j, detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, disp_brightness);
        TO_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        TO_JSON_AUTO_SAME_NAME(j, is_color_camera);
        TO_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }
};

// 解析 R/G/B 三色通道参数数组；兼容旧版单对象配置（复制到三个元素）。
template<typename T>
inline void fromJsonRgbChannelParamsArray(const json& j,
                                          const std::string& key,
                                          std::array<T, kPureColorChannelCount>& arr)
{
    if (!j.contains(key) || j[key].is_null()) {
        return;
    }
    if (j[key].is_array()) {
        fromJson(j, key, arr);
    } else if (j[key].is_object()) {
        for (auto& elem : arr) {
            elem.reflectFromJson(j[key]);
        }
    }
    arr[0].color = PureDetectColor::Red;
    arr[1].color = PureDetectColor::Green;
    arr[2].color = PureDetectColor::Blue;
}

// 解析 R/G/B 三色布尔开关；兼容旧版单 bool 配置（复制到三个元素）。
inline void fromJsonRgbBoolFlag(const json& j,
                                const std::string& key,
                                std::array<bool, kPureColorChannelCount>& arr)
{
    if (!j.contains(key) || j[key].is_null()) {
        return;
    }
    if (j[key].is_array()) {
        fromJson(j, key, arr);
    } else if (j[key].is_boolean()) {
        arr.fill(j[key].get<bool>());
    }
}

// 序列化 R/G/B 三色通道参数数组。
template<typename T>
inline void toJsonRgbChannelParamsArray(json& j,
                                        const std::string& key,
                                        const std::array<T, kPureColorChannelCount>& arr)
{
    toJson(j[key], arr);
}

// 序列化 R/G/B 三色布尔开关数组。
inline void toJsonRgbBoolFlag(json& j,
                              const std::string& key,
                              const std::array<bool, kPureColorChannelCount>& arr)
{
    toJson(j[key], arr);
}

// 黑屏亮灯检测阈值
struct BlackScreenLightDetectThreshold {
    float thr_black_light = 0.f;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_black_light);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_black_light);
    }
};

using BlackScreenLightDefectPoint = common::geometry::Point;

// 黑屏亮灯检测输入参数（仅使用黑场 bg_image）
struct BlackScreenLightDetectParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> bg_image;

    // --- 检测配置 ---
    BlackScreenLightDetectThreshold detect_threshold{};

    // --- 灯板（bg_image 复用 black_screen 采图，轮廓复用白场 white_green.contour） ---
    int led_w = 0;
    int led_h = 0;
    int cam_exposure = 0;

    // --- 相机 / Shot 公共参数 ---
    bool is_color_camera = true;
    float fixing_defective_thr = 3.f;

    // --- 轮廓（强制复用白场 white_green.contour 路径） ---
    std::string white_green_contour_file_name;

    // --- 原图输出 ---
    bool save_original_image_flag = false;
    std::string black_image_file_name;

    // --- 结果图输出 ---
    bool save_result_image_flag = false;
    std::string black_screen_light_png_file_name;

    // 生成各输出文件的基础文件名（不含目录路径，固定命名）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        // 白场轮廓
        white_green_contour_file_name = "white_green.contour";

        // 原图
        black_image_file_name = "black.jpg";

        // 结果图
        black_screen_light_png_file_name = "black_light.jpg";
    }

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, is_color_camera);
        FROM_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, is_color_camera);
        TO_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
    }
};

// 黑屏检测阈值
struct BlackScreenDetectThreshold {
    float thr_black_screen = 0.f;

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, thr_black_screen);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, thr_black_screen);
    }
};

// 黑屏检测输入参数
struct BlackScreenDetectParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> black_image;
    std::shared_ptr<ImageBuffer> normal_image;

    // --- 检测配置 ---
    BlackScreenDetectThreshold detect_threshold{};

    // --- 灯板（输入图像由步骤层统一采集） ---
    int led_w = 0;
    int led_h = 0;
    int disp_delay_ms{ 0 };           // 打屏稳定延迟（毫秒）
    int black_cam_exposure = 0;       // 打黑屏相机曝光
    int white_cam_exposure = 0;       // 打白屏相机曝光
    int black_disp_brightness{ 0 };   // 黑屏打屏亮度
    int white_disp_brightness{ 255 }; // 白屏打屏亮度

    // --- 原图输出（黑/白场原图由步骤层 / WhiteContourGenParams 统一保存） ---
    bool save_original_image_flag = false;
    std::string black_image_file_name;
    std::string white_image_file_name;

    // 生成各输出文件的基础文件名（不含目录路径，固定命名）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        black_image_file_name = "black.jpg";
        white_image_file_name = "white.jpg";
    }

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, detect_threshold);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        FROM_JSON_AUTO_SAME_NAME(j, black_cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, white_cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, black_disp_brightness);
        FROM_JSON_AUTO_SAME_NAME(j, white_disp_brightness);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, detect_threshold);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, disp_delay_ms);
        TO_JSON_AUTO_SAME_NAME(j, black_cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, white_cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, black_disp_brightness);
        TO_JSON_AUTO_SAME_NAME(j, white_disp_brightness);
    }
};

// 白场轮廓生成参数（ShotMeter::RunWhite 异步生成 white_red/green/blue.contour，供混色/黑屏亮灯复用）。
struct WhiteContourGenParams {
    // --- 输入图像 ---
    std::shared_ptr<ImageBuffer> image;

    // --- 检测配置 ---
    PureDetectColor color = PureDetectColor::White;
    bool virtual_flag = false;

    // --- 灯板与曝光 ---
    int led_w = 0;
    int led_h = 0;
    int cam_exposure = 0;

    // --- 相机 / Shot 公共参数 ---
    bool is_color_camera = true;
    float fixing_defective_thr = 3.f;

    // --- ROI ---
    bool use_auto_roi = true;
    AlgoPoint left_top{};
    AlgoPoint right_top{};
    AlgoPoint left_bottom{};
    AlgoPoint right_bottom{};
    AlgoSize image_size{};

    // --- 轮廓（打白屏强制落盘 white_red/green/blue.contour；正视 is_side=false） ---
    int angle = 0;
    std::string white_red_contour_file_name;
    std::string white_green_contour_file_name;
    std::string white_blue_contour_file_name;

    // --- 原图输出 ---
    bool save_original_image_flag = false;
    std::string white_image_file_name;

    // 生成各输出文件的基础文件名（不含目录路径，固定命名）
    // 调用方可配合 JoinPathAndFileName 拼接完整路径
    void adjustFileName()
    {
        // 白场轮廓（R / G / B）
        white_red_contour_file_name = "white_red.contour";
        white_green_contour_file_name = "white_green.contour";
        white_blue_contour_file_name = "white_blue.contour";

        // 原图
        white_image_file_name = "white.jpg";
    }

    void reflectFromJson(const json& j)
    {
        fromJsonPureDetectColor(j, "color", color);
        FROM_JSON_AUTO_SAME_NAME(j, virtual_flag);
        FROM_JSON_AUTO_SAME_NAME(j, led_w);
        FROM_JSON_AUTO_SAME_NAME(j, led_h);
        FROM_JSON_AUTO_SAME_NAME(j, cam_exposure);
        FROM_JSON_AUTO_SAME_NAME(j, is_color_camera);
        FROM_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
        FROM_JSON_AUTO_SAME_NAME(j, angle);
    }

    void reflectToJson(json& j) const
    {
        toJsonPureDetectColor(j, "color", color);
        TO_JSON_AUTO_SAME_NAME(j, virtual_flag);
        TO_JSON_AUTO_SAME_NAME(j, led_w);
        TO_JSON_AUTO_SAME_NAME(j, led_h);
        TO_JSON_AUTO_SAME_NAME(j, cam_exposure);
        TO_JSON_AUTO_SAME_NAME(j, is_color_camera);
        TO_JSON_AUTO_SAME_NAME(j, fixing_defective_thr);
        TO_JSON_AUTO_SAME_NAME(j, angle);
    }
};

// 点亮检测各子功能启用标志。
// dead_light / dark_light / bright_light / color_variation / mix_color / string_light
// 均为 R/G/B 三色数组，下标 0/1/2 分别对应打红屏 / 绿屏 / 蓝屏时是否启用该项检测。
struct LightenDetectFeatureFlags {
    bool black_screen{ false };    // 黑屏
    bool black_light{ false };     // 黑屏亮灯
    std::array<bool, kPureColorChannelCount> dead_light{};      // 死灯
    std::array<bool, kPureColorChannelCount> dark_light{};      // 暗灯
    std::array<bool, kPureColorChannelCount> bright_light{};    // 亮灯
    std::array<bool, kPureColorChannelCount> color_variation{}; // 串色
    std::array<bool, kPureColorChannelCount> mix_color{};       // 混色
    std::array<bool, kPureColorChannelCount> string_light{};    // 串灯

    static bool anyFlag(const std::array<bool, kPureColorChannelCount>& flags)
    {
        for (bool v : flags) {
            if (v) {
                return true;
            }
        }
        return false;
    }

    bool runPureAlgoForColor(size_t color_idx) const
    {
        return dead_light[color_idx] || dark_light[color_idx]
            || bright_light[color_idx] || color_variation[color_idx];
    }

    bool anyPureAlgo() const
    {
        for (size_t i = 0; i < kPureColorChannelCount; ++i) {
            if (runPureAlgoForColor(i)) {
                return true;
            }
        }
        return false;
    }

    bool anyMixColor() const { return anyFlag(mix_color); }
    bool anyStringLight() const { return anyFlag(string_light); }

    bool needColorCycle() const
    {
        for (size_t i = 0; i < kPureColorChannelCount; ++i) {
            if (runPureAlgoForColor(i) || mix_color[i]) {
                return true;
            }
        }
        return false;
    }

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, black_screen);
        FROM_JSON_AUTO_SAME_NAME(j, black_light);
        fromJsonRgbBoolFlag(j, "dead_light", dead_light);
        fromJsonRgbBoolFlag(j, "dark_light", dark_light);
        fromJsonRgbBoolFlag(j, "bright_light", bright_light);
        fromJsonRgbBoolFlag(j, "color_variation", color_variation);
        fromJsonRgbBoolFlag(j, "mix_color", mix_color);
        fromJsonRgbBoolFlag(j, "string_light", string_light);
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, black_screen);
        TO_JSON_AUTO_SAME_NAME(j, black_light);
        toJsonRgbBoolFlag(j, "dead_light", dead_light);
        toJsonRgbBoolFlag(j, "dark_light", dark_light);
        toJsonRgbBoolFlag(j, "bright_light", bright_light);
        toJsonRgbBoolFlag(j, "color_variation", color_variation);
        toJsonRgbBoolFlag(j, "mix_color", mix_color);
        toJsonRgbBoolFlag(j, "string_light", string_light);
    }
};

// 点亮检测各子阶段算法参数（由 loadParam 从 params JSON 文件加载，按相机维度存于 map）。
// cameraPath 由 HomeView 从 device_config.json 注入，用于图像/轮廓落盘子目录。
struct LightenDetectAlgoParams {
    std::string cameraPath;                  // 相机输出子目录（device_config.json DeviceManager.Camera.*.path）
    std::string output_image_root;           // 结果图输出根目录
    std::string output_data_root;            // CSV 等数据输出根目录
    bool save_original_image_flag{ false };  // 原图保存总开关，分发至各子参数
    bool save_result_image_flag{ false };    // 结果图保存总开关，分发至各子参数

    // --- ROI（相机级统一配置，由 loadParam 从 JSON 加载后分发至各子算法参数） ---
    bool use_auto_roi = true;
    AlgoPoint left_top{};
    AlgoPoint right_top{};
    AlgoPoint left_bottom{};
    AlgoPoint right_bottom{};
    AlgoSize image_size{};

    // --- 黑屏相关算法（输入图为步骤层已采集的参考图） ---
    BlackScreenDetectParams black_screen;     // 黑屏判定（黑场 + 白场参考图）
    BlackScreenLightDetectParams black_light;   // 黑屏亮灯（黑场参考图 + white_green.contour）

    // --- 参考图采图（步骤层统一采集，算法层仅消费 ImageBuffer） ---
    WhiteContourGenParams white;                    // 白场轮廓生成

    // --- R/G/B 三色循环检测 ---
    std::array<PureColorDetectParams, kPureColorChannelCount> pure_color{};        // 纯色（含串色）
    std::array<MixColorVariationDetectParams, kPureColorChannelCount> mix_color{};   // 混色（复用白场轮廓）
    std::array<StringLightDetectParams, kPureColorChannelCount> string_light{};    // 串灯（复用纯色轮廓或 ROI）

    // 将相机级 ROI 同步到含 ROI 字段的子算法参数（不参与子结构体 JSON 解析）。
    void applyRoiToSubParams()
    {
        auto apply = [this](auto& sub) {
            sub.use_auto_roi = use_auto_roi;
            sub.left_top = left_top;
            sub.right_top = right_top;
            sub.left_bottom = left_bottom;
            sub.right_bottom = right_bottom;
            sub.image_size = image_size;
        };
        apply(white);
        for (auto& pc : pure_color) {
            apply(pc);
        }
        for (auto& sl : string_light) {
            apply(sl);
        }
    }

    // 将相机级原图/结果图保存开关同步到各子算法参数（不参与子结构体 JSON 解析）。
    void applySaveImageFlagsToSubParams()
    {
        black_screen.save_original_image_flag = save_original_image_flag;

        black_light.save_original_image_flag = save_original_image_flag;
        black_light.save_result_image_flag = save_result_image_flag;

        white.save_original_image_flag = save_original_image_flag;

        for (auto& pc : pure_color) {
            pc.save_original_image_flag = save_original_image_flag;
            pc.save_result_image_flag = save_result_image_flag;
        }

        for (auto& mc : mix_color) {
            mc.save_result_image_flag = save_result_image_flag;
        }

        for (auto& sl : string_light) {
            sl.save_original_image_flag = save_original_image_flag;
            sl.save_result_image_flag = save_result_image_flag;
        }
    }

    void reflectFromJson(const json& j)
    {
        FROM_JSON_AUTO_SAME_NAME(j, cameraPath);
        FROM_JSON_AUTO_SAME_NAME(j, output_image_root);
        FROM_JSON_AUTO_SAME_NAME(j, output_data_root);
        FROM_JSON_AUTO_SAME_NAME(j, save_original_image_flag);
        FROM_JSON_AUTO_SAME_NAME(j, save_result_image_flag);
        FROM_JSON_AUTO_SAME_NAME(j, use_auto_roi);
        fromJsonAlgoPoint(j, "left_top", left_top);
        fromJsonAlgoPoint(j, "right_top", right_top);
        fromJsonAlgoPoint(j, "left_bottom", left_bottom);
        fromJsonAlgoPoint(j, "right_bottom", right_bottom);
        fromJsonAlgoSize(j, "image_size", image_size);
        FROM_JSON_AUTO_SAME_NAME(j, black_screen);
        FROM_JSON_AUTO_SAME_NAME(j, black_light);
        FROM_JSON_AUTO_SAME_NAME(j, white);
        fromJsonRgbChannelParamsArray(j, "pure_color", pure_color);
        fromJsonRgbChannelParamsArray(j, "mix_color", mix_color);
        fromJsonRgbChannelParamsArray(j, "string_light", string_light);
        applyRoiToSubParams();
        applySaveImageFlagsToSubParams();
    }

    void reflectToJson(json& j) const
    {
        TO_JSON_AUTO_SAME_NAME(j, cameraPath);
        TO_JSON_AUTO_SAME_NAME(j, output_image_root);
        TO_JSON_AUTO_SAME_NAME(j, output_data_root);
        TO_JSON_AUTO_SAME_NAME(j, save_original_image_flag);
        TO_JSON_AUTO_SAME_NAME(j, save_result_image_flag);
        TO_JSON_AUTO_SAME_NAME(j, use_auto_roi);
        toJsonAlgoPoint(j, "left_top", left_top);
        toJsonAlgoPoint(j, "right_top", right_top);
        toJsonAlgoPoint(j, "left_bottom", left_bottom);
        toJsonAlgoPoint(j, "right_bottom", right_bottom);
        toJsonAlgoSize(j, "image_size", image_size);
        TO_JSON_AUTO_SAME_NAME(j, black_screen);
        TO_JSON_AUTO_SAME_NAME(j, black_light);
        TO_JSON_AUTO_SAME_NAME(j, white);
        toJsonRgbChannelParamsArray(j, "pure_color", pure_color);
        toJsonRgbChannelParamsArray(j, "mix_color", mix_color);
        toJsonRgbChannelParamsArray(j, "string_light", string_light);
    }
};

} // namespace algorithm
} // namespace common
