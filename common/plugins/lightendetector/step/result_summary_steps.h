#pragma once

/**
 * @file result_summary_steps.h
 * @brief 点亮检测结果汇总与综合判定步骤（typeName: "lighten.result_summary"）声明。
 *
 * 从 FlowContext 读取各子检测项的缺陷点列表，按 panelCoord 转换坐标后统计数量并判级，
 * 按 specialDetectRules 执行线性/区块不良检测，合并 Flash 检测结果，输出 overallDetectResult，
 * 并追加检测历史记录；缺陷点按 (x,y,rgb) 去重后写入 KEY_DEDUP_DEFECTS_OUT，
 * 并按第三方软件路径导出坐标汇总 CSV（point/年/月/YYYYMMDD.csv）。
 * 线性/区块不良汇总写入 KEY_LINE_DEFECT_SUMMARY_OUT / KEY_BLOCK_DEFECT_SUMMARY_OUT。
 */

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/include/algorithm_data/lightendetector_data.h>

#include <scheduler/step/i_step.h>

namespace step {

// 汇总点亮检测各子项缺陷数量并综合判定的调度步骤（typeName: "lighten.result_summary"）。
// 从 FlowContext 读取 lighten.light_detect / lighten.flash_detect 输出，按 defectCountLimits 判级，
// 并将 overallDetectResult（OK/NG/RW）写回 FlowContext。
class ResultSummarySteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "lighten.result_summary"; }

private:
    // 从运行时 JSON 加载 defectCountLimits / specialDetectRules / panelCoord / thirdPartySoftwarePath。
    bool loadParamsFromJson(const nlohmann::json& root);

    // 将 FlowContext 中 6 类 × R/G/B 缺陷点坐标转换至配方坐标系并写回。
    void transformAllDefectCoordinates(scheduler::FlowContext& ctx) const;

    // 从 FlowContext 各输出键统计各类缺陷数量及总缺陷数。
    common::algorithm::LightenDetectDefectCounts collectDefectCounts(
        scheduler::FlowContext& ctx) const;
    // 按 m_limits 逐项判级，合并黑屏与 Flash 结果，返回综合 OK/NG/RW 状态。
    common::algorithm::DetectResultStatus judgeOverallStatus(
        const common::algorithm::LightenDetectDefectCounts& counts,
        common::algorithm::DetectResultStatus flash_result) const;
    // 按 specialDetectRules 执行线性/区块不良检测（R/G/B 分色缺陷汇总后判定），检出则 NG。
    common::algorithm::DetectResultStatus judgeLineAndBlockDefects(
        scheduler::FlowContext& ctx) const;

    // 将去重缺陷与线性/区块不良汇总写入第三方路径坐标 CSV（路径为空或不可写时静默跳过）。
    void writeThirdPartyCoordSummaryCsv(
        const scheduler::FlowContext& ctx,
        common::algorithm::DetectResultStatus overall) const;

    // 各缺陷数量判定上下限（JSON: defectCountLimits；由 KEY_RESULT_SUMMARY_STEP_CONFIG 加载）。
    common::algorithm::DefectCountLimits m_limits{};
    // 特殊检测规则（JSON: specialDetectRules；由 KEY_RESULT_SUMMARY_STEP_CONFIG 加载）。
    common::algorithm::SpecialDetectRules m_special_detect_rules{};
    // 灯板分辨率与坐标系（JSON: panelCoord；由 KEY_RESULT_SUMMARY_STEP_CONFIG 加载）。
    common::algorithm::ResultSummaryPanelCoord m_panel_coord{};
    // 第三方软件根路径（JSON: thirdPartySoftwarePath；系统配置「第三方软件路径」）。
    std::string m_third_party_path{};
};

} // namespace step
