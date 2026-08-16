// ============================================================
// step_flow_controller_example.cpp — 流程执行步骤控制器使用示例
//
// 演示：
//   1. 注册 control.script 脚本名 → C++ 回调（须在 FlowLoader 加载流程之前）
//   2. 从 step_flows_config.json 加载「步骤名 → 流程文件」映射
//   3. 按步骤名 runStep；叶子脚本内打印便于观察每一步（嵌套步骤不会单独触发
//      FlowInstance 的 StepStarted 事件，仅顶层子步骤会触发）
// ============================================================

#include <algorithm>
#include <iostream>
#include <filesystem>

#include <scheduler/step/step_flow_controller.h>
#include "../engine/flow_loader.h"
#include "../event/event_bus.h"
#include "../core/flow_types.h"

#ifndef SCHEDULER_STEP_FLOW_EXAMPLES_DIR
#define SCHEDULER_STEP_FLOW_EXAMPLES_DIR "."
#endif

using namespace scheduler;

namespace {

std::string batchTag(FlowContext& ctx) {
    return ctx.initParams.value("batchId", "?");
}

void registerStepctrlDemoScripts() {
    auto& R = ScriptRegistry::instance();

    R.registerScript("stepctrl_wu_power_stable", [](FlowContext& ctx) {
        std::cout << "    [脚本] 串行_上电稳定 | batch=" << batchTag(ctx)
                  << " | 检查供电、急停回路、气压阈值\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_wu_ch_a_check", [](FlowContext& ctx) {
        std::cout << "    [脚本] 并行_通道A | batch=" << batchTag(ctx)
                  << " | 相机/IO 自检\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_wu_ch_b_check", [](FlowContext& ctx) {
        std::cout << "    [脚本] 并行_通道B | batch=" << batchTag(ctx)
                  << " | 运动轴回零与限位\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_wu_ch_c_check", [](FlowContext& ctx) {
        std::cout << "    [脚本] 并行_通道C | batch=" << batchTag(ctx)
                  << " | 传感器采样校验\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_wu_tail_prepare", [](FlowContext& ctx) {
        std::cout << "    [脚本] 收尾_步骤1 | batch=" << batchTag(ctx)
                  << " | 写入预热完成标志\n";
        ctx.set("warmup_phase", std::string("prepare_done"), VariableScope::FLOW);
        return StepResult::ok();
    });
    R.registerScript("stepctrl_wu_tail_confirm", [](FlowContext& ctx) {
        std::cout << "    [脚本] 收尾_步骤2 | batch=" << batchTag(ctx)
                  << " | 确认可进入加工 (warmup_phase="
                  << ctx.tryGet<std::string>("warmup_phase").value_or("?") << ")\n";
        return StepResult::ok();
    });

    R.registerScript("stepctrl_fn_stop_infeed", [](FlowContext& ctx) {
        std::cout << "    [脚本] 串行_停止进料 | batch=" << batchTag(ctx)
                  << " | 关闭进料阀、通知上位\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_fn_drain_station1", [](FlowContext& ctx) {
        std::cout << "    [脚本] 排空_工位1 | batch=" << batchTag(ctx)
                  << " | 工位1 泄压与残料排出\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_fn_drain_station2", [](FlowContext& ctx) {
        std::cout << "    [脚本] 排空_工位2 | batch=" << batchTag(ctx)
                  << " | 工位2 泄压与残料排出\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_fn_full_stop", [](FlowContext& ctx) {
        std::cout << "    [脚本] 串行_完全停机 | batch=" << batchTag(ctx)
                  << " | 主轴/伺服下使能、切断动力\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_fn_led_green_reset", [](FlowContext& ctx) {
        std::cout << "    [脚本] 并行_绿灯 | batch=" << batchTag(ctx)
                  << " | 复位运行灯状态\n";
        return StepResult::ok();
    });
    R.registerScript("stepctrl_fn_led_amber_reset", [](FlowContext& ctx) {
        std::cout << "    [脚本] 并行_黄灯 | batch=" << batchTag(ctx)
                  << " | 复位告警灯状态\n";
        return StepResult::ok();
    });
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    registerStepctrlDemoScripts();

    const fs::path configPath =
        fs::path(SCHEDULER_STEP_FLOW_EXAMPLES_DIR) / "step_flows_config.json";

    std::cout << "[示例] 步骤控制器配置: " << configPath.string() << "\n";

    StepFlowController controller;
    try {
        controller.loadFromFile(configPath.string());
    } catch (const std::exception& ex) {
        std::cerr << "[示例] 加载失败: " << ex.what() << "\n";
        return 1;
    }

    auto names = controller.stepNames();
    std::sort(names.begin(), names.end());
    std::cout << "[示例] 已注册步骤名 (" << names.size() << "): ";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << names[i];
    }
    std::cout << "\n";

    auto bus = std::make_shared<EventBus>();
    bus->subscribe(SchedulerEventType::FlowStarted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 流程开始: " << e.flowName << " instance=" << e.instanceId << "\n";
    });
    bus->subscribe(SchedulerEventType::FlowCompleted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 流程完成: " << e.flowName << "\n";
    });
    // 仅顶层「根顺序」下的直接子步骤会收到 Step* 事件；嵌套叶子请看上方 [脚本] 行
    bus->subscribe(SchedulerEventType::StepStarted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 顶层步骤开始: " << e.stepName << " (flow=" << e.flowName << ")\n";
    });
    bus->subscribe(SchedulerEventType::StepCompleted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 顶层步骤完成: " << e.stepName << "\n";
    });
    bus->subscribe(SchedulerEventType::StepFailed, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 顶层步骤失败: " << e.stepName << "\n";
    });

    json init;
    init["batchId"] = "DEMO-001";

    for (const auto& stepName : names) {
        std::cout << "\n[示例] 执行步骤名: \"" << stepName << "\"\n";
        StepResult r = controller.runStep(stepName, init, bus);
        if (r.isOk()) {
            std::cout << "  >> 结果: 成功\n";
        } else {
            std::cout << "  >> 结果: 失败 — " << r.message << " (code=" << r.errorCode << ")\n";
            return 2;
        }
    }

    std::cout << "\n[示例] 全部步骤执行完毕。\n";
    return 0;
}
