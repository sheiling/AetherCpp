// ============================================================
// basic_example.cpp — 调度引擎基础功能示例
//
// 演示内容：
//   1. 使用 FlowBuilder DSL 构建流程定义
//   2. 提交并运行流程实例
//   3. 流程状态监控与事件订阅
//   4. 补偿事务机制
//   5. 整机状态机控制（MachineFSM）
// ============================================================

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <windows.h> // 需要包含此头文件

#include "../step/flow_builder.h"
#include "../engine/scheduler.h"
#include "../event/event_bus.h"
#include "../alarm/alarm_manager.h"
#include "../machine/machine_fsm.h"
#include "../machine/machine_status.h"
#include "../core/flow_types.h"

using namespace scheduler;
using namespace std::chrono_literals;

// ── 辅助工具 ──────────────────────────────────────────────────
static void printSeparator(const std::string& title) {
    std::cout << "\n╔══════════════════════════════════════════════════╗\n";
    std::cout << "║  " << title << "\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n";
}

// ── 示例 1: 简单顺序流程 ─────────────────────────────────────
void example1_simple_sequential() {
    printSeparator("示例 1: 简单顺序流程");

    auto eventBus = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);

    // 订阅流程事件
    eventBus->subscribe(SchedulerEventType::StepStarted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 步骤开始: " << e.stepName << "\n";
    });
    eventBus->subscribe(SchedulerEventType::FlowCompleted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 流程完成: " << e.flowName << "\n";
    });

    // 使用 FlowBuilder 构建流程
    auto def = FlowBuilder("简单采样流程")
        .description("演示基本顺序步骤执行")
        .timeout(10000)
        .onStart([](FlowContext& ctx) {
            ctx.set("start_time", std::chrono::system_clock::now());
            std::cout << "  [回调] 流程启动: " << ctx.flowName << "\n";
        })
        .onComplete([](FlowContext& ctx) {
            std::cout << "  [回调] 流程完成: " << ctx.flowName << "\n";
        })
        .script("初始化", [](FlowContext& ctx) {
            ctx.set("sample_id", std::string("SN-20260101-001"));
            ctx.set("step_count", 0);
            std::cout << "    >> 初始化完成，样品ID: SN-20260101-001\n";
            return StepResult::ok();
        })
        .delay(200, "预热延时")
        .script("执行检测", [](FlowContext& ctx) {
            auto id = ctx.get<std::string>("sample_id");
            std::cout << "    >> 正在检测样品: " << id << " ...\n";
            std::this_thread::sleep_for(100ms);
            ctx.set("result", std::string("PASS"));
            return StepResult::ok();
        })
        .script("上报结果", [](FlowContext& ctx) {
            auto result = ctx.get<std::string>("result");
            std::cout << "    >> 检测结果: " << result << "\n";
            return StepResult::ok();
        })
        .build();

    scheduler->registerFlow(def);

    // 提交流程
    auto instanceId = scheduler->submit({"简单采样流程", 5, {{"lot", "Lot-2026"}}});
    std::cout << "  已提交流程，实例ID: " << instanceId << "\n";

    // 等待完成
    std::this_thread::sleep_for(1500ms);

    auto stats = scheduler->stats();
    std::cout << "  调度器统计: 运行中=" << stats.runningCount
              << " 已完成=" << stats.completedTotal << "\n";
}

// ── 示例 2: 并行步骤 ─────────────────────────────────────────
void example2_parallel_steps() {
    printSeparator("示例 2: 并行步骤");

    auto eventBus = std::make_shared<EventBus>();
    auto sched    = std::make_shared<Scheduler>(eventBus);

    auto def = FlowBuilder("并行检测流程")
        .script("准备工件", [](FlowContext&) {
            std::cout << "    >> 准备工件\n";
            return StepResult::ok();
        })
        .parallel("并行检测组")
            .script("视觉检测", [](FlowContext&) {
                std::cout << "    >> [视觉] 开始检测...\n";
                std::this_thread::sleep_for(200ms);
                std::cout << "    >> [视觉] 检测完成\n";
                return StepResult::ok();
            })
            .script("尺寸测量", [](FlowContext&) {
                std::cout << "    >> [尺寸] 开始测量...\n";
                std::this_thread::sleep_for(150ms);
                std::cout << "    >> [尺寸] 测量完成\n";
                return StepResult::ok();
            })
            .script("重量检测", [](FlowContext&) {
                std::cout << "    >> [重量] 开始称重...\n";
                std::this_thread::sleep_for(100ms);
                std::cout << "    >> [重量] 称重完成\n";
                return StepResult::ok();
            })
        .end()
        .script("汇总结果", [](FlowContext&) {
            std::cout << "    >> 所有检测完成，汇总结果\n";
            return StepResult::ok();
        })
        .build();

    sched->registerFlow(def);
    sched->submit({"并行检测流程"});

    std::this_thread::sleep_for(1000ms);
    std::cout << "  并行流程完成。\n";
}

// ── 示例 3: 条件分支与循环 ───────────────────────────────────
void example3_loop_and_branch() {
    printSeparator("示例 3: 循环与条件分支");

    auto eventBus = std::make_shared<EventBus>();
    auto sched    = std::make_shared<Scheduler>(eventBus);

    std::atomic<int> retryCount{0};

    auto def = FlowBuilder("重试检测流程")
        .script("初始化计数", [](FlowContext& ctx) {
            ctx.set("pass_count", 0);
            ctx.set("fail_count", 0);
            return StepResult::ok();
        })
        .loop(5, "循环检测5次")
            .script("单次检测", [&retryCount](FlowContext& ctx) {
                int iter = ctx.tryGet<int>("loop_iteration").value_or(0);
                std::cout << "    >> 第 " << (iter + 1) << " 次检测\n";
                // 模拟：偶数次通过，奇数次失败（但标记为可选）
                if (iter % 2 == 0) {
                    ctx.set("pass_count", ctx.get<int>("pass_count") + 1);
                    return StepResult::ok();
                } else {
                    ctx.set("fail_count", ctx.get<int>("fail_count") + 1);
                    ++retryCount;
                    return StepResult::ok(); // 演示：这里也返回ok
                }
            })
        .end()
        .script("统计结果", [](FlowContext& ctx) {
            std::cout << "    >> 通过次数: " << ctx.get<int>("pass_count")
                      << " 失败次数: " << ctx.get<int>("fail_count") << "\n";
            return StepResult::ok();
        })
        .build();

    sched->registerFlow(def);
    sched->submit({"重试检测流程"});

    std::this_thread::sleep_for(500ms);
}

// ── 示例 4: 补偿事务 ─────────────────────────────────────────
void example4_compensation() {
    printSeparator("示例 4: 补偿事务（中止时回滚）");

    auto eventBus = std::make_shared<EventBus>();
    auto sched    = std::make_shared<Scheduler>(eventBus);

    eventBus->subscribe(SchedulerEventType::FlowAborted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 流程已中止: " << e.flowName << "，补偿完成\n";
    });

    // 创建带补偿的步骤
    auto grabStep = std::make_shared<ScriptStep>([](FlowContext&) {
        std::cout << "    >> [动作] 机械臂取料\n";
        return StepResult::ok();
    });
    StepConfig cfg1{"取料", "control.script"};
    // 补偿：放回物料
    auto releaseStep = std::make_shared<ScriptStep>([](FlowContext&) {
        std::cout << "    >> [补偿] 机械臂放回物料\n";
        return StepResult::ok();
    });
    releaseStep->setConfig(StepConfig{"放回物料", "control.script"});
    cfg1.compensationStep = releaseStep;
    grabStep->setConfig(cfg1);

    auto placeStep = std::make_shared<ScriptStep>([](FlowContext&) {
        std::cout << "    >> [动作] 放置到检测台\n";
        return StepResult::ok();
    });
    StepConfig cfg2{"放置", "control.script"};
    auto removeStep = std::make_shared<ScriptStep>([](FlowContext&) {
        std::cout << "    >> [补偿] 从检测台移除\n";
        return StepResult::ok();
    });
    removeStep->setConfig(StepConfig{"从检测台移除", "control.script"});
    cfg2.compensationStep = removeStep;
    placeStep->setConfig(cfg2);

    auto failStep = std::make_shared<ScriptStep>([sched](FlowContext& ctx) {
        std::cout << "    >> [动作] 检测中... 发现异常，触发中止\n";
        // 关键：失败时主动触发 abort，使 FlowInstance 进入补偿分支
        sched->abort(ctx.instanceId);
        return StepResult::fail("检测失败，已触发 abort", -1);
    });
    failStep->setConfig(StepConfig{"检测", "control.script"});

    auto def = std::make_shared<FlowDefinition>();
    def->name              = "补偿演示流程";
    def->enableCompensation = true;
    def->onAbort = [](FlowContext& ctx) {
        std::cout << "  [回调] 流程中止，已执行补偿 (" << ctx.flowName << ")\n";
    };

    auto root = std::make_shared<SequentialStep>();
    root->setConfig(StepConfig{"root", "control.sequential"});
    root->addChild(grabStep);
    root->addChild(placeStep);
    root->addChild(failStep);
    def->rootStep = root;

    sched->registerFlow(def);
    sched->submit({"补偿演示流程"});

    std::this_thread::sleep_for(500ms);
}

// ── 示例 5: 报警与整机状态机 ────────────────────────────────
void example5_alarm_and_machine_fsm() {
    printSeparator("示例 5: 报警管理 + 整机状态机");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);
    auto alarmMgr  = std::make_shared<AlarmManager>(eventBus);
    auto machine   = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    // 注册报警规则
    alarmMgr->registerAlarm(1001, AlarmLevel::Warning, AlarmAction::Notify,
                             "真空压力偏低警告");
    alarmMgr->registerAlarm(1002, AlarmLevel::Fault, AlarmAction::PauseFlow,
                             "相机连接异常");
    alarmMgr->registerAlarm(1003, AlarmLevel::Stop, AlarmAction::StopAllFlows,
                             "运动轴超限报警");

    machine->onStatusChanged([](MachineStatus from, MachineStatus to) {
        std::cout << "  [状态机] " << statusToString(from)
                  << " → " << statusToString(to) << "\n";
    });

    std::cout << "  当前状态: " << statusToString(machine->currentStatus()) << "\n";

    machine->init();
    std::this_thread::sleep_for(50ms);
    std::cout << "  初始化后状态: " << statusToString(machine->currentStatus()) << "\n";

    machine->uiResetButton();
    std::this_thread::sleep_for(50ms);
    std::cout << "  复位后状态: " << statusToString(machine->currentStatus()) << "\n";

    machine->uiStartButton();
    std::this_thread::sleep_for(50ms);
    std::cout << "  启动后状态: " << statusToString(machine->currentStatus()) << "\n";

    // 触发 L1 警告（不影响运行）
    alarmMgr->raise(1001);
    std::cout << "  触发警告报警后，活动报警数: "
              << alarmMgr->activeAlarms().size() << "\n";

    // 触发 L3 停机报警
    alarmMgr->raise(1003);
    std::this_thread::sleep_for(50ms);
    std::cout << "  触发停机报警后状态: "
              << statusToString(machine->currentStatus()) << "\n";

    machine->uiClearAlarm();
    machine->uiResetButton();
    std::this_thread::sleep_for(50ms);
    std::cout << "  报警复位后状态: " << statusToString(machine->currentStatus()) << "\n";

    machine->uiStartButton();
    std::this_thread::sleep_for(50ms);

    machine->uiEstopButton();
    std::this_thread::sleep_for(50ms);
    std::cout << "  急停后状态: " << statusToString(machine->currentStatus()) << "\n";

    machine->sendEvent(MachineEvent::EstopRelease);
    machine->uiResetButton();
    std::this_thread::sleep_for(50ms);
    std::cout << "  急停复位后状态: " << statusToString(machine->currentStatus()) << "\n";
}

// ── 示例 6: 优先级调度与并发控制 ────────────────────────────
void example6_priority_scheduling() {
    printSeparator("示例 6: 优先级调度（并发上限 = 2）");

    auto eventBus = std::make_shared<EventBus>();
    auto sched    = std::make_shared<Scheduler>(eventBus);
    sched->setMaxConcurrent(2);

    // 注册一个通用流程
    auto makeFlowDef = [](const std::string& name, int durationMs) {
        return FlowBuilder(name)
            .script("工作", [durationMs, name](FlowContext&) {
                std::cout << "    >> [" << name << "] 开始执行 ("
                          << durationMs << "ms)\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
                std::cout << "    >> [" << name << "] 执行完成\n";
                return StepResult::ok();
            })
            .build();
    };

    sched->registerFlow(makeFlowDef("低优先级流程A", 300));
    sched->registerFlow(makeFlowDef("低优先级流程B", 300));
    sched->registerFlow(makeFlowDef("高优先级流程C", 200));
    sched->registerFlow(makeFlowDef("紧急流程D", 100));

    eventBus->subscribe(SchedulerEventType::FlowStarted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 开始运行: " << e.flowName << "\n";
    });

    // 同时提交多个请求（优先级不同）
    sched->submit({"低优先级流程A", 1});
    sched->submit({"低优先级流程B", 1});
    sched->submit({"高优先级流程C", 8});
    sched->submit({"紧急流程D", 9});

    auto stats = sched->stats();
    std::cout << "  提交后: 运行中=" << stats.runningCount
              << " 队列中=" << stats.queuedCount << "\n";

    std::this_thread::sleep_for(1500ms);

    stats = sched->stats();
    std::cout << "  完成后: 已完成=" << stats.completedTotal << "\n";
}

// ── main ──────────────────────────────────────────────────────
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "========================================\n";
    std::cout << "  调度引擎示例程序\n";
    std::cout << "  Automated Device Process Scheduler Demo\n";
    std::cout << "========================================\n";

    try {
        example1_simple_sequential();
        example2_parallel_steps();
        example3_loop_and_branch();
        example4_compensation();
        example5_alarm_and_machine_fsm();
        example6_priority_scheduling();
    } catch (const std::exception& e) {
        std::cerr << "\n[错误] " << e.what() << "\n";
        return 1;
    }

    std::cout << "\n========================================\n";
    std::cout << "  所有示例执行完成！\n";
    std::cout << "========================================\n";

    return 0;
}
