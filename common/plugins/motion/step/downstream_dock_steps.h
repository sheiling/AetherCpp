#pragma once

/**
 * @file downstream_dock_steps.h
 * @brief 下游对接出料工步（typeName: "motion.downstream_dock"）声明。
 *
 * 执行流程（下游对接开启时）：
 *   1. 读取 KEY_OVERALL_DETECT_RESULT_OUT，写入 OK/NG 输出信号
 *   2. WriteIO(本机有板信号, true)
 *   3. 轮询 ReadIO(下游响应信号) 直至为 true
 *   4. WriteIO(传送带方向信号, beltDirection) → WriteIO(传送带启动信号, true)
 *   5. 轮询 ReadIO(出料感应信号) 直至为 false — 出料完成
 *   6. sleep(stopBeltDelayMs)
 *   7. WriteIO(传送带启动信号, false)
 *   8. WriteIO(本机有板信号, false)
 *   9. 清除 OK/NG 输出信号（写 false）
 *
 * 运行时开关（GLOBAL）：
 *   - KEY_DOWNSTREAM_DOCK_STEP_CONFIG.enabled ← 系统配置「下游对接」
 *   - 由 HomeView::slot_initialize → setDownstreamDockStepConfig 写入
 *   - enabled=false 时不访问硬件，直接返回 ok
 *
 * JSON params 示例：
 * @code
 * {
 *   "okOutputName": "DO_DOWNSTREAM_OK",
 *   "ngOutputName": "DO_DOWNSTREAM_NG",
 *   "boardPresentOutputName": "DO_BOARD_PRESENT",
 *   "downstreamResponseInputName": "DI_DOWNSTREAM_READY",
 *   "beltDirectionOutputName": "DO_BELT_DIR",
 *   "beltDirection": true,
 *   "beltStartOutputName": "DO_BELT_START",
 *   "dischargeInductionInputName": "DI_DISCHARGE",
 *   "stopBeltDelayMs": 500,
 *   "iMSpace": 50,
 *   "iMTimeout": 5000
 * }
 * @endcode
 *
 * 未在 JSON 中配置的数值参数使用成员默认值（stopBeltDelayMs=500，iMSpace=50，iMTimeout=5000）。
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>

#include <scheduler/step/i_step.h>

namespace step {

/// 下游对接出料工步：输出 OK/NG → 有板 → 等待下游 → 启皮带 → 出料 → 停皮带 → 无板 → 清信号。
class DownstreamDockSteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.downstream_dock"; }

private:
    /// OK 判定输出 DO 名称（JSON: okOutputName）。
    std::string m_okOutputName;
    /// NG 判定输出 DO 名称（JSON: ngOutputName）。
    std::string m_ngOutputName;
    /// 本机有板通知 DO 名称（JSON: boardPresentOutputName）。
    std::string m_boardPresentOutputName;
    /// 下游收料就绪 DI 名称（JSON: downstreamResponseInputName）。
    std::string m_downstreamResponseInputName;
    /// 传送带方向 DO 名称（JSON: beltDirectionOutputName）。
    std::string m_beltDirectionOutputName;
    /// 写入方向 DO 的目标电平（JSON: beltDirection）。
    bool        m_beltDirection{ false };
    /// 传送带启动 DO 名称（JSON: beltStartOutputName）。
    std::string m_beltStartOutputName;
    /// 出料感应 DI 名称；读 false 表示出料完成（JSON: dischargeInductionInputName）。
    std::string m_dischargeInductionInputName;
    /// 出料到位后、停止皮带前的延时毫秒（JSON: stopBeltDelayMs，默认 500）。
    int         m_stopBeltDelayMs{ 500 };
    /// DI 轮询间隔毫秒（JSON: iMSpace，默认 50）。
    int         m_iMSpace{ 50 };
    /// 等待 DI 超时毫秒（JSON: iMTimeout，默认 5000）。
    int         m_iMTimeout{ 5000 };
};

} // namespace step
