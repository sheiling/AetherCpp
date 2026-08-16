#include "scan_code_steps.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>

#include "motion_step_common.h"
#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/utility/json_tool/reflector.h>
#include <common/utility/string_tool/time_format.hpp>

#include <scheduler/step/step_factory.h>

/**
 * @file scan_code_steps.cpp
 * @brief ScanCodeSteps 的实现。
 *
 * 执行流程：
 *   1. 清除 KEY_BOARD_SN → 读取 KEY_SCAN_STEP_CONFIG.enabled；关闭时用 yyyyMMddHHmmss 写入 KEY_BOARD_SN 并返回 ok
 *   2. 校验 scanner / triggerCmd → 获取 ScannerHandler（未连接时尝试 Init 一次）
 *   3. RegisterRspCB 异步收集条码 → Request(triggerCmd) → 按 iMSpace 轮询直至 iMTimeout
 *   4. 条码与 scanFailedCode 一致则失败；否则写入 KEY_BOARD_SN（FLOW scope）并返回 ok
 *
 * 轮询策略（iMTimeout / iMSpace 在 reflectFromJson 内归一化）：
 *   - iMSpace <= 0 → 50ms
 *   - iMTimeout <= 0 → 5000ms
 *
 * 错误码：
 *   -20  scanner 为空
 *   -21  triggerCmd 为空
 *   -22  DeviceManager 不可用
 *   -23  扫码枪句柄未找到
 *   -24  扫码枪未连接且 Init 失败
 *   -25  触发指令发送失败
 *   -26  等待条码超时
 *   -27  扫到失败条码（与 scanFailedCode 一致）
 *   -99  未捕获 std::exception
 *
 * 注册名 "motion.scan_code"；实现位于 motion 插件。
 */

namespace step {
namespace {

using json = nlohmann::json;

constexpr int kDefaultPollSpaceMs   = 50;
constexpr int kDefaultScanTimeoutMs = 5000;

std::string TrimBarcode(std::string value)
{
    // 去除首尾空白与尾部 NUL，避免误判空条码
    auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(),
                std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(),
                value.end());
    while (!value.empty() && (value.back() == '\0')) {
        value.pop_back();
    }
    return value;
}

void NormalizeScanParams(int& timeout_ms, int& poll_space_ms)
{
    if (timeout_ms <= 0) {
        timeout_ms = kDefaultScanTimeoutMs;
    }
    if (poll_space_ms <= 0) {
        poll_space_ms = kDefaultPollSpaceMs;
    }
}

void ClearBoardSnOutput(scheduler::FlowContext& ctx)
{
    ctx.remove(KEY_BOARD_SN, scheduler::VariableScope::FLOW);
}

void SetBoardSn(scheduler::FlowContext& ctx, const std::string& sn)
{
    ctx.set(KEY_BOARD_SN, sn, scheduler::VariableScope::FLOW);
}

bool IsFailedBarcode(const std::string& barcode, const std::string& failed_code)
{
    return !failed_code.empty() && barcode == failed_code;
}

} // namespace

void ScanCodeSteps::reflectFromJson(const json& j)
{
    fromJson(j, "scanner", m_scanner);
    fromJson(j, "triggerCmd", m_trigger_cmd);
    fromJson(j, "scanFailedCode", m_scan_failed_code);
    fromJson(j, "iMTimeout", m_iMTimeout);
    fromJson(j, "iMSpace", m_iMSpace);

    m_scan_failed_code = TrimBarcode(std::move(m_scan_failed_code));
    NormalizeScanParams(m_iMTimeout, m_iMSpace);
}

scheduler::StepResult ScanCodeSteps::execute(scheduler::FlowContext& ctx)
{
    const std::string& label = name();

    try {
        ClearBoardSnOutput(ctx);
        // 扫码开关关闭：不访问硬件，以时间戳作为 SN
        if (!detail::IsRuntimeStepEnabled(ctx, KEY_SCAN_STEP_CONFIG)) {
            SetBoardSn(ctx, common::utility::TimeFormat::GetCurrentTimeStr());
            return scheduler::StepResult::ok();
        }

        // 参数校验
        if (m_scanner.empty()) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: scanner is empty", -20);
        }
        if (m_trigger_cmd.empty()) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: triggerCmd is empty", -21);
        }

        auto dm = CAPP->make<common::device::DeviceManager>();
        if (!dm) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: device manager not found", -22);
        }

        const auto scanner = dm->ScannerHandler(m_scanner);
        if (!scanner) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: scanner handler not found", -23);
        }

        // 未连接时尝试 Init 一次
        if (!scanner->IsConnected() && !scanner->Init()) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: scanner not connected", -24);
        }

        // 异步回调收集条码
        struct ScanCapture {
            std::mutex  mtx;
            std::string barcode;
            bool        received{ false };
        };
        auto capture = std::make_shared<ScanCapture>();
        scanner->RegisterRspCB([capture](const std::string& payload) {
            const std::string trimmed = TrimBarcode(payload);
            if (trimmed.empty()) {
                return;
            }
            std::lock_guard<std::mutex> lock(capture->mtx);
            capture->barcode  = trimmed;
            capture->received = true;
        });

        // 下发触发指令
        if (!scanner->Request(m_trigger_cmd)) {
            return scheduler::StepResult::fail(
                "ScanCodeSteps[" + label + "]: trigger command failed", -25);
        }

        // 轮询等待条码直至超时
        const auto deadline = std::chrono::steady_clock::now()
            + std::chrono::milliseconds(m_iMTimeout);

        for (;;) {
            {
                std::lock_guard<std::mutex> lock(capture->mtx);
                if (capture->received) {
                    if (IsFailedBarcode(capture->barcode, m_scan_failed_code)) {
                        return scheduler::StepResult::fail(
                            "ScanCodeSteps[" + label + "]: barcode matches scanFailedCode",
                            -27);
                    }
                    SetBoardSn(ctx, capture->barcode);
                    return scheduler::StepResult::ok();
                }
            }

            if (std::chrono::steady_clock::now() >= deadline) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(m_iMSpace));
        }

        return scheduler::StepResult::fail(
            "ScanCodeSteps[" + label + "]: barcode read timeout", -26);
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::ScanCodeSteps, "motion.scan_code")
