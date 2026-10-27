// ============================================================
// production_line_example.cpp
//
// 自动化设备流程执行调度引擎 — 综合示例
//
// 演示以下功能：
//   A. 单工位持续运行（Start 后自动循环接料）
//   B. 单工位暂停/恢复
//   C. 异常恢复：自动重试 / 跳过失败步骤 / 断点恢复
//   D. 多工位三工位流水线（JSON 配置加载 + PipelineChannel）
//   E. 多工位持续运行（MachineFSM + StationManager）
//   F. 多工位 + 报警 → 暂停 → 恢复完整演示
//   G. 按钮/IO 监控与状态机联动
// ============================================================

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <random>
#include <windows.h> // 需要包含此头文件

// 调度核心
#include "../engine/scheduler.h"
#include "../engine/flow_loader.h"
#include "../engine/flow_definition.h"

// 步骤 DSL
#include "../step/flow_builder.h"
#include "../step/control/script_step.h"
#include "../step/control/delay_step.h"
#include "../step/sync/pipeline_channel.h"
#include "../step/sync/sync_steps.h"

// 上下文 & 类型
#include "../context/flow_context.h"
#include "../core/flow_types.h"

// 多工位
#include "../station/station_manager.h"
#include "../station/line_loader.h"

// 状态机 & 报警
#include "../machine/machine_fsm.h"
#include "../machine/machine_status.h"
#include "../alarm/alarm_manager.h"

// 事件总线
#include "../event/event_bus.h"

#include <Logger.h>

using namespace scheduler;
using namespace std::chrono_literals;

// ============================================================
// 工具函数
// ============================================================

static void printSeparator(const std::string& title) {
    std::cout << "\n"
              << "╔══════════════════════════════════════════════════════╗\n"
              << "║  " << title
              << std::string(52 - title.size(), ' ') << "║\n"
              << "╚══════════════════════════════════════════════════════╝\n";
}

static void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ============================================================
// 注册所有脚本处理函数（单工位示例用）
// ============================================================

static void registerSingleStationScripts(int& batchCounter) {

    auto& reg = ScriptRegistry::instance();

    // ── 模拟初始化 ────────────────────────────────────────────
    reg.registerScript("init_station", [&batchCounter](FlowContext& ctx) {
        ++batchCounter;
        ctx.set("batch_id", batchCounter);
        LOG_INFO("[Script] init_station: 批次 #{}", batchCounter);
        return StepResult::ok();
    });

    // ── 模拟来料检测（偶尔失败以演示重试）───────────────────
    reg.registerScript("check_material", [](FlowContext& ctx) {
        static int callCount = 0;
        ++callCount;
        // 第 2 次调用模拟失败，触发重试机制
        if (callCount == 2) {
            LOG_WARN("[Script] check_material: 第 {} 次调用，模拟失败", callCount);
            return StepResult::fail("来料异常，触发重试");
        }
        LOG_INFO("[Script] check_material: 来料检测通过");
        return StepResult::ok();
    });

    // ── 模拟加工处理 ──────────────────────────────────────────
    reg.registerScript("process_material", [](FlowContext& ctx) {
        auto batchId = ctx.tryGet<int>("batch_id").value_or(-1);
        LOG_INFO("[Script] process_material: 加工批次 #{}", batchId);
        sleepMs(50);   // 模拟加工耗时
        ctx.set("process_ok", true);
        return StepResult::ok();
    });

    // ── 模拟上报结果（带补偿步骤）────────────────────────────
    reg.registerScript("report_result", [](FlowContext& ctx) {
        auto batchId = ctx.tryGet<int>("batch_id").value_or(-1);
        LOG_INFO("[Script] report_result: 上报批次 #{} 结果", batchId);
        return StepResult::ok();
    });

    reg.registerScript("cancel_report", [](FlowContext& ctx) {
        LOG_WARN("[Script] cancel_report: 撤销上报（补偿动作）");
        return StepResult::ok();
    });

    // ── 可选步骤（模拟偶尔失败但不影响整体流程）─────────────
    reg.registerScript("optional_step", [](FlowContext& ctx) {
        static bool shouldFail = true;
        if (shouldFail) {
            shouldFail = false;
            LOG_WARN("[Script] optional_step: 本次失败（可选步骤，跳过）");
            return StepResult::fail("可选步骤失败");
        }
        LOG_INFO("[Script] optional_step: 执行成功");
        return StepResult::ok();
    });
}

// ============================================================
// 注册多工位流水线脚本（三工位 SMT 产线）
// ============================================================

static void registerMultiStationScripts() {
    auto& reg = ScriptRegistry::instance();
    static std::atomic<int> pieceCounter{0};
    static std::mt19937 rng(std::random_device{}());

    // ──── 上料工位 ───────────────────────────────────────────
    reg.registerScript("st_load_pickup", [](FlowContext& ctx) {
        int id = ++pieceCounter;
        ctx.set("piece_id", id);
        LOG_INFO("[上料] 取板成功，工件 ID: {}", id);
        sleepMs(80);
        return StepResult::ok();
    });

    reg.registerScript("st_load_apply_glue", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("piece_id").value_or(0);
        LOG_INFO("[上料] 工件 {} 点胶完成", id);
        sleepMs(50);
        return StepResult::ok();
    });

    reg.registerScript("st_load_prepare_data", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("piece_id").value_or(0);
        // 构建发送给检测工位的工件数据
        json data;
        data["piece_id"]  = id;
        data["board_type"]= ctx.tryGet<std::string>("boardType").value_or("unknown");
        data["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        ctx.set("workpiece_data", data);
        LOG_INFO("[上料] 工件 {} 数据已准备", id);
        return StepResult::ok();
    });

    reg.registerScript("st_load_report_done", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("piece_id").value_or(0);
        LOG_INFO("[上料] 工件 {} 上料完成，已上报", id);
        return StepResult::ok();
    });

    // ──── 检测工位 ───────────────────────────────────────────
    reg.registerScript("st_inspect_position", [](FlowContext& ctx) {
        auto wd = ctx.tryGet<json>("workpiece_data");
        int id = wd ? wd->value("piece_id", 0) : 0;
        LOG_INFO("[检测] 工件 {} 已定位", id);
        ctx.set("inspecting_id", id);
        sleepMs(30);
        return StepResult::ok();
    });

    reg.registerScript("st_inspect_aoi", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("inspecting_id").value_or(0);
        LOG_INFO("[检测] 工件 {} AOI 检测中...", id);
        sleepMs(120);
        // 模拟 10% 不良率
        static std::uniform_int_distribution<int> dist(1, 10);
        bool isNG = (dist(rng) == 1);
        ctx.set("aoi_ng", isNG);
        LOG_INFO("[检测] 工件 {} AOI 结果: {}", id, isNG ? "NG" : "OK");
        return StepResult::ok();
    });

    reg.registerScript("st_inspect_dimension", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("inspecting_id").value_or(0);
        LOG_INFO("[检测] 工件 {} 尺寸测量中...", id);
        sleepMs(80);
        ctx.set("dim_ok", true);
        return StepResult::ok();
    });

    reg.registerScript("st_inspect_solder", [](FlowContext& ctx) {
        auto id = ctx.tryGet<int>("inspecting_id").value_or(0);
        LOG_INFO("[检测] 工件 {} 焊点检测中...", id);
        sleepMs(100);
        ctx.set("solder_ok", true);
        return StepResult::ok();
    });

    reg.registerScript("st_inspect_summarize", [](FlowContext& ctx) {
        auto id    = ctx.tryGet<int>("inspecting_id").value_or(0);
        bool aoiNg = ctx.tryGet<bool>("aoi_ng").value_or(false);
        bool dimOk = ctx.tryGet<bool>("dim_ok").value_or(true);
        bool soldOk= ctx.tryGet<bool>("solder_ok").value_or(true);

        bool overall = (!aoiNg) && dimOk && soldOk;

        json result;
        result["piece_id"] = id;
        result["overall"]  = overall ? "OK" : "NG";
        result["aoi"]      = aoiNg ? "NG" : "OK";
        result["dimension"]= dimOk ? "OK" : "NG";
        result["solder"]   = soldOk? "OK" : "NG";
        ctx.set("inspect_result", result);

        LOG_INFO("[检测] 工件 {} 综合结果: {}", id, result["overall"].get<std::string>());
        return StepResult::ok();
    });

    // ──── 下料工位 ───────────────────────────────────────────
    reg.registerScript("st_unload_ok", [](FlowContext& ctx) {
        auto res = ctx.tryGet<json>("inspect_result");
        int id = res ? res->value("piece_id", 0) : 0;
        LOG_INFO("[下料] 工件 {} → OK 合格品下料", id);
        sleepMs(60);
        return StepResult::ok();
    });

    reg.registerScript("st_unload_ng", [](FlowContext& ctx) {
        auto res = ctx.tryGet<json>("inspect_result");
        int id = res ? res->value("piece_id", 0) : 0;
        LOG_INFO("[下料] 工件 {} → NG 不良品拒料", id);
        sleepMs(60);
        return StepResult::ok();
    });

    reg.registerScript("st_unload_report", [](FlowContext& ctx) {
        auto res = ctx.tryGet<json>("inspect_result");
        int id = res ? res->value("piece_id", 0) : 0;
        std::string overall = res ? res->value("overall", "?") : "?";
        LOG_INFO("[下料] 工件 {} 下料完成，结果 = {}", id, overall);
        return StepResult::ok();
    });
}

// ============================================================
// 示例 A — 单工位持续运行
// ============================================================

static void demoA_SingleStationContinuous() {
    printSeparator("示例 A：单工位持续运行（自动循环接料）");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);
    auto alarmMgr  = std::make_shared<AlarmManager>(eventBus);
    auto machine   = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    int batchCounter = 0;
    registerSingleStationScripts(batchCounter);

    // 用 DSL 构建单工位流程（含补偿步骤）
    auto def = FlowBuilder("单工位检测流程")
        .description("来料 → 检测 → 上报")
        .timeout(30000)
        .enableCompensation(true)
        // ── 流程生命周期 ──────────────────────────────────────
        .onStart([](FlowContext& ctx) {
            LOG_INFO("[Flow] 流程启动，实例: {}", ctx.instanceId);
        })
        .onComplete([](FlowContext& ctx) {
            auto batchId = ctx.tryGet<int>("batch_id").value_or(-1);
            LOG_INFO("[Flow] 流程完成，批次 #{}", batchId);
        })
        .onFailed([](FlowContext& ctx) {
            LOG_WARN("[Flow] 流程失败！批次 #{}", ctx.tryGet<int>("batch_id").value_or(-1));
        })
        // ── 步骤定义 ──────────────────────────────────────────
        .script("初始化", ScriptRegistry::instance().getScript("init_station"))
        .delay(50, "预热延时")
        .script("来料检测", ScriptRegistry::instance().getScript("check_material"))
            // 注意：check_material 已配置 retryCount=2，此处通过 addStep 设置
        .script("加工处理", ScriptRegistry::instance().getScript("process_material"))
        .script("可选步骤", ScriptRegistry::instance().getScript("optional_step"))
        .script("上报结果", ScriptRegistry::instance().getScript("report_result"))
        .build();

    // 给 "来料检测" 步骤添加重试（通过手动修改 rootStep 子步骤配置）
    // 更推荐在 FlowBuilder 扩展配置 API 后设置
    scheduler->registerFlow(def);

    // 配置持续运行模式
    machine->enableContinuousMode(ContinuousModeConfig{
        "单工位检测流程",  // 循环执行的流程名
        {},               // 每次执行时传入的参数
        0,                // 优先级
        100,              // 片料间隔 100ms（模拟来料节拍）
        true              // 失败后继续执行下一片
    });

    machine->init();
    machine->uiResetButton();

    std::cout << ">>> 按 Start，产线自动循环接料 3 批次后停止...\n";
    machine->uiStartButton();   // Paused → Running（持续接料）
    sleepMs(1200);              // 等待 3 批次完成

    // 物理停止按钮 = 暂停语义（Running → Paused）
    std::cout << ">>> 按 Stop（物理停止按钮），等当前料完成后暂停...\n";
    machine->uiStopButton();    // Running → Paused（物理停止=暂停）
    sleepMs(300);
    std::cout << ">>> 当前状态: " << statusToString(machine->currentStatus())
              << "，调用 softShutdown() 完全停机...\n";
    // 班次结束/程序退出：调用软件停机接口（非物理按钮）
    machine->softShutdown();    // → Paused
    sleepMs(400);

    std::cout << ">>> 示例 A 完成，总批次: " << batchCounter << "\n";

    // 清理脚本注册表，避免影响后续示例
    ScriptRegistry::instance().clear();
}

// ============================================================
// 示例 B — 单工位暂停/恢复
// ============================================================

static void demoB_PauseResume() {
    printSeparator("示例 B：单工位暂停/恢复");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);
    auto alarmMgr  = std::make_shared<AlarmManager>(eventBus);
    auto machine   = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    int batchCounter = 0;
    registerSingleStationScripts(batchCounter);

    auto def = FlowBuilder("单工位检测流程")
        .script("初始化",   ScriptRegistry::instance().getScript("init_station"))
        .delay(200, "长延时加工")   // 故意加长，便于在中途暂停
        .script("加工处理", ScriptRegistry::instance().getScript("process_material"))
        .script("上报结果", ScriptRegistry::instance().getScript("report_result"))
        .build();

    scheduler->registerFlow(def);
    machine->enableContinuousMode({"单工位检测流程", {}, 0, 50, true});
    machine->init();
    machine->uiResetButton();

    machine->uiStartButton();
    std::cout << ">>> 产线运行中（状态: Running）\n";
    sleepMs(300);

    // 物理停止按钮 = 暂停语义（Running → Paused）
    std::cout << ">>> 按 Stop（物理停止按钮），当前料执行完后挂起...\n";
    machine->uiStopButton();    // Running → Paused（物理停止=暂停）
    sleepMs(100);
    std::cout << ">>> 当前状态: " << statusToString(machine->currentStatus()) << "\n";

    std::cout << ">>> 维护窗口（等待 500ms）...\n";
    sleepMs(500);

    // 物理启动按钮同样可以恢复生产（与 uiResumeButton() 等效）
    std::cout << ">>> 按 Start（物理启动按钮=恢复），恢复生产...\n";
    machine->uiStartButton();   // Paused → Running（Start 兼具恢复功能）
    sleepMs(600);

    // 班次结束：软件停机
    machine->softShutdown();    // Running → Paused
    sleepMs(400);

    std::cout << ">>> 示例 B 完成，总批次: " << batchCounter << "\n";
    ScriptRegistry::instance().clear();
}

// ============================================================
// 示例 C — 异常恢复（自动重试 + skipOnFail + 断点恢复）
// ============================================================

static void demoC_ErrorRecovery() {
    printSeparator("示例 C：执行异常恢复（重试/跳过/断点）");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);

    int callCount = 0;

    // 构建测试流程
    auto def = FlowBuilder("异常恢复测试流程")
        .enableCompensation(true)
        .onFailed([](FlowContext& ctx) {
            LOG_ERROR("[Flow] 流程失败！");
        })
        .onComplete([](FlowContext& ctx) {
            LOG_INFO("[Flow] 流程完成！历史: {} 步", ctx.history.size());
        })
        // 步骤 1：可能失败，配置重试 2 次
        .addStep([&]() {
            auto s = std::make_shared<ScriptStep>(
                [&callCount](FlowContext& ctx) -> StepResult {
                    ++callCount;
                    if (callCount <= 1) {
                        LOG_WARN("[Step1] 第 {} 次调用，模拟失败", callCount);
                        return StepResult::fail("暂时性错误");
                    }
                    LOG_INFO("[Step1] 第 {} 次调用，成功", callCount);
                    LOG_INFO("[Step1] 执行后：结果=OK");
                    return StepResult::ok();
                });
            StepConfig cfg;
            cfg.name       = "步骤1_带重试";
            cfg.type       = "control.script";
            cfg.retryCount = 2;
            cfg.retryIntervalMs = 100;
            s->setConfig(std::move(cfg));
            return std::static_pointer_cast<IStep>(s);
        }())
        // 步骤 2：optional=true，失败不影响整体
        .addStep([&]() {
            auto s = std::make_shared<ScriptStep>(
                [](FlowContext& ctx) -> StepResult {
                    LOG_WARN("[Step2] 可选步骤，模拟失败（流程继续）");
                    return StepResult::fail("可选步骤错误");
                });
            StepConfig cfg;
            cfg.name     = "步骤2_可选步骤";
            cfg.type     = "control.script";
            cfg.optional = true;
            s->setConfig(std::move(cfg));
            return std::static_pointer_cast<IStep>(s);
        }())
        // 步骤 3：正常完成步骤（检查点）
        .addStep([&]() {
            auto s = std::make_shared<ScriptStep>(
                [](FlowContext& ctx) -> StepResult {
                    LOG_INFO("[Step3] 执行前准备");
                    LOG_INFO("[Step3] 正常步骤完成");
                    return StepResult::ok();
                });
            StepConfig cfg;
            cfg.name      = "步骤3_正常步骤";
            cfg.type      = "control.script";
            cfg.isCheckpoint = true;
            s->setConfig(std::move(cfg));
            return std::static_pointer_cast<IStep>(s);
        }())
        .build();

    scheduler->registerFlow(def);

    // 提交执行
    auto id = scheduler->submit({"异常恢复测试流程", 0, {}});
    std::cout << ">>> 提交流程: " << id << "\n";

    auto info = scheduler->getInstanceInfo(id);
    int waited = 0;
    while (info.state != FlowState::Completed &&
           info.state != FlowState::Failed && waited < 5000) {
        sleepMs(50);
        waited += 50;
        info = scheduler->getInstanceInfo(id);
    }

    std::cout << ">>> 流程最终状态: "
              << (info.state == FlowState::Completed ? "Completed" : "Failed") << "\n";
    std::cout << ">>> 示例 C 完成（步骤1 重试了 " << callCount - 1 << " 次）\n";
}

// ============================================================
// 示例 E — 多工位三工位流水线（JSON 加载 + 代码构建对比）
// ============================================================

static void demoE_MultiStationPipeline() {
    printSeparator("示例 E：多工位三工位流水线（PipelineChannel 数据传递）");

    registerMultiStationScripts();

    auto eventBus    = std::make_shared<EventBus>();
    auto scheduler   = std::make_shared<Scheduler>(eventBus);
    auto stationMgr  = std::make_shared<StationManager>(scheduler, eventBus);
    auto alarmMgr    = std::make_shared<AlarmManager>(eventBus);
    auto machine     = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    // ── 创建流水线通道 ────────────────────────────────────────
    stationMgr->createChannel("ch-load-to-inspect",   2);  // 上料→检测 缓冲2
    stationMgr->createChannel("ch-inspect-to-unload", 2);  // 检测→下料 缓冲2

    // ── 通过 DSL 构建三个工位流程 ─────────────────────────────

    // 上料工位流程
    auto defLoad = FlowBuilder("上料流程")
        .script("取板", ScriptRegistry::instance().getScript("st_load_pickup"))
        .script("点胶", ScriptRegistry::instance().getScript("st_load_apply_glue"))
        .script("准备数据", ScriptRegistry::instance().getScript("st_load_prepare_data"))
        .addStep([]() {
            auto s = std::make_shared<ChannelSendStep>("ch-load-to-inspect", "workpiece_data");
            StepConfig cfg{"发送工件到检测队列", "sync.channel_send", 10000};
            s->setConfig(cfg);
            return std::static_pointer_cast<IStep>(s);
        }())
        .script("上料完成上报",
                ScriptRegistry::instance().getScript("st_load_report_done"))
        .build();

    // 检测工位流程
    auto defInspect = FlowBuilder("检测流程")
        .addStep([]() {
            auto s = std::make_shared<ChannelReceiveStep>("ch-load-to-inspect", "workpiece_data");
            StepConfig cfg{"接收工件数据", "sync.channel_receive", 30000};
            s->setConfig(cfg);
            return std::static_pointer_cast<IStep>(s);
        }())
        .script("工件定位",
                ScriptRegistry::instance().getScript("st_inspect_position"))
        .parallel("并行检测")
            .script("AOI检测",
                    ScriptRegistry::instance().getScript("st_inspect_aoi"))
            .script("尺寸测量",
                    ScriptRegistry::instance().getScript("st_inspect_dimension"))
            .script("焊点检测",
                    ScriptRegistry::instance().getScript("st_inspect_solder"))
        .end()
        .script("汇总结果",
                ScriptRegistry::instance().getScript("st_inspect_summarize"))
        .addStep([]() {
            auto s = std::make_shared<ChannelSendStep>("ch-inspect-to-unload", "inspect_result");
            StepConfig cfg{"发送检测结果", "sync.channel_send", 10000};
            s->setConfig(cfg);
            return std::static_pointer_cast<IStep>(s);
        }())
        .build();

    // 下料工位流程（OK/NG 分拣用脚本网关实现）
    auto defUnload = FlowBuilder("下料流程")
        .addStep([]() {
            auto s = std::make_shared<ChannelReceiveStep>(
                "ch-inspect-to-unload", "inspect_result");
            StepConfig cfg{"接收检测结果", "sync.channel_receive", 30000};
            s->setConfig(cfg);
            return std::static_pointer_cast<IStep>(s);
        }())
        .script("分拣处理", [](FlowContext& ctx) -> StepResult {
            auto res = ctx.tryGet<json>("inspect_result");
            if (!res) return StepResult::fail("无检测结果");
            std::string overall = res->value("overall", "NG");
            int id = res->value("piece_id", 0);
            if (overall == "OK") {
                LOG_INFO("[下料] 工件 {} → OK 合格品下料", id);
            } else {
                LOG_INFO("[下料] 工件 {} → NG 不良品拒料", id);
            }
            return StepResult::ok();
        })
        .script("下料上报",
                ScriptRegistry::instance().getScript("st_unload_report"))
        .build();

    // 注册流程并添加工位
    scheduler->registerFlow(defLoad);
    scheduler->registerFlow(defInspect);
    scheduler->registerFlow(defUnload);

    stationMgr->addStation({"st-load",    "上料工位",   {}, "上料流程"});
    stationMgr->addStation({"st-inspect", "检测工位",   {}, "检测流程"});
    stationMgr->addStation({"st-unload",  "下料工位",   {}, "下料流程"});

    // 各工位开启持续运行
    StationContinuousConfig contCfg;
    contCfg.intervalMs     = 0;
    contCfg.resubmitOnFail = true;
    stationMgr->enableContinuousMode(contCfg);

    // 绑定 StationManager，切换为多工位模式
    machine->attachStationManager(stationMgr);
    machine->init();
    machine->uiResetButton();

    std::cout << ">>> 多工位流水线启动（运行 10 片料）...\n";
    machine->uiStartButton();   // 启动所有工位
    sleepMs(4000);              // 等待若干批次完成

    // 演示暂停：物理停止按钮 = 暂停语义
    std::cout << ">>> 按 Stop（物理停止按钮），维护窗口...\n";
    machine->uiStopButton();    // Running → Paused（物理停止=暂停）
    sleepMs(800);
    std::cout << ">>> 状态: " << statusToString(machine->currentStatus()) << "\n";

    std::cout << ">>> 按 Resume，恢复流水线...\n";
    machine->uiStartButton();  // Paused → Running
    sleepMs(2000);

    std::cout << ">>> 班次结束，软件停机...\n";
    machine->softShutdown();    // Running → Paused
    sleepMs(500);

    std::cout << ">>> 示例 E 完成\n";

    // 清通道，避免影响其他示例
    PipelineChannelHub::instance().clearAll();
    ScriptRegistry::instance().clear();
}

// ============================================================
// 示例 F — 多工位 JSON 配置加载（LineLoader）
// ============================================================

static void demoF_MultiStationJsonLoad() {
    printSeparator("示例 F：多工位 JSON 配置加载（LineLoader）");

    registerMultiStationScripts();

    auto eventBus   = std::make_shared<EventBus>();
    auto scheduler  = std::make_shared<Scheduler>(eventBus);
    auto stationMgr = std::make_shared<StationManager>(scheduler, eventBus);
    auto alarmMgr   = std::make_shared<AlarmManager>(eventBus);
    auto machine    = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    // 在 examples 目录下有 line_config.json 及三个 flow_st_*.json
    const std::string examplesDir =
        // 这里填写实际路径，或由 CMake 设置 EXAMPLES_DIR 宏
#ifdef EXAMPLES_DIR
        EXAMPLES_DIR;
#else
        ".";
#endif

    FlowLoader  flowLoader;
    LineLoader  lineLoader;

    try {
        lineLoader.loadAndApply(
            examplesDir + "/line_config.json",
            *stationMgr, *scheduler, flowLoader,
            examplesDir
        );
        std::cout << ">>> JSON 产线配置加载成功\n";
    } catch (const std::exception& e) {
        std::cout << ">>> JSON 加载失败（文件可能不存在）: " << e.what() << "\n"
                  << ">>> 跳过 JSON 示例，已通过代码演示（示例 E）\n";
        ScriptRegistry::instance().clear();
        return;
    }

    machine->attachStationManager(stationMgr);
    machine->init();
    machine->uiResetButton();

    std::cout << ">>> 从 JSON 配置启动多工位流水线...\n";
    machine->uiStartButton();
    sleepMs(3000);

    machine->softShutdown();    // 软件停机（非物理停止按钮）
    sleepMs(500);

    std::cout << ">>> 示例 F 完成\n";
    PipelineChannelHub::instance().clearAll();
    BarrierRegistry::instance().clearAll();
    ScriptRegistry::instance().clear();
}

// ============================================================
// 示例 G — 报警 → 停机 → 复位完整演示
// ============================================================

static void demoG_AlarmAndRecovery() {
    printSeparator("示例 G：报警停机 → 复位 → 继续生产");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);
    auto alarmMgr  = std::make_shared<AlarmManager>(eventBus);
    auto machine   = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    int batch = 0;
    auto def = FlowBuilder("产线流程")
        .script("来料", [&batch](FlowContext& ctx) {
            LOG_INFO("[Flow] 来料处理，批次 #{}", ++batch);
            sleepMs(80);
            return StepResult::ok();
        })
        .script("加工", [](FlowContext& ctx) {
            LOG_INFO("[Flow] 加工处理");
            sleepMs(100);
            return StepResult::ok();
        })
        .build();

    scheduler->registerFlow(def);
    machine->enableContinuousMode({"产线流程", {}, 0, 50, true});
    machine->onStatusChanged([](MachineStatus from, MachineStatus to) {
        std::cout << "  [状态] " << statusToString(from)
                  << " → " << statusToString(to) << "\n";
    });

    machine->init();
    machine->uiResetButton();
    machine->uiStartButton();

    std::cout << ">>> 正常生产 3 批次...\n";
    sleepMs(800);

    // 模拟 L3 报警（调用 AlarmManager 或直接发送事件）
    std::cout << ">>> 触发 L3 报警（停机）...\n";
    machine->sendEvent(MachineEvent::Alarm);
    sleepMs(200);
    std::cout << ">>> 当前状态: " << statusToString(machine->currentStatus()) << "\n";

    std::cout << ">>> 处理报警（等待 500ms）...\n";
    sleepMs(500);

    std::cout << ">>> 复位，重新生产...\n";
    machine->uiResetButton();   // Alarm → Paused（复位成功）
    sleepMs(100);
    machine->uiStartButton();   // Paused → Running
    sleepMs(600);

    std::cout << ">>> 触发急停...\n";
    machine->uiEstopButton();
    sleepMs(100);

    std::cout << ">>> 释放急停...\n";
    machine->sendEvent(MachineEvent::EstopRelease);
    machine->uiResetButton();

    std::cout << ">>> 最终状态: " << statusToString(machine->currentStatus()) << "\n";
    std::cout << ">>> 示例 G 完成，总批次: " << batch << "\n";
}

// ============================================================
// 示例 H — 硬件按钮监控（ButtonMonitor + MotionController IO）
// ============================================================
// 本示例演示如何将运动控制卡 IO 信号对接到状态机：
//   1. 用 ButtonMonitor 监控 5 个物理按钮的 IO 信号
//   2. 通过模拟 IoReader 验证完整的按钮→事件→状态机流转
//
// 实际项目中将 mockIoReader 替换为真实的 MotionController::ReadIO 即可：
//   ButtonMonitor::IoReader reader =
//       [&motionCtrl](const std::string& name, bool& val) -> bool {
//           return motionCtrl.ReadIO(name, val) == nullptr;
//       };
// ============================================================

#include "../machine/button_monitor.h"

static void demoH_ButtonMonitor() {
    printSeparator("示例 H：硬件按钮监控（ButtonMonitor）");

    auto eventBus  = std::make_shared<EventBus>();
    auto scheduler = std::make_shared<Scheduler>(eventBus);
    auto alarmMgr  = std::make_shared<AlarmManager>(eventBus);
    auto machine   = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

    int batch = 0;
    auto def = FlowBuilder("监控测试流程")
        .script("处理", [&batch](FlowContext& ctx) {
            LOG_INFO("[Flow] 处理批次 #{}", ++batch);
            sleepMs(100);
            return StepResult::ok();
        })
        .build();
    scheduler->registerFlow(def);
    machine->enableContinuousMode({"监控测试流程", {}, 0, 50, true});

    machine->onStatusChanged([](MachineStatus from, MachineStatus to) {
        std::cout << "  [状态] " << statusToString(from)
                  << " → " << statusToString(to) << "\n";
    });

    // ── 模拟 IO 读取器（仿真各按钮信号）──────────────────────
    // 实际项目替换为：motionCtrl.ReadIO(name, val) == nullptr
    struct MockIO {
        bool start{false}, stop{false}, reset{false};
        bool modeAuto{true};    // 高=自动，低=手动
        bool estopOk{true};     // 高=正常，低=急停触发
    } io;

    ButtonMonitor::IoReader mockIoReader =
        [&io](const std::string& name, bool& val) -> bool {
            if (name == "DI_StartBtn")   { val = io.start;    return true; }
            if (name == "DI_StopBtn")    { val = io.stop;     return true; }
            if (name == "DI_ResetBtn")   { val = io.reset;    return true; }
            if (name == "DI_ModeSwitch") { val = io.modeAuto; return true; }
            if (name == "DI_Estop")      { val = io.estopOk;  return true; }
            return false;
        };

    // ── 创建并启动 ButtonMonitor ──────────────────────────────
    ButtonMonitor::Config cfg;
    cfg.startBtn       = "DI_StartBtn";
    cfg.stopBtn        = "DI_StopBtn";
    cfg.resetBtn       = "DI_ResetBtn";
    cfg.modeSwitch     = "DI_ModeSwitch";
    cfg.estop          = "DI_Estop";
    cfg.pollIntervalMs = 20;

    auto monitor = std::make_shared<ButtonMonitor>(machine, mockIoReader, cfg);

    machine->init();
    monitor->start();

    sleepMs(50);
    std::cout << ">>> [仿真] 按下复位按钮...\n";
    io.reset = true;   sleepMs(40);
    io.reset = false;  sleepMs(100);

    // ── 仿真场景 1：自动模式下按启动 ─────────────────────────
    std::cout << ">>> [仿真] 按下启动按钮...\n";
    io.start = true;   sleepMs(40);
    io.start = false;  sleepMs(400);   // 运行 400ms

    // ── 仿真场景 2：物理停止按钮（暂停语义）─────────────────
    std::cout << ">>> [仿真] 按下停止按钮（暂停）...\n";
    io.stop = true;    sleepMs(40);
    io.stop = false;   sleepMs(300);   // 等待当前料完成

    // ── 仿真场景 3：按启动恢复 ────────────────────────────────
    std::cout << ">>> [仿真] 再次按下启动按钮（恢复生产）...\n";
    io.start = true;   sleepMs(40);
    io.start = false;  sleepMs(400);

    // ── 仿真场景 4：手自动切换拨到手动 ──────────────────────
    std::cout << ">>> [仿真] 手自动切换拨到手动...\n";
    io.modeAuto = false;  sleepMs(200);

    // ── 仿真场景 5：手动模式下误按启动（应被拦截）───────────
    std::cout << ">>> [仿真] 手动模式下误按启动（应被拦截）...\n";
    io.start = true;   sleepMs(40);
    io.start = false;  sleepMs(100);

    // ── 仿真场景 6：切回自动 → 启动 ─────────────────────────
    std::cout << ">>> [仿真] 切回自动模式...\n";
    io.modeAuto = true;  sleepMs(80);
    io.start = true;   sleepMs(40);
    io.start = false;  sleepMs(400);

    // ── 仿真场景 7：急停触发与恢复 ───────────────────────────
    std::cout << ">>> [仿真] 触发急停（常闭触点断开）...\n";
    io.estopOk = false;  sleepMs(150);
    std::cout << ">>> [仿真] 释放急停（常闭触点恢复）...\n";
    io.estopOk = true;   sleepMs(100);
    io.reset = true;   sleepMs(40);
    io.reset = false;  sleepMs(100);

    monitor->stop();    // 停止 IO 轮询

    std::cout << ">>> 最终状态: " << statusToString(machine->currentStatus()) << "\n";
    std::cout << ">>> 示例 H 完成，总批次: " << batch << "\n";
    ScriptRegistry::instance().clear();
}

// ============================================================
// main
// ============================================================

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::cout << "===================================================\n"
              << "  调度引擎 & 状态机 综合示例\n"
              << "===================================================\n\n";

    demoA_SingleStationContinuous();
    demoB_PauseResume();
    demoC_ErrorRecovery();
    demoE_MultiStationPipeline();
    demoF_MultiStationJsonLoad();
    demoG_AlarmAndRecovery();
    demoH_ButtonMonitor();

    std::cout << "\n===================================================\n"
              << "  所有示例执行完毕\n"
              << "===================================================\n";
    return 0;
}
