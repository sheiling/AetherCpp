#pragma once

/**
 * @file scan_code_steps.h
 * @brief 扫码枪读条码工步（typeName: "motion.scan_code"）声明。
 *
 * 通过 DeviceManager::ScannerHandler 获取扫码枪，发送触发指令后轮询等待条码；
 * 成功后将条码写入 FlowContext 的 KEY_BOARD_SN（FLOW scope）。
 *
 * 运行时开关（GLOBAL）：
 *   - KEY_SCAN_STEP_CONFIG.enabled ← 系统配置「是否扫码」
 *   - 由 HomeView::slot_initialize → setScanStepConfig 写入
 *   - enabled=false 时不访问硬件，以 yyyyMMddHHmmss 作为条码并返回 ok
 *
 * 轮询策略：
 *   - iMSpace <= 0：归一化为 50ms
 *   - iMTimeout <= 0：归一化为 5000ms
 *   - 无扫码失败重试（单次触发 + 超时即失败）
 *
 * JSON params 示例：
 * @code
 * {
 *   "scanner": "Scanner1",
 *   "triggerCmd": "start",
 *   "iMTimeout": 5000,
 *   "iMSpace": 50,
 *   "scanFailedCode": "NOREAD"
 * }
 * @endcode
 */

#include <string>

#include <common/3rdparty/nlohmann/json.hpp>
#include <common/include/algorithm_data/lightendetector_data.h>

#include <scheduler/step/i_step.h>

namespace step {

/// 扫码枪读条码工步；扫码开关关闭时使用当前时间戳作为条码。
class ScanCodeSteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "motion.scan_code"; }

private:
    /// 扫码枪配置键名（JSON: scanner；对应 device_config.json DeviceManager.Scanner 段键名）。
    std::string m_scanner;
    /// 触发扫码指令原文（JSON: triggerCmd；由 Request 异步下发）。
    std::string m_trigger_cmd;
    /// 失败条码；扫到与该值完全一致的条码时判定为扫码失败（JSON: scanFailedCode；空串表示不校验）。
    std::string m_scan_failed_code;
    /// 等待条码超时毫秒数；<=0 时归一化为 5000ms（JSON: iMTimeout）。
    int         m_iMTimeout{ 5000 };
    /// 触发后轮询间隔毫秒数；<=0 时归一化为 50ms（JSON: iMSpace）。
    int         m_iMSpace{ 50 };
};

} // namespace step
