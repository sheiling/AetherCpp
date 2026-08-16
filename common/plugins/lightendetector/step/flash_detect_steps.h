#pragma once

/**
 * @file flash_detect_steps.h
 * @brief Flash 检测调度步骤（typeName: "lighten.flash_detect"）声明。
 *
 * 通过串口打开 Flash 检测设备，调用 CheckFlash 执行 Flash 校验，
 * 并将检测结果（0=OK，1=NG）写入 FlowContext 的 KEY_FLASH_DETECT_OUT。
 */

#include <string>
#include <common/3rdparty/nlohmann/json.hpp>
#include <common/include/algorithm_data/lightendetector_data.h>

#include <scheduler/step/i_step.h>

namespace step {

// 通过串口执行 Flash 检测的调度步骤（typeName: "lighten.flash_detect"）。
// 打开设备、调用 CheckFlash，并将检测结果（0=OK，1=NG）写入 KEY_FLASH_DETECT_OUT。
class FlashDetectSteps final : public scheduler::IStep {
public:
    void reflectFromJson(const nlohmann::json& j) override;
    scheduler::StepResult execute(scheduler::FlowContext& ctx) override;
    std::string typeName() const override { return "lighten.flash_detect"; }

private:
    // DeviceManager 中 Flash 检测设备的配置键（JSON: flashDetectKey）。
    std::string m_flash_detect_key;
    // Flash 检测协议目标地址（JSON: destAddr，范围 0-255）。
    uint8_t     m_dest_addr{ 1 };
    // Flash 检测协议端口地址（JSON: portAddr，范围 0-255）。
    uint8_t     m_port_addr{ 1 };
    // Flash 检测协议板卡地址（JSON: boardAddr，范围 0-65535）。
    uint16_t    m_board_addr{ 1 };
    // 是否启用本步骤；为 false 时直接 skip（JSON: enabled；由 execute 从 KEY_FLASH_STEP_CONFIG 加载）。
    bool        m_enabled{ true };
};

} // namespace step
