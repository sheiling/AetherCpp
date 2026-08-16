#include "flash_detect_steps.h"

#include <exception>

#include <common/core/container/application.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <common/core/messagecenter/message_center.h>
#include <common/utility/defer/include/Defer.h>

#include <scheduler/step/step_factory.h>

/**
 * @file flash_detect_steps.cpp
 * @brief FlashDetectSteps 的实现。
 *
 * 执行流程：清除 KEY_FLASH_DETECT_OUT → 读取 KEY_FLASH_STEP_CONFIG → 检查 enabled → 打开串口 → CheckFlash → 写入 KEY_FLASH_DETECT_OUT 并通知 EVENT_FLASH_DETECT_RESULT（bool）。
 * 串口在函数退出前通过 DEFER 保证关闭。
 */

namespace step {
namespace {

using json = nlohmann::json;
using common::algorithm::DetectResultStatus;
using common::algorithm::kLightenDetectProgressColorNone;
using common::algorithm::LightenDetectProgressKind;
using common::algorithm::LightenDetectProgressStatus;

// 清空本工步 execute 写入 FlowContext 的输出键，避免 Flow 复用时残留上次检测结果。
void ClearFlashDetectOutput(scheduler::FlowContext& ctx)
{
    ctx.remove(KEY_FLASH_DETECT_OUT, scheduler::VariableScope::FLOW);
}

} // namespace

// =============================================================================
// FlashDetectSteps 配置解析（reflectFromJson）
// reflectFromJson 解析 flashDetectKey 与协议地址；enabled 在 execute 入口从 KEY_FLASH_STEP_CONFIG 加载。
// =============================================================================

void FlashDetectSteps::reflectFromJson(const json& j)
{
    fromJson(j, "flashDetectKey", m_flash_detect_key);

    int dest_addr = static_cast<int>(m_dest_addr);
    int port_addr = static_cast<int>(m_port_addr);
    int board_addr = static_cast<int>(m_board_addr);

    fromJson(j, "destAddr", dest_addr);
    fromJson(j, "portAddr", port_addr);
    fromJson(j, "boardAddr", board_addr);

    // 协议地址超出合法范围时裁剪到边界，避免 uint8/uint16 溢出。
    if (dest_addr < 0) {
        dest_addr = 0;
    } else if (dest_addr > 255) {
        dest_addr = 255;
    }
    if (port_addr < 0) {
        port_addr = 0;
    } else if (port_addr > 255) {
        port_addr = 255;
    }
    if (board_addr < 0) {
        board_addr = 0;
    } else if (board_addr > 65535) {
        board_addr = 65535;
    }

    m_dest_addr = static_cast<uint8_t>(dest_addr);
    m_port_addr = static_cast<uint8_t>(port_addr);
    m_board_addr = static_cast<uint16_t>(board_addr);
}

// =============================================================================
// 主执行入口 execute
// 打开 Flash 检测串口 → CheckFlash → 将 DetectResultStatus 写入 KEY_FLASH_DETECT_OUT。
// =============================================================================

scheduler::StepResult FlashDetectSteps::execute(scheduler::FlowContext& ctx)
{
    try {
        ClearFlashDetectOutput(ctx);
        m_enabled = true;
        const auto cfg_opt = ctx.tryGet<json>(KEY_FLASH_STEP_CONFIG);
        if (cfg_opt && cfg_opt->is_object()) {
            fromJson(*cfg_opt, "enabled", m_enabled);
        }
        if (!m_enabled) {
            ctx.set(KEY_FLASH_DETECT_OUT,
                     static_cast<int>(DetectResultStatus::Ok),
                     scheduler::VariableScope::FLOW);
            return scheduler::StepResult::skip();
        }
        if (m_flash_detect_key.empty()) {
            return scheduler::StepResult::fail("FlashDetectSteps: flashDetectKey is empty", -20);
        }

        auto dm = CAPP->make<common::device::DeviceManager>();
        auto flash_detect = dm ? dm->FlashDetectHandler(m_flash_detect_key) : nullptr;
        if (!flash_detect) {
            return scheduler::StepResult::fail("FlashDetectSteps: flash detect handler not found", -21);
        }

        if (flash_detect->OpenComm() != 0) {
            return scheduler::StepResult::fail("FlashDetectSteps: open flash detect serial failed", -22);
        }
        // 无论检测成功与否，函数退出前关闭串口。
        DEFER([&]() { flash_detect->CloseComm(); });

        MessageCenter::Center::Notify(
            EVENT_LIGHTEN_DETECT_PROGRESS,
            LIGHTEN_DETECT_PROGRESS_NOTIFY_ARGS(
                LightenDetectProgressKind::Flash,
                kLightenDetectProgressColorNone,
                LightenDetectProgressStatus::InProgress));

        const int ret = flash_detect->CheckFlash(m_dest_addr, m_port_addr, m_board_addr);
        const DetectResultStatus flash_status =
            (ret == 0) ? DetectResultStatus::Ok : DetectResultStatus::Ng;
        ctx.set(KEY_FLASH_DETECT_OUT,
                  static_cast<int>(flash_status),
                  scheduler::VariableScope::FLOW);
        MessageCenter::Center::Notify(EVENT_FLASH_DETECT_RESULT, ret == 0);
        MessageCenter::Center::Notify(
            EVENT_LIGHTEN_DETECT_PROGRESS,
            LIGHTEN_DETECT_PROGRESS_NOTIFY_ARGS(
                LightenDetectProgressKind::Flash,
                kLightenDetectProgressColorNone,
                LightenDetectProgressStatus::Completed));

        if (ret != 0) {
            return scheduler::StepResult::fail("FlashDetectSteps: flash check failed", -23);
        }
        return scheduler::StepResult::ok();
    } catch (const std::exception& e) {
        return scheduler::StepResult::fail(e.what(), -99);
    }
}

} // namespace step

SCHEDULER_REGISTER_STEP_CLASS(step::FlashDetectSteps, "lighten.flash_detect")
