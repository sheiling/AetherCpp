// ============================================================
// station_example.cpp — 多工位协同示例
//
// 演示内容：
//   1. 多工位独立运行并通过屏障同步
//   2. StationManager 批量控制
//   3. 工位状态监控
// ============================================================

#include <iostream>
#include <thread>
#include <chrono>

#include "../station/station_manager.h"
#include "../step/flow_builder.h"
#include "../step/sync/sync_steps.h"
#include "../engine/scheduler.h"
#include "../event/event_bus.h"

using namespace scheduler;
using namespace std::chrono_literals;

int main() {
    std::cout << "========================================\n";
    std::cout << "  多工位协同示例\n";
    std::cout << "========================================\n\n";

    auto eventBus  = std::make_shared<EventBus>();
    auto sched     = std::make_shared<Scheduler>(eventBus);
    auto stationMgr = std::make_shared<StationManager>(sched, eventBus);

    sched->setMaxConcurrent(4);

    // ── 创建共享屏障（3个工位全部到达后才能继续）────────────
    auto barrier = stationMgr->createBarrier(
        "装配同步点",
        {"station-1", "station-2", "station-3"}
    );

    // ── 工位流程工厂（每个工位有不同的工作时间）──────────────
    auto makeStationFlow = [&](const std::string& stationId, int workMs) {
        auto barrierStep = std::make_shared<BarrierStep>(barrier, stationId);
        StepConfig bCfg{stationId + "_barrier", "sync.barrier"};
        bCfg.timeoutMs = 10000;
        barrierStep->setConfig(bCfg);

        auto def = std::make_shared<FlowDefinition>();
        def->name        = "工位流程_" + stationId;
        def->maxInstances = 1;

        auto root = std::make_shared<SequentialStep>();
        root->setConfig(StepConfig{def->name + "_root", "control.sequential"});

        // 准备步骤
        auto prepStep = std::make_shared<ScriptStep>([stationId, workMs](FlowContext&) {
            std::cout << "  [" << stationId << "] 准备中... (需要 "
                      << workMs << "ms)\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(workMs));
            std::cout << "  [" << stationId << "] 准备完成，等待同步\n";
            return StepResult::ok();
        });
        prepStep->setConfig(StepConfig{stationId + "_prep", "control.script"});

        // 同步后步骤
        auto afterStep = std::make_shared<ScriptStep>([stationId](FlowContext&) {
            std::cout << "  [" << stationId << "] 同步完成，执行装配\n";
            std::this_thread::sleep_for(50ms);
            std::cout << "  [" << stationId << "] 装配完成\n";
            return StepResult::ok();
        });
        afterStep->setConfig(StepConfig{stationId + "_after", "control.script"});

        root->addChild(prepStep);
        root->addChild(barrierStep);
        root->addChild(afterStep);
        def->rootStep = root;

        return def;
    };

    // 注册三个工位流程（不同准备时间）
    sched->registerFlow(makeStationFlow("station-1", 100));
    sched->registerFlow(makeStationFlow("station-2", 250));
    sched->registerFlow(makeStationFlow("station-3", 150));

    // 添加三个工位
    stationMgr->addStation({"station-1", "工位1", {}, "工位流程_station-1"});
    stationMgr->addStation({"station-2", "工位2", {}, "工位流程_station-2"});
    stationMgr->addStation({"station-3", "工位3", {}, "工位流程_station-3"});

    // 订阅事件
    eventBus->subscribe(SchedulerEventType::FlowCompleted, [](const SchedulerEvent& e) {
        std::cout << "  [事件] 流程完成: " << e.flowName << "\n";
    });

    std::cout << "启动所有工位...\n";
    auto t0 = std::chrono::steady_clock::now();
    stationMgr->startAll();

    // 等待所有工位完成
    std::this_thread::sleep_for(3000ms);

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();

    std::cout << "\n所有工位执行完成（总耗时: " << elapsed << "ms）\n";
    std::cout << "（注：屏障使最慢工位 station-2(250ms) 成为瓶颈）\n";

    // 输出工位状态
    std::cout << "\n当前工位状态:\n";
    for (auto& [id, state] : stationMgr->allStationStates()) {
        std::cout << "  " << id << ": " << flowStateToString(state) << "\n";
    }

    auto stats = sched->stats();
    std::cout << "\n调度器统计: 已完成=" << stats.completedTotal
              << " 失败=" << stats.failedTotal << "\n";

    return 0;
}
