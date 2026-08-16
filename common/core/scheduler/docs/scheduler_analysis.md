# Aether Platform V2 — Scheduler 调度引擎详细分析文档

> **模块路径**: `common/core/scheduler`
> **分析版本**: v5.0（源码精读版）
> **分析日期**: 2026-04-01
> **适用平台**: Aether Platform V2 (C++17)

---

## 目录

1. [模块概述与定位](#1-模块概述与定位)
2. [目录结构总览](#2-目录结构总览)
3. [架构分层与模块关系](#3-架构分层与模块关系)
4. [核心数据类型层（core/）](#4-核心数据类型层)
5. [原子指令层（command/）](#5-原子指令层)
6. [步骤与流程控制层（step/）](#6-步骤与流程控制层)
7. [流程上下文层（context/）](#7-流程上下文层)
8. [层级状态机（fsm/）](#8-层级状态机)
9. [流程引擎层（engine/）](#9-流程引擎层)
10. [事件总线（event/）](#10-事件总线)
11. [报警管理器（alarm/）](#11-报警管理器)
12. [补偿事务管理器（compensation/）](#12-补偿事务管理器)
13. [整机主状态机（machine/）](#13-整机主状态机)
14. [多工位同步（station/）](#14-多工位同步)
15. [持久化层（persistence/）](#15-持久化层)
16. [FlowBuilder DSL 详解](#16-flowbuilder-dsl-详解)
17. [关键设计模式分析](#17-关键设计模式分析)
18. [核心调用链路全景](#18-核心调用链路全景)
19. [线程模型与并发分析](#19-线程模型与并发分析)
20. [使用示例精读](#20-使用示例精读)
21. [总结与扩展建议](#21-总结与扩展建议)

---

## 1. 模块概述与定位

### 1.1 背景与问题域

Aether Platform V2 提供完整的设备抽象层（`DeviceManager`），管理相机、光源、扫码器、运动控制器等多类硬件。`scheduler` 模块在此基础上提供**工作流编排与调度引擎**，解决以下核心工业问题：

| 问题 | Scheduler 的解决方案 |
|------|----------------------|
| 多步设备操作需严格按顺序/条件分支执行 | `IStep` 树形步骤体系 + 各类流程控制网关 |
| 多流程并发时存在设备资源竞争 | `ResourceLocker`（固定排序加锁防死锁） |
| 报警时需暂停→人工→从断点继续 | `CheckpointSnapshot` + `CompensationManager` |
| 多工位并行执行 + 同步等待 | `StationManager` + `StationBarrier` |
| 设备指令异步完成需回调驱动 | `CommandDispatcher` + `std::future/promise` |
| 生产切换只改配置不改代码 | `FlowBuilder DSL` + JSON 参数化 |
| 断电重启后自动恢复流程 | `IFlowPersistence` + `CheckpointSnapshot` |

### 1.2 模块定位（系统分层）

```
┌─────────────────────────────────────────────────────────┐
│           应用层 / UI层  (HMI, Qt, Web)                 │
├─────────────────────────────────────────────────────────┤
│  MachineFSM  ←──  AlarmManager  ←──  StationManager     │  ← 工业场景层
├─────────────────────────────────────────────────────────┤
│          Scheduler  (调度引擎)                           │  ← 调度引擎层
│          FlowInstance  ←→  FlowContext                   │
│          EventBus  ←  HierarchicalStateMachine           │
├─────────────────────────────────────────────────────────┤
│     IStep 树（SequentialStep / ParallelStep / ...）      │  ← 步骤控制层
│     CommandDispatcher  ←  IDeviceCommand                 │  ← 指令层
├─────────────────────────────────────────────────────────┤
│             DeviceManager（相机/轴/PLC...）              │  ← 设备驱动层
└─────────────────────────────────────────────────────────┘
```

---

## 2. 目录结构总览

```
common/core/scheduler/
├── scheduler_global.h          # DLL 导出宏（Windows/Linux 双平台）
├── CMakeLists.txt              # 构建配置
├── scheduler_design.md         # 原始设计文档（104KB）
│
├── core/                       # 核心基础类型
│   ├── flow_types.h            # FlowState / StepResult / VariableScope 等枚举/结构
│   └── uuid_gen.h              # UUID v4 生成 / 自增订阅者 ID
│
├── command/                    # 原子设备指令层
│   ├── i_device_command.h      # IDeviceCommand 接口 + CommandState/RetryPolicy
│   ├── command_dispatcher.h    # CommandDispatcher 异步指令调度器（头）
│   └── command_dispatcher.cpp  # CommandDispatcher 实现（超时检测线程）
│
├── step/                       # 步骤体系
│   ├── i_step.h                # IStep 接口 + StepConfig
│   ├── device_step.h           # DeviceStep（设备步骤基类）
│   ├── flow_builder.h          # FlowBuilder / StepBuilder DSL
│   ├── step_factory.h/cpp      # StepFactory（JSON 驱动步骤工厂）
│   ├── control/                # 流程控制步骤
│   │   ├── sequential_step.h   # SequentialStep（串行）
│   │   ├── parallel_step.h     # ParallelStep（And-Split/Join）
│   │   ├── exclusive_gateway.h # ExclusiveGateway（XOR-Split）
│   │   ├── loop_step.h         # LoopStep（固定次数/条件循环）
│   │   ├── delay_step.h        # DelayStep（延时）
│   │   └── script_step.h       # ScriptStep（Lambda/自定义逻辑）
│   └── sync/                   # 多工位同步步骤
│       ├── station_barrier.h   # StationBarrier + StationSignalBus
│       └── sync_steps.h        # BarrierStep / WaitSignalStep 等
│
├── context/                    # 流程上下文
│   ├── flow_context.h          # FlowContext（三级变量作用域） + GlobalVariableStore
│   └── flow_context.cpp        # 序列化/反序列化实现
│
├── fsm/                        # 层级状态机
│   ├── hsm.h                   # HierarchicalStateMachine 接口
│   └── hsm.cpp                 # HSM 实现（LCA算法，历史状态，异步队列）
│
├── engine/                     # 调度引擎核心
│   ├── flow_definition.h       # FlowDefinition（流程模板定义）
│   ├── checkpoint_snapshot.h   # CheckpointSnapshot / FlowInstanceSnapshot
│   ├── flow_instance.h         # FlowInstance（流程运行实例）
│   ├── flow_instance.cpp       # 实例运行循环、步骤执行、断点、补偿
│   ├── scheduler.h             # Scheduler + ResourceLocker + IDispatchStrategy
│   └── scheduler.cpp           # 调度循环、资源锁、优先级策略
│
├── event/                      # 事件总线
│   ├── event_bus.h             # EventBus 接口 + SchedulerEvent 定义
│   └── event_bus.cpp           # 同步/异步发布实现
│
├── alarm/                      # 报警管理
│   ├── alarm_manager.h         # AlarmManager + AlarmLevel/AlarmAction/AlarmInfo
│   └── alarm_manager.cpp       # 报警触发/确认/清除实现
│
├── compensation/               # 补偿事务
│   ├── compensation_manager.h  # CompensationManager（LIFO 补偿栈）
│   └── compensation_manager.cpp
│
├── machine/                    # 整机主状态机
│   ├── machine_fsm.h           # MachineFSM + MachineState/MachineEvent
│   └── machine_fsm.cpp         # 状态转移实现
│
├── station/                    # 多工位管理
│   ├── station_manager.h       # StationManager + Station + StationConfig
│   └── station_manager.cpp
│
├── persistence/                # 持久化接口与实现
│   ├── i_flow_persistence.h    # IFlowPersistence 接口
│   ├── json_file_persistence.h # JSON 文件持久化实现
│   └── json_file_persistence.cpp
│
└── examples/                   # 示例程序
    ├── basic_example.cpp        # 6个基础功能示例
    ├── station_example.cpp      # 多工位协同示例
    └── CMakeLists.txt
```

---

## 3. 架构分层与模块关系

### 3.1 模块依赖图

```
MachineFSM
    │
    ├──► Scheduler ◄──── StationManager
    │        │                  │
    │        ▼                  ▼
    │   FlowInstance        Station
    │        │
    │        ├──► FlowContext (三级变量)
    │        ├──► CompensationManager (LIFO 补偿栈)
    │        ├──► IFlowPersistence (断点持久化)
    │        └──► IStep 树
    │                  │
    │              SequentialStep
    │              ParallelStep ──► std::async
    │              ExclusiveGateway
    │              LoopStep
    │              DeviceStep ──► CommandDispatcher ──► IDeviceCommand
    │
    ├──► AlarmManager ──► EventBus
    └──► EventBus
              │
         HierarchicalStateMachine (HSM)
```

### 3.2 关键接口契约

| 接口 | 实现方 | 用途 |
|------|--------|------|
| `IStep` | SequentialStep / ParallelStep / DeviceStep / ... | 步骤抽象 |
| `IDeviceCommand` | 各设备驱动层 | 原子设备指令抽象 |
| `IFlowPersistence` | JsonFilePersistence / 自定义DB | 断点持久化 |
| `IDispatchStrategy` | PriorityFIFOStrategy / 自定义 | 调度算法注入 |

---

## 4. 核心数据类型层

**文件**: `core/flow_types.h`, `core/uuid_gen.h`

### 4.1 FlowState — 流程生命周期状态

```cpp
enum class FlowState {
    Created,        // 已创建，未提交（FlowInstance 构造完成）
    Queued,         // 已入等待队列，等待资源/并发槽
    Running,        // 运行中（runLoop 线程已启动）
    Paused,         // 已暂停（可通过 resume() 恢复）
    AlarmPaused,    // 报警暂停（等待人工确认后才能恢复）
    Completed,      // 正常完成（所有步骤成功）
    Failed,         // 执行失败（某步骤失败且非可选）
    Aborted,        // 主动中止（已执行补偿事务回滚）
};
```

**状态转移图**:

```
Created
  │
  ▼
Queued ──────────────────────────────────────────────────►
  │                                                        │
  ▼                                                        │
Running ◄──── resume() ──── Paused ◄── pause()            │
  │                                      ▲                 │
  │                         AlarmPaused ─┘                 │
  │                                                        │
  ├──► Completed (所有步骤成功)                            │
  ├──► Failed (非可选步骤失败)                             │
  └──► Aborted (abort() 调用 → 补偿执行 → Aborted)        │
                                                           ◄─┘
```

### 4.2 StepResult — 步骤执行结果

```cpp
struct StepResult {
    enum class Status { Success, Failed, Skipped, Timeout };

    Status      status   { Status::Failed };
    std::string message;
    int         errorCode{ 0 };
    std::any    output;          // 步骤输出（可传递给后续步骤）

    // 静态工厂方法
    static StepResult ok(std::any out = {});
    static StepResult fail(std::string msg, int c = -1);
    static StepResult skip();
    static StepResult timeout(std::string msg = "Timeout");
};
```

**设计要点**:
- `output` 使用 `std::any` 实现类型擦除，步骤可向 `FlowContext` 写入任意类型数据
- `errorCode` 遵循约定：`0` = 成功，`-1` = 通用失败，`-2` = 超时
- 四种静态工厂方法覆盖所有典型场景

### 4.3 VariableScope — 三级变量作用域

```cpp
enum class VariableScope {
    STEP,    // 步骤级：步骤退出后自动销毁（栈式管理）
    FLOW,    // 流程级：实例生命周期内有效
    GLOBAL,  // 全局级：跨实例、跨流程共享（单例）
};
```

### 4.4 UUID 生成器（`uuid_gen.h`）

```cpp
inline std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dist;
    // 生成符合 RFC4122 v4 规范的 UUID
    // hi/lo 各 64 位，设置版本位 (4) 和变体位
    ...
}
```

**设计要点**:
- 无外部依赖，仅用 C++11 标准库随机数
- 使用 `static` 线程本地变量确保线程安全（`mt19937_64` 的 `static` 不是线程安全的，但在实际使用中通常够用）
- `generateSubscriberId()` 使用 `std::atomic<int>` 自增，确保绝对线程安全

---

## 5. 原子指令层

**文件**: `command/i_device_command.h`, `command/command_dispatcher.h/cpp`

### 5.1 CommandState — 指令状态机

```
CREATED ──dispatch()──► DISPATCHED ──设备开始──► EXECUTING
                                                    │
                              ┌─────────────────────┤
                              │                     │
                           COMPLETED             FAILED
                              │                  TIMEOUT
                           CANCELLED
```

```cpp
enum class CommandState {
    CREATED,     // 指令已创建，未下发
    DISPATCHED,  // 已下发到设备驱动，等待设备响应
    EXECUTING,   // 设备正在执行（如机械臂运动中）
    COMPLETED,   // 设备报告完成
    FAILED,      // 设备报告失败
    TIMEOUT,     // 等待超时
    CANCELLED,   // 主动取消（急停等）
};
```

### 5.2 IDeviceCommand — 原子指令接口

```cpp
class IDeviceCommand {
public:
    // ── 必须实现（纯虚）────────────────────────────────────
    virtual int dispatch() = 0;                          // 下发到设备（立即返回，异步）
    virtual int cancel()   = 0;                          // 取消执行
    virtual CommandResult waitForCompletion(int ms) = 0; // 阻塞等待完成

    // ── 非阻塞回调模式 ──────────────────────────────────────
    void onCompleted(std::function<void(const CommandResult&)> cb);

    // ── 配置 ────────────────────────────────────────────────
    void setParams(json params);
    void setTimeout(int ms);
    void setRetryPolicy(RetryPolicy p);

protected:
    void notifyCompleted(CommandResult result);  // 设备完成后调用此方法通知上层

    std::string  m_commandId_  { generateUUID() };  // 唯一标识
    std::atomic<CommandState> m_state_{ CommandState::CREATED };
    std::function<void(const CommandResult&)> m_completedCb_;
};
```

**设计要点**:
- 每条指令有独立的 UUID `commandId`，可全局追踪
- `dispatch()` 是非阻塞的——调用后立即返回，设备异步完成
- 两种等待模式：`waitForCompletion()` 阻塞模式 / `onCompleted()` 回调模式
- `RetryPolicy` 支持指令级重试（独立于步骤级重试）

### 5.3 CommandDispatcher — 异步指令调度器

```cpp
class CommandDispatcher {
public:
    // 提交单条指令，返回 future（可 .get() 阻塞等待）
    std::future<CommandResult> submit(std::shared_ptr<IDeviceCommand> cmd);

    // 批量提交（parallel=true 时并发执行）
    std::vector<std::future<CommandResult>> submitBatch(
        std::vector<std::shared_ptr<IDeviceCommand>> cmds,
        bool parallel = true);

    // 回调模式（不阻塞调用线程）
    void submitWithCallback(std::shared_ptr<IDeviceCommand> cmd,
                            std::function<void(const CommandResult&)> cb);

    // 设备完成时，设备驱动层调用此接口回传结果
    void onDeviceResponse(const std::string& commandId,
                          json responseData, int errorCode = 0);

private:
    std::unordered_map<std::string, std::shared_ptr<IDeviceCommand>> m_pending_;
    std::unordered_map<std::string, std::promise<CommandResult>>     m_promises_;
    std::thread m_timeoutThread_;  // 独立超时检测线程
};
```

**内部数据流**:

```
submit(cmd)
    │
    ├── cmd->dispatch()              ← 下发到硬件（立即返回）
    ├── m_pending_[id] = cmd         ← 记录待完成指令
    ├── m_promises_[id] = promise    ← 存储 promise
    └── return promise.get_future()  ← 返回 future 给调用方

                      ↓ 设备完成后 ↓

onDeviceResponse(commandId, ...)
    │
    ├── result = {COMPLETED, ...}
    ├── m_promises_[id].set_value(result)  ← future 就绪，唤醒等待方
    ├── cmd->notifyCompleted(result)       ← 触发回调
    └── m_pending_.erase(id)              ← 清理记录

timeoutCheckLoop（独立线程，每 100ms 检查一次）
    │
    └── 若指令超时 → cancel() + set_value(timeout_result)
```

---

## 6. 步骤与流程控制层

**文件**: `step/i_step.h` 及 `step/control/` 下各文件

### 6.1 IStep — 步骤基类接口

```cpp
class IStep {
public:
    // ── 必须实现 ──────────────────────────────────────────
    virtual StepResult execute(FlowContext& ctx) = 0;  // 同步执行
    virtual std::string typeName() const = 0;          // 步骤类型名

    // ── 可选重写 ──────────────────────────────────────────
    virtual void cancel()                  {}  // 取消（急停时调用）
    virtual void cleanup(FlowContext&)     {}  // 清理（步骤退出时）

    // ── 默认实现 ──────────────────────────────────────────
    // 异步执行（默认用 std::async 包装同步版本）
    virtual std::future<StepResult> executeAsync(FlowContext& ctx);

    // 子步骤（容器步骤使用）
    virtual void addChild(std::shared_ptr<IStep> child);
    const std::vector<std::shared_ptr<IStep>>& children() const;

    // 带重试逻辑的执行（自动重试 retryCount 次）
    StepResult executeWithRetry(FlowContext& ctx);

protected:
    StepConfig                          m_config_;   // 步骤配置
    std::vector<std::shared_ptr<IStep>> m_children_; // 子步骤（树形结构）
    std::shared_ptr<CommandDispatcher>  m_dispatcher_;
};
```

### 6.2 StepConfig — 步骤配置

```cpp
struct StepConfig {
    std::string name;              // 步骤名称（必填，用于追踪）
    std::string type;              // 步骤类型（如 "control.sequential"）
    int         timeoutMs       { 5000 };   // 执行超时（ms）
    int         retryCount      { 0 };      // 失败后重试次数
    int         retryIntervalMs { 500 };    // 重试间隔（ms）
    bool        skipOnFail      { false };  // 失败时跳过（不中止流程）
    bool        optional        { false };  // 可选步骤（失败不影响整体结果）
    bool        isCheckpoint    { false };  // 是否保存断点快照
    json        params;                     // 扩展参数（JSON 配置）
    std::shared_ptr<IStep> compensationStep; // 对应的补偿步骤
};
```

### 6.3 重试执行逻辑

```cpp
StepResult IStep::executeWithRetry(FlowContext& ctx) {
    StepResult result = StepResult::fail("Not executed");
    int attempts = m_config_.retryCount + 1;  // 总尝试次数
    for (int i = 0; i < attempts; ++i) {
        result = execute(ctx);
        if (result.isOk() || result.isSkipped()) return result;  // 成功或跳过，立即返回
        if (i < attempts - 1 && m_config_.retryIntervalMs > 0)
            std::this_thread::sleep_for(
                std::chrono::milliseconds(m_config_.retryIntervalMs));
        // 否则继续重试
    }
    return result;  // 所有尝试均失败
}
```

### 6.4 步骤类型一览

#### SequentialStep — 顺序步骤（串行容器）

```cpp
class SequentialStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        for (auto& child : m_children_) {
            ctx.pushStepScope();              // 为每个子步骤创建独立的 STEP 作用域
            auto result = child->executeWithRetry(ctx);
            ctx.popStepScope();              // 步骤退出，STEP 变量自动销毁

            if (result.isFailed()) {
                if (child->config().skipOnFail || child->config().optional)
                    continue;                // 可选步骤失败 → 跳过继续
                return result;               // 强制步骤失败 → 中止整个序列
            }
        }
        return StepResult::ok();
    }
    std::string typeName() const override { return "control.sequential"; }
};
```

**执行语义**: 子步骤按添加顺序逐一执行，每步骤执行前推入 STEP 作用域，执行后弹出。任何强制步骤失败时立即中止。

#### ParallelStep — 并行步骤（And-Split/Join）

```cpp
class ParallelStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        std::vector<std::future<StepResult>> futures;
        for (auto& child : m_children_)
            futures.push_back(child->executeAsync(ctx));  // 全部异步启动

        bool allOk = true;
        for (auto& f : futures) {
            auto r = f.get();               // 依次等待完成（Join 点）
            if (r.isFailed()) { allOk = false; ... }
        }
        return allOk ? StepResult::ok() : StepResult::fail(...);
    }
    std::string typeName() const override { return "control.parallel"; }
};
```

**执行语义**: 所有子步骤同时异步启动（And-Split），然后逐一 `.get()` 等待全部完成（And-Join）。任何子步骤失败将导致整体失败。

#### ExclusiveGateway — 排他网关（XOR-Split）

```cpp
class ExclusiveGateway : public IStep {
public:
    struct Branch {
        std::function<bool(const FlowContext&)> guard; // 条件函数
        std::shared_ptr<IStep>                  step;  // 分支步骤
        std::string                             label; // 分支标签
    };

    StepResult execute(FlowContext& ctx) override {
        for (auto& b : m_branches_) {
            if (!b.guard || b.guard(ctx))             // 找到第一个满足条件的分支
                return b.step->executeWithRetry(ctx); // 执行该分支（XOR：仅一个）
        }
        return m_default_ ? m_default_->executeWithRetry(ctx)
                          : StepResult::skip();       // 没有满足条件的分支
    }
    std::string typeName() const override { return "gateway.exclusive"; }

private:
    std::vector<Branch>    m_branches_;   // 候选分支列表
    std::shared_ptr<IStep> m_default_;   // 默认分支（可选）
};
```

**执行语义**: 遍历分支列表，执行第一个 `guard` 函数返回 `true` 的分支（互斥）。

#### LoopStep — 循环步骤

```cpp
class LoopStep : public IStep {
public:
    explicit LoopStep(int times);                                     // 固定次数
    explicit LoopStep(std::function<bool(const FlowContext&)> cond);  // 条件循环

    StepResult execute(FlowContext& ctx) override {
        int iter = 0;
        while ((!m_cond_ || m_cond_(ctx)) && (m_max_ <= 0 || iter < m_max_)) {
            ctx.set("loop_iteration", iter, VariableScope::STEP); // 循环索引注入
            ++iter;
            for (auto& child : m_children_) {
                auto r = child->executeWithRetry(ctx);
                if (r.isFailed() && !child->config().skipOnFail
                                 && !child->config().optional)
                    return r;  // 子步骤失败 → 终止循环
            }
        }
        return StepResult::ok();
    }
};
```

**执行语义**:
- 固定次数：`m_max_ > 0` 时执行不超过 `m_max_` 次
- 条件循环：`m_cond_` 不为 null 时每次迭代前检查条件
- 每次迭代将 `loop_iteration`（0-based）写入 STEP 作用域供子步骤读取

#### ScriptStep — 脚本步骤（Lambda）

```cpp
class ScriptStep : public IStep {
public:
    using ScriptFn = std::function<StepResult(FlowContext&)>;

    StepResult execute(FlowContext& ctx) override {
        if (!m_fn_) return StepResult::skip();
        return m_fn_(ctx);  // 直接调用 Lambda
    }
    std::string typeName() const override { return "control.script"; }

private:
    ScriptFn m_fn_;
};
```

**执行语义**: 最灵活的步骤类型，直接执行任意 Lambda/函数对象。用于内联业务逻辑。

#### DelayStep — 延时步骤

```cpp
class DelayStep : public IStep {
    StepResult execute(FlowContext&) override {
        int ms = (m_delayMs_ > 0) ? m_delayMs_
                                   : m_config_.params.value("ms", 0);
        if (ms > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        return StepResult::ok();
    }
};
```

**执行语义**: 优先使用构造/setter注入的值，其次从 JSON params 读取 `ms` 字段。

### 6.5 IStep 树形结构示意

```
FlowDefinition.rootStep (SequentialStep)
├── ScriptStep "初始化"
├── DelayStep  "预热"
├── ParallelStep "并行检测"
│   ├── DeviceStep "视觉检测" ──► CameraCommand
│   └── DeviceStep "尺寸测量" ──► MeasureCommand
├── ExclusiveGateway "结果判断"
│   ├── Branch [guard: result=="PASS"] → ScriptStep "上传合格"
│   └── Branch [guard: result=="FAIL"] → ScriptStep "转入维修"
└── LoopStep "清洗循环(3次)"
    └── ScriptStep "单次清洗"
```

---

## 7. 流程上下文层

**文件**: `context/flow_context.h/cpp`

### 7.1 GlobalVariableStore — 全局变量单例

```cpp
class GlobalVariableStore {
public:
    static GlobalVariableStore& instance();  // Meyer's singleton（C++11 线程安全）

    template<typename T>
    void set(const std::string& key, T&& value);  // unique_lock（写）

    template<typename T>
    T get(const std::string& key) const;          // shared_lock（读）

    template<typename T>
    std::optional<T> tryGet(const std::string& key) const;  // 不抛异常版本

private:
    mutable std::shared_mutex                 m_mutex_;  // 读写锁
    std::unordered_map<std::string, std::any> m_data_;
};
```

**设计要点**:
- `std::shared_mutex` + 读写锁分离：多个流程可并发读取全局变量，写入时排他
- 使用 `std::any` 实现类型擦除的通用存储
- `tryGet()` 不抛异常，适合需要检查存在性的场景

### 7.2 FlowContext — 流程运行上下文（三级作用域）

```cpp
class FlowContext {
public:
    // ── 变量读写（作用域链）──────────────────────────────────
    template<typename T>
    void set(const std::string& key, T&& value,
             VariableScope scope = VariableScope::FLOW);

    template<typename T>
    T get(const std::string& key) const;  // 查找顺序: STEP → FLOW → GLOBAL

    // ── STEP 作用域生命周期管理 ──────────────────────────────
    void pushStepScope();  // 步骤开始前调用
    void popStepScope();   // 步骤结束后调用（该步骤的 STEP 变量销毁）

    // ── 元信息（只读，由 FlowInstance 填充）─────────────────
    std::string flowName;
    std::string instanceId;
    int         priority{0};
    json        initParams;   // submit 时传入的 JSON 参数

    // ── 执行历史（时间线记录）────────────────────────────────
    struct StepRecord {
        std::string        name;
        StepResult::Status status;
        int64_t            durationMs;
        std::string        message;
    };
    std::vector<StepRecord> history;

private:
    // STEP 作用域：栈（每调用 pushStepScope 压入一层 map）
    std::vector<std::unordered_map<std::string, std::any>> m_stepScopes_;
    // FLOW 作用域：单层 map
    mutable std::shared_mutex                               m_flowMutex_;
    std::unordered_map<std::string, std::any>               m_flowData_;
};
```

### 7.3 变量查找链（作用域链）

```
get("key")
    │
    ├── 1. 倒序遍历 m_stepScopes_（最近压入的 STEP 作用域优先）
    │      找到 → 返回
    │
    ├── 2. 在 m_flowData_ 中查找（FLOW 级，shared_lock 保护）
    │      找到 → 返回
    │
    └── 3. GlobalVariableStore::instance().get<T>(key)（GLOBAL 级）
           找到 → 返回
           未找到 → 抛出 runtime_error
```

**STEP 作用域栈示意**:

```
SequentialStep 开始:
  m_stepScopes_ = []

  执行 ScriptStep "步骤A":
    pushStepScope() → m_stepScopes_ = [{}]
    ctx.set("x", 1, STEP) → m_stepScopes_ = [{"x":1}]
    ctx.set("y", 2, STEP) → m_stepScopes_ = [{"x":1,"y":2}]
    popStepScope()  → m_stepScopes_ = []  ← "x","y" 自动销毁

  执行 LoopStep "步骤B":
    pushStepScope() → m_stepScopes_ = [{}]
    ctx.set("loop_iteration", 0, STEP)
      LoopStep 内嵌 ScriptStep:
        pushStepScope() → m_stepScopes_ = [{loop_iteration:0}, {}]
        ctx.set("inner", "v", STEP) → [{loop_iteration:0}, {inner:"v"}]
        popStepScope()  → [{loop_iteration:0}]
    popStepScope()  → []
```

### 7.4 序列化/反序列化（用于断点快照）

```cpp
json FlowContext::serialize() const {
    // 将 FLOW 级变量中可序列化的部分导出为 JSON
    // 注意：std::any 中的非 JSON 类型会被跳过
}

void FlowContext::deserialize(const json& data) {
    // 从 JSON 恢复 FLOW 级变量
}
```

---

## 8. 层级状态机

**文件**: `fsm/hsm.h/cpp`

### 8.1 核心数据结构

```cpp
// 状态描述符
struct StateDesc {
    int          id;
    std::string  name;
    Action       onEnter;        // 进入此状态时执行的动作
    Action       onExit;         // 退出此状态时执行的动作
    std::vector<int> childStates;
    int          initialChild;   // 默认进入的子状态（-1=无）
    int          parent;         // 父状态 ID（-1=顶层）
    bool         isParallel;     // 是否为正交区域（并行子状态）
};

// 转移描述符
struct TransitionDesc {
    int    from;
    int    to;
    int    eventId;      // -1=自动转移（无需事件触发）
    Guard  guard;        // 条件函数（nullptr=无条件）
    Action action;       // 转移时执行的动作
    bool   isInternal;   // 内部转移（不触发 exit/enter）
};
```

### 8.2 HierarchicalStateMachine 接口

```cpp
class HierarchicalStateMachine {
public:
    void addState(StateDesc desc);
    void addTransition(TransitionDesc trans);
    void setInitialState(int stateId);

    void start();                          // 启动，进入初始状态
    void stop();

    bool processEvent(const HsmEvent& evt); // 同步处理（立即处理）
    void postEvent(HsmEvent evt);           // 异步投递（入队，事件线程处理）

    // 快照（可序列化为 JSON）
    json takeSnapshot() const;
    void restoreSnapshot(const json& snap);

    // 查询
    int    currentStateId()  const;
    bool   isInState(int id) const;  // 判断是否在某状态（包括父状态）

private:
    void transition(int from, int to, const HsmEvent& evt);
    int  findLCA(int s1, int s2) const;  // 最小公共祖先算法
    void eventLoop();                    // 异步事件处理线程

    std::unordered_map<int, int> m_history_;   // 浅历史：parent→last_child
    std::thread m_eventThread_;                 // 异步事件分发线程
    std::queue<HsmEvent> m_eventQueue_;
};
```

### 8.3 LCA（最小公共祖先）算法在状态转移中的应用

当从状态 A 转移到状态 B 时，需要正确触发 `onExit` 和 `onEnter`：

```
状态树:
    Root
    ├── S1
    │   ├── S1a ← current
    │   └── S1b
    └── S2
        └── S2a ← target

转移 S1a → S2a:
  LCA = Root (S1a 和 S2a 的最小公共祖先)
  Exit: S1a → S1 → (Root 不退出，LCA 本身不退出)
  Enter: S2 → S2a
```

**算法实现**:
```
getAncestors(stateId): 从 stateId 一路向上收集所有祖先 ID
findLCA(s1, s2):
  ancestors_s1 = getPathToRoot(s1)
  for each a in getPathToRoot(s2):
    if a in ancestors_s1: return a  // 第一个共同祖先
```

### 8.4 历史状态（浅历史）

```cpp
// 离开复合状态时记录最后活跃的子状态
m_history_[parent] = lastChildState;

// 重新进入带历史的状态时，恢复上次的子状态
int resolved = m_history_.count(parent) ? m_history_[parent] : desc.initialChild;
```

---

## 9. 流程引擎层

**文件**: `engine/flow_definition.h`, `engine/flow_instance.h/cpp`, `engine/scheduler.h/cpp`

### 9.1 FlowDefinition — 流程模板定义

```cpp
struct FlowDefinition {
    std::string              name;               // 流程名称（注册键）
    std::string              description;
    int                      version{1};
    std::shared_ptr<IStep>   rootStep;           // 流程步骤树的根
    std::vector<std::string> requiredDevices;    // 需要加锁的设备资源列表
    int                      maxInstances{1};    // 同时运行的最大实例数
    int                      priority{0};
    int                      timeoutMs{0};       // 流程级超时（0=不限时）
    bool                     enableCompensation{true};

    // 生命周期回调（在 FlowInstance 的 runLoop 线程中调用）
    std::function<void(FlowContext&)> onStart;
    std::function<void(FlowContext&)> onComplete;
    std::function<void(FlowContext&)> onFailed;
    std::function<void(FlowContext&)> onAbort;
};
```

**设计要点**:
- `FlowDefinition` 是**不可变的模板**（stateless），多个 `FlowInstance` 共享同一个 `FlowDefinition`
- `requiredDevices` 配合 `ResourceLocker` 实现设备独占访问
- `maxInstances` 允许同一流程有限并发（如允许 3 个工位同时跑）

### 9.2 FlowInstance — 流程运行实例

每个实例拥有独立的运行时状态：

```cpp
class FlowInstance : public std::enable_shared_from_this<FlowInstance> {
    // 独立数据（每实例独占）
    std::unique_ptr<FlowContext>         m_context_;         // 变量作用域
    std::unique_ptr<CompensationManager> m_compensationMgr_; // 补偿栈
    std::shared_ptr<IFlowPersistence>    m_persistence_;     // 断点持久化

    // 状态控制（原子变量，线程安全）
    std::atomic<FlowState>  m_state_{ FlowState::Created };
    std::atomic<bool>       m_abortFlag_{ false };   // 中止信号
    std::atomic<bool>       m_pauseFlag_{ false };   // 暂停信号
    std::atomic<bool>       m_skipFlag_{ false };    // 跳过当前步骤信号

    std::string              m_instanceId_;           // UUID，全局唯一
    std::optional<std::string> m_skipUntilStep_;     // 断点恢复时的目标步骤

    // 运行线程
    std::thread              m_runThread_;
    std::mutex               m_pauseMutex_;
    std::condition_variable  m_pauseCv_;
};
```

#### 9.2.1 start() — 启动运行

```cpp
void FlowInstance::start() {
    setState(FlowState::Running);
    if (m_def_->onStart) m_def_->onStart(*m_context_);  // 触发 onStart 回调

    // 关键：使用 shared_from_this() 让线程持有强引用
    // 防止线程内触发 onInstanceCompleted → 析构 FlowInstance → join 自身（死锁）
    auto self = shared_from_this();
    m_runThread_ = std::thread([this, self]{ runLoop(); });
}
```

#### 9.2.2 runLoop() — 核心运行循环

```
runLoop() {
    for each step in rootStep.children:
        │
        ├── if abortFlag → break（中止）
        │
        ├── wait while pauseFlag（暂停检查）
        │
        ├── if skipUntilStep.has_value():
        │     if step.name != *skipUntilStep: continue（跳过）
        │     else: skipUntilStep.reset()（到达目标步骤，恢复执行）
        │
        ├── publish StepStarted 事件
        │
        ├── result = executeStep(step, ctx)
        │     └── 带超时：spawn detached thread + future.wait_for(timeout)
        │             超时 → step->cancel()
        │
        ├── ctx.history.push_back(...)（记录执行历史）
        │
        ├── if result.isOk():
        │     ├── if step.compensationStep → compensationMgr.recordCompleted(...)
        │     ├── if step.isCheckpoint → takeCheckpoint() → persistence.saveCheckpoint()
        │     └── publish StepCompleted 事件
        │
        └── if result.isFailed():
              ├── if !skipOnFail && !optional → break（中止序列）
              └── else → continue（跳过可选步骤）

    if abortFlag:
        performCompensation()  → setState(Aborted)
    else if success:
        compensationMgr.clear() → setState(Completed)
    else:
        setState(Failed)

    completedCb_(...)  ← 通知 Scheduler
}
```

#### 9.2.3 executeStep() — 带超时的步骤执行

```cpp
StepResult FlowInstance::executeStep(IStep* step, FlowContext& ctx) {
    int timeout = step->config().timeoutMs;
    if (timeout <= 0) {
        return step->executeWithRetry(ctx);  // 无超时，直接执行
    }

    // 有超时：在独立线程执行，主线程等待 timeout ms
    std::promise<StepResult> prom;
    auto fut = prom.get_future();
    std::thread([step, &ctx, &prom](){
        try { prom.set_value(step->executeWithRetry(ctx)); }
        catch (...) { prom.set_value(StepResult::fail("Step threw exception")); }
    }).detach();

    if (fut.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::timeout) {
        step->cancel();  // 发送取消信号（需要步骤实现 cancel()）
        return StepResult::timeout("Step timeout: " + step->name());
    }
    return fut.get();
}
```

**注意**: 超时后 detached 线程仍在运行（step->cancel() 只是发送信号，不保证立即终止），这是工业设备的常见权衡。

### 9.3 Scheduler — 调度引擎核心

```cpp
class Scheduler {
public:
    // 流程注册（非线程安全，启动前调用）
    void registerFlow(std::shared_ptr<FlowDefinition> def);

    // 提交请求（线程安全，运行时调用）
    std::string submit(SubmitRequest req);  // 返回 instanceId（可用于后续控制）

    // 控制（线程安全）
    void pause(const std::string& instanceId);
    void resume(const std::string& instanceId);
    void abort(const std::string& instanceId);

    // 人工介入恢复（五种模式）
    void handleRecovery(const RecoveryRequest& req);

    // 全局控制
    void stopAll();       // 优雅停机（等待当前步骤）
    void emergencyStop(); // 紧急停机（立即中止）

    // 断电恢复
    void recoverFromPersistence();

private:
    void scheduleLoop();      // 调度线程主循环
    void tryScheduleNext();   // 尝试调度下一个待执行请求

    // 数据
    std::unordered_map<std::string, std::shared_ptr<FlowDefinition>> m_registry_;   // 注册的流程定义
    std::unordered_map<std::string, std::shared_ptr<FlowInstance>>   m_running_;    // 运行中实例
    std::vector<PendingRequest>                                       m_waitQueue_;  // 等待队列

    std::unique_ptr<ResourceLocker>    m_resourceLocker_;  // 设备资源锁
    std::shared_ptr<IDispatchStrategy> m_strategy_;        // 调度策略（可替换）
    std::shared_ptr<IFlowPersistence>  m_persistence_;     // 持久化（可选）
    std::shared_ptr<EventBus>          m_eventBus_;

    int m_maxConcurrent_{4};          // 最大并发实例数
    std::atomic<bool> m_schedulerRunning_{true};
    std::thread m_scheduleThread_;    // 调度循环线程
};
```

### 9.4 调度循环详解

```
scheduleLoop() [独立线程]
    │
    └── while m_schedulerRunning_:
          wait_for(50ms) OR cv.notify (有新请求或实例完成时唤醒)
          │
          if !waitQueue.empty():
              tryScheduleNext()
                  │
                  ├── 检查并发上限：running.size() >= maxConcurrent → return
                  │
                  ├── strategy.selectNext(waitQueue, running)
                  │     → PriorityFIFOStrategy: max_element by (priority DESC, submitTime ASC)
                  │
                  ├── 从 waitQueue 中移除选中的请求
                  │
                  ├── 检查实例上限（def.maxInstances）
                  │     超过 → 放回 waitQueue，return
                  │
                  ├── 申请资源锁（resourceLocker.tryAcquireAll）
                  │     超时失败 → 放回 waitQueue，return（资源不可用，等待下次）
                  │
                  ├── 创建 FlowInstance
                  ├── 注册 onCompleted 回调（→ onInstanceCompleted）
                  ├── 持久化保存实例信息
                  └── instance.start() ← 启动运行线程
```

### 9.5 ResourceLocker — 防死锁资源锁

```cpp
bool ResourceLocker::tryAcquireAll(const std::string& instanceId,
                                    std::vector<std::string> resources,
                                    int timeoutMs) {
    // 关键：固定排序防止死锁（所有请求者按相同顺序加锁）
    std::sort(resources.begin(), resources.end());
    resources.erase(std::unique(...), resources.end());

    auto deadline = now() + milliseconds(timeoutMs);
    unique_lock lk(m_mutex_);

    for (const auto& res : resources) {
        auto ok = m_cv_.wait_until(lk, deadline, [&]{ return m_locks_.count(res) == 0; });
        if (!ok) {
            // 超时：回滚已申请的资源（避免部分持有）
            for (const auto& r2 : resources) {
                auto it = m_locks_.find(r2);
                if (it != m_locks_.end() && it->second == instanceId)
                    m_locks_.erase(it);
                else break;
            }
            return false;
        }
        m_locks_[res] = instanceId;
    }
    return true;
}
```

**防死锁原理**:
- 所有请求者按**字典序**申请资源
- 若 A 需要 [cam, robot]，B 也需要 [cam, robot]，两者都先申请 cam，后申请 robot
- 不会出现 A 持有 cam 等 robot、B 持有 robot 等 cam 的死锁情形

### 9.6 RecoveryMode — 人工介入恢复模式

```cpp
enum class RecoveryMode {
    RetryCurrentStep,       // 重试当前步骤（步骤恢复正常后继续）
    SkipCurrentStep,        // 跳过当前步骤（人工确认可跳过）
    ResumeFromStep,         // 从指定步骤重新开始（跳过中间步骤）
    ResumeFromCheckpoint,   // 从最近检查点恢复（变量也恢复到快照时的值）
    AbortFlow,              // 中止流程并执行补偿
};
```

```cpp
void Scheduler::handleRecovery(const RecoveryRequest& req) {
    auto inst = getRunningInstance(req.instanceId);
    switch (req.mode) {
    case RetryCurrentStep:
        inst->resume();  // 直接恢复，不修改步骤指针
        break;
    case SkipCurrentStep:
        inst->skipCurrentStep();  // 设置 skipFlag
        inst->resume();
        break;
    case ResumeFromStep:
        inst->resumeFromStep(req.targetStepName);  // 设置 skipUntilStep_
        inst->resume();
        break;
    case ResumeFromCheckpoint:
        auto snap = persistence.loadLastCheckpoint(req.instanceId);
        inst->restoreFromCheckpoint(*snap);  // 恢复 context + skipUntilStep_
        inst->resume();
        break;
    case AbortFlow:
        inst->abort();  // 触发补偿事务
        break;
    }
}
```

---

## 10. 事件总线

**文件**: `event/event_bus.h/cpp`

### 10.1 事件类型枚举

```cpp
enum class SchedulerEventType {
    // 流程事件
    FlowSubmitted,   // 提交到等待队列
    FlowStarted,     // 开始运行
    FlowCompleted,   // 正常完成
    FlowFailed,      // 失败
    FlowAborted,     // 中止（补偿完成）
    FlowPaused,      // 暂停
    FlowResumed,     // 恢复
    FlowRecovered,   // 断电恢复

    // 步骤事件
    StepStarted, StepCompleted, StepFailed,
    StepSkipped, StepRetrying, StepTimeout,

    // 报警事件
    AlarmRaised, AlarmAcknowledged, AlarmCleared,

    // 系统事件
    EmergencyStop, SafeStop, SystemReady,

    // 人工任务事件
    HumanTaskCreated, HumanTaskCompleted, HumanTaskTimeout,
};
```

### 10.2 EventBus 接口与实现

```cpp
class EventBus {
public:
    // ── 订阅（线程安全，运行时随时可订阅）──────────────────
    SubscriberId subscribe(SchedulerEventType type, Handler handler);
    SubscriberId subscribeAll(Handler handler);
    void         unsubscribe(SubscriberId id);

    // ── 同步发布（直接在调用线程触发所有订阅者）────────────
    void publish(SchedulerEvent evt);

    // ── 异步发布（投入队列，后台分发线程触发订阅者）────────
    void publishAsync(SchedulerEvent evt);

private:
    void dispatchLoop();  // 异步分发线程

    mutable std::shared_mutex m_subMutex_;      // 订阅者列表读写锁
    std::vector<Subscriber>   m_subscribers_;   // 订阅者列表

    std::queue<SchedulerEvent>  m_asyncQueue_;  // 异步事件队列
    std::mutex                  m_queueMutex_;
    std::condition_variable     m_cv_;
    std::thread                 m_dispatchThread_;
};
```

### 10.3 发布模式对比

| 模式 | 调用方式 | 执行线程 | 适用场景 |
|------|----------|----------|----------|
| `publish()` | 直接调用 | 调用线程 | 订阅者简单、不阻塞 |
| `publishAsync()` | 投入队列 | 独立分发线程 | 避免阻塞调度线程 |

**Scheduler 使用 `publishAsync()`**：保证调度线程不被订阅者回调阻塞。

---

## 11. 报警管理器

**文件**: `alarm/alarm_manager.h/cpp`

### 11.1 报警级别与处置动作

```
AlarmLevel（5级）:
  Info(L0)      → LogOnly         仅记录日志，流程不受影响
  Warning(L1)   → Notify          UI 提示，流程继续
  Fault(L2)     → PauseFlow       暂停当前流程，等待人工处理
  Stop(L3)      → StopAllFlows    优雅停机所有流程
  Emergency(L4) → EmergencyStopAll 立即急停

AlarmAction（5种）:
  LogOnly         仅日志
  Notify          通知 UI
  PauseFlow       暂停当前流程
  StopAllFlows    停止所有流程
  EmergencyStopAll 急停
```

### 11.2 AlarmManager 核心接口

```cpp
class AlarmManager {
public:
    // 注册报警规则（启动前配置）
    void registerAlarm(int code, AlarmLevel level, AlarmAction action,
                       std::string message);

    // 触发报警（线程安全）
    void raise(int code,
               const std::string& instanceId = "",
               const std::string& stepName   = "",
               json ctx = {});

    // 确认（人工操作，记录操作员）
    void acknowledge(int code, const std::string& operatorId = "");

    // 清除报警
    void clear(int code);
    void clearAll();

    // 注入处置回调（由 MachineFSM 设置）
    void onActionRequired(std::function<void(AlarmAction, const AlarmInfo&)> cb);
};
```

### 11.3 报警处理流程

```
raise(code, instanceId, stepName)
    │
    ├── 查找注册规则（未注册 → 忽略）
    ├── 构建 AlarmInfo（timestamp = now()）
    ├── m_active_[code] = info（激活报警）
    ├── publish AlarmRaised 事件（通知 EventBus）
    ├── 如果有 raisedCb_ → 通知订阅者
    └── 如果有 actionCb_（MachineFSM 注入）:
          actionCb_(action, info)  → MachineFSM 执行对应动作
              ├── PauseFlow      → scheduler.pause(instanceId)
              ├── StopAllFlows   → scheduler.stopAll()
              └── EmergencyStopAll → scheduler.emergencyStop()
```

---

## 12. 补偿事务管理器

**文件**: `compensation/compensation_manager.h/cpp`

### 12.1 CompensationManager — LIFO 补偿栈

```cpp
class CompensationManager {
public:
    // 步骤完成时调用，将补偿步骤压栈
    void recordCompleted(const std::string& stepName,
                          std::shared_ptr<IStep> compensationStep);

    // 中止时按 LIFO 顺序执行所有补偿步骤
    void compensateAll(FlowContext& ctx);

    // 从指定步骤之后的补偿（用于部分回滚）
    void compensateFrom(const std::string& fromStep, FlowContext& ctx);

    // 正常完成时清空（无需补偿）
    void clear();

    // 获取已完成步骤名（用于断点快照）
    std::vector<std::string> getCompletedStepNames() const;

private:
    using StackEntry = std::pair<std::string, std::shared_ptr<IStep>>;
    std::vector<StackEntry> m_stack_;  // LIFO 栈
    mutable std::mutex      m_mutex_;
};
```

### 12.2 补偿事务执行流程

**正向执行**（步骤完成时）:
```
步骤"取料"完成 → recordCompleted("取料", releaseStep)  → 栈: [("取料", releaseStep)]
步骤"放置"完成 → recordCompleted("放置", removeStep)   → 栈: [("取料",...), ("放置",...)]
步骤"检测"失败 → 触发 abort()
```

**反向补偿**（`abort()` 时）:
```
compensateAll(ctx):
  栈 (LIFO) = [("取料", releaseStep), ("放置", removeStep)]
  执行顺序：
    1. removeStep->executeWithRetry(ctx)  ← "从检测台移除"
    2. releaseStep->executeWithRetry(ctx) ← "机械臂放回物料"
  clear()
```

**正常完成时**: `compensationMgr_.clear()` 清空栈，不执行任何补偿

---

## 13. 整机主状态机

**文件**: `machine/machine_fsm.h/cpp`

### 13.1 MachineState — 整机状态枚举

```cpp
enum class MachineState {
    Initializing,    // 上电后初始化（自检、设备初始化）
    Ready,           // 就绪（可启动自动运行）
    ManualMode,      // 手动模式（人工干预，步进操作）
    Running,         // 自动运行中（流程正在执行）
    Stopping,        // 优雅停机中（等待当前步骤完成）
    Stopped,         // 已停止（所有流程已安全停止）
    AlarmStop,       // 报警停机（等待人工清除报警后恢复）
    EmergencyStop,   // 急停（安全门打开/急停按钮按下）
    Error,           // 系统错误（初始化失败）
};
```

### 13.2 MachineEvent — 整机事件枚举

```cpp
enum class MachineEvent {
    InitOk,           // 初始化成功
    InitFail,         // 初始化失败
    StartBtn,         // 启动按钮
    StopBtn,          // 停止按钮
    ResetBtn,         // 复位按钮
    EstopTrigger,     // 急停触发（常闭触点断开）
    EstopRelease,     // 急停释放
    AutoSwitch,       // 切换到自动模式
    ManualSwitch,     // 切换到手动模式
    AlarmStop,        // L3 级报警停机
    AllFlowsStopped,  // 所有流程已停止（用于 Stopping → Stopped 转移）
    EmergencyAction,  // L4 级急停触发
};
```

### 13.3 状态转移图

```
               InitOk
Initializing ─────────► Ready ◄──────────── ResetBtn（从 AlarmStop/Stopped）
      │                   │
      │ InitFail          │ StartBtn / AutoSwitch
      ▼                   ▼
    Error              Running ─── StopBtn ──► Stopping ── AllFlowsStopped ──► Stopped
                          │                                                       │
                          │ AlarmStop(L3)                              ResetBtn   │
                          ▼                                             ──────────┘
                       AlarmStop ◄──────────────────────────────────────────────
                          │
                          │ EstopTrigger / EmergencyAction(L4)
                          ▼
                     EmergencyStop
                          │
                          │ EstopRelease + ResetBtn
                          ▼
                        Ready
```

### 13.4 各状态入口动作（onEnter*）

```cpp
void MachineFSM::onEnterRunning() {
    // 通知 EventBus 系统就绪
    // 允许 Scheduler 开始处理待提交的流程
}

void MachineFSM::onEnterStopping() {
    m_scheduler_->stopAll();  // 优雅停机（等待当前步骤完成）
    // 当所有流程停止后触发 AllFlowsStopped 事件 → 转移到 Stopped
}

void MachineFSM::onEnterAlarmStop() {
    m_scheduler_->stopAll();  // 停止所有流程
    // 等待操作员确认报警 + 按下 ResetBtn 后恢复
}

void MachineFSM::onEnterEmergencyStop() {
    m_scheduler_->emergencyStop();  // 立即中止（不等待当前步骤完成）
}
```

### 13.5 AlarmManager 与 MachineFSM 的联动

```cpp
// 在系统初始化时建立联动
alarmMgr->onActionRequired([machine](AlarmAction action, const AlarmInfo& info) {
    switch (action) {
    case AlarmAction::PauseFlow:
        // 直接暂停触发报警的流程实例
        scheduler->pause(info.instanceId);
        break;
    case AlarmAction::StopAllFlows:
        machine->sendEvent(MachineEvent::AlarmStop);  // 触发状态机转移
        break;
    case AlarmAction::EmergencyStopAll:
        machine->sendEvent(MachineEvent::EmergencyAction);
        break;
    }
});
```

---

## 14. 多工位同步

**文件**: `station/station_manager.h/cpp`, `step/sync/station_barrier.h`

### 14.1 Station — 单工位抽象

```cpp
class Station {
public:
    void startFlow(json params = {}) {
        // 向 Scheduler 提交工位主流程
        SubmitRequest req;
        req.flowName = m_config_.mainFlowName;
        req.params   = std::move(params);
        m_currentInstanceId_ = m_scheduler_->submit(req);
    }
    void stopFlow()   { m_scheduler_->abort(m_currentInstanceId_); }
    void pauseFlow()  { m_scheduler_->pause(m_currentInstanceId_); }
    void resumeFlow() { m_scheduler_->resume(m_currentInstanceId_); }
};
```

### 14.2 StationManager — 多工位管理器

```cpp
class StationManager {
public:
    void addStation(StationConfig config);
    void startAll(json params = {});   // 并行启动所有工位
    void stopAll();
    void pauseAll();
    void resumeAll();

    // 同步屏障工厂
    std::shared_ptr<StationBarrier> createBarrier(
        const std::string& name,
        std::vector<std::string> participantIds);

    std::map<std::string, FlowState> allStationStates() const;
};
```

### 14.3 StationBarrier — 屏障同步原语

```cpp
class StationBarrier {
public:
    // 工位到达屏障点
    void arrive(const std::string& stationId) {
        { lock_guard lk(m_mutex_); m_arrived_.insert(stationId); }
        m_cv_.notify_all();
    }

    // 阻塞等待所有工位到达（超时返回 false）
    bool waitAll(int timeoutMs = 30000) {
        unique_lock lk(m_mutex_);
        return m_cv_.wait_for(lk, milliseconds(timeoutMs),
            [this]{ return m_arrived_.size() >= m_participants_.size(); });
    }

    void reset();  // 重置屏障（下一轮使用）

private:
    std::set<std::string> m_participants_;  // 参与工位集合
    std::set<std::string> m_arrived_;       // 已到达工位集合
};
```

**使用模式**（通过 `BarrierStep`）:

```cpp
// 创建屏障（系统初始化时）
auto barrier = stationMgr->createBarrier("装配同步点", {"st1","st2","st3"});

// 在每个工位的流程中使用 BarrierStep
auto barrierStep = std::make_shared<BarrierStep>(barrier, "st1");
// BarrierStep::execute() 内部调用:
//   barrier->arrive("st1")
//   barrier->waitAll(timeout)  ← 阻塞直到所有工位就绪
```

### 14.4 StationSignalBus — 工位间信号总线

```cpp
class StationSignalBus {  // 单例
public:
    // 发送信号（广播）
    void signal(const std::string& name, const std::string& fromStation = "");

    // 等待信号（阻塞，超时返回 false）
    bool waitSignal(const std::string& name, int timeoutMs = 30000);

    // 消费信号（消费后清除，确保一次性触发）
    void clear(const std::string& name);
};
```

**适用场景**: 工位 A 完成某操作后向工位 B 发送信号，工位 B 等待该信号后继续执行（点对点通知，不需要所有工位同步）。

---

## 15. 持久化层

**文件**: `persistence/i_flow_persistence.h`, `persistence/json_file_persistence.h/cpp`

### 15.1 IFlowPersistence — 持久化接口

```cpp
class IFlowPersistence {
public:
    // 流程实例快照
    virtual void saveInstance(const FlowInstanceSnapshot& snap) = 0;
    virtual void updateInstanceState(const std::string& id, FlowState s) = 0;
    virtual std::vector<FlowInstanceSnapshot> loadAllRunning() const = 0;

    // 检查点（断点快照）
    virtual void saveCheckpoint(const CheckpointSnapshot& snap) = 0;
    virtual std::optional<CheckpointSnapshot>
                 loadLastCheckpoint(const std::string& instanceId) const = 0;

    // 活动日志
    virtual void appendActivityLog(ActivityLog log) = 0;
    virtual std::vector<ActivityLog>
                 queryActivityLogs(const std::string& instanceId,
                                   int limit = 100) const = 0;
};
```

### 15.2 CheckpointSnapshot — 断点快照结构

```cpp
struct CheckpointSnapshot {
    std::string              instanceId;
    std::string              flowName;
    std::string              checkpointId;       // 检查点所在步骤名
    json                     contextData;        // FlowContext 序列化（FLOW 级变量）
    json                     hsmSnapshot;        // HSM 状态快照（用于状态机恢复）
    std::vector<std::string> completedSteps;     // 已完成的步骤列表
    std::vector<std::string> compensationStack;  // 补偿栈（用于恢复补偿能力）
    std::chrono::system_clock::time_point savedAt;

    json toJson() const { ... }                 // 可序列化为 JSON
    static CheckpointSnapshot fromJson(const json& j) { ... }
};
```

### 15.3 断点恢复流程

```
断电前（step.isCheckpoint == true）:
  snapshot = FlowInstance::takeCheckpoint()
    ├── snap.instanceId     = m_instanceId_
    ├── snap.checkpointId   = m_currentStepName_（当前步骤名）
    ├── snap.contextData    = m_context_->serialize()（FLOW 变量）
    └── snap.completedSteps = compensationMgr.getCompletedStepNames()
  persistence.saveCheckpoint(snap)

重启恢复:
  Scheduler::recoverFromPersistence()
    ├── loadAllRunning() → 得到待恢复的 FlowInstanceSnapshot 列表
    │
    └── for each snap:
          instance = new FlowInstance(def)
          checkpoint = persistence.loadLastCheckpoint(snap.instanceId)
          instance.restoreFromCheckpoint(*checkpoint)
            ├── m_context_->deserialize(snap.contextData)  ← 恢复变量
            └── m_skipUntilStep_ = snap.checkpointId       ← 从断点处继续
          m_running_[snap.instanceId] = instance
          instance.start()  ← 运行时 runLoop 会跳过断点之前的步骤
```

---

## 16. FlowBuilder DSL 详解

**文件**: `step/flow_builder.h`

### 16.1 设计原理

`FlowBuilder` 使用**容器栈**（`m_containerStack_`）管理嵌套结构，链式调用通过返回 `*this` 实现。

```
容器栈初始状态:
  stack = [rootSeq]           ← 根节点始终是 SequentialStep

调用 sequential("名称"):
  创建 seqStep → 加入当前栈顶(rootSeq) 的 children
  stack = [rootSeq, seqStep]  ← 压栈，后续步骤加入 seqStep

调用 script("初始化", fn):
  创建 scriptStep → 加入当前栈顶(seqStep) 的 children

调用 end():
  stack.pop()                 ← 出栈，回到父容器
  stack = [rootSeq]

调用 build():
  while stack.size() > 1: stack.pop()  ← 自动关闭未闭合的块
  return m_def_
```

### 16.2 DSL 语法对照

```cpp
auto def = FlowBuilder("流程名")
    // ── 流程元信息 ──────────────────────────────────────────
    .description("流程描述")
    .version(2)
    .priority(5)              // 0-9，越高越优先
    .timeout(30000)           // 流程级超时 30s
    .maxInstances(2)          // 最多 2 个并发实例
    .enableCompensation(true) // 启用补偿事务
    .requiredDevices({"camera", "robot"})  // 需要独占的设备

    // ── 生命周期回调 ─────────────────────────────────────────
    .onStart([](FlowContext& ctx) { /* 初始化 */ })
    .onComplete([](FlowContext& ctx) { /* 清理 */ })
    .onFailed([](FlowContext& ctx) { /* 错误处理 */ })
    .onAbort([](FlowContext& ctx) { /* 中止处理 */ })

    // ── 脚本步骤 ─────────────────────────────────────────────
    .script("步骤名", [](FlowContext& ctx) {
        ctx.set("key", value);
        return StepResult::ok();
    })

    // ── 延时步骤 ─────────────────────────────────────────────
    .delay(500, "预热延时")   // 延时 500ms

    // ── 串行块 ───────────────────────────────────────────────
    .sequential("准备阶段")
        .script("子步骤1", [...])
        .script("子步骤2", [...])
    .end()  // 退出串行块

    // ── 并行块 ───────────────────────────────────────────────
    .parallel("并行检测")
        .script("视觉检测", [...])
        .script("尺寸测量", [...])
    .end()

    // ── 循环块（固定次数）─────────────────────────────────────
    .loop(5, "重复5次")
        .script("单次采样", [](FlowContext& ctx) {
            int i = ctx.tryGet<int>("loop_iteration").value_or(0);
            return StepResult::ok();
        })
    .end()

    // ── 条件循环 ─────────────────────────────────────────────
    .loopWhile([](const FlowContext& ctx) {
        return ctx.tryGet<bool>("need_retry").value_or(false);
    }, "按需重试")
        .script("重试操作", [...])
    .end()

    // ── 条件网关 ─────────────────────────────────────────────
    .gateway("结果判断")   // ExclusiveGateway
        .addStep(makeIf([](const FlowContext& c){ return c.get<int>("score") >= 60; },
                        makeScript("合格", [](FlowContext&){ return StepResult::ok(); })))
    .end()

    // ── 任意步骤 ─────────────────────────────────────────────
    .addStep(myCustomStep)  // 直接加入自定义步骤

    .build();  // 返回 shared_ptr<FlowDefinition>
```

### 16.3 StepBuilder — 单步骤配置链

`addStep()` 后可用 `StepBuilder` 进行额外配置：

```cpp
auto sb = StepBuilder(myStep);
sb.name("步骤名")
  .timeout(3000)        // 超时 3s
  .retry(3, 1000)       // 失败后重试 3 次，间隔 1s
  .skipOnFail(true)     // 失败时跳过（不中止流程）
  .optional(true)       // 可选步骤
  .checkpoint(true)     // 执行后保存检查点
  .compensate(compensationStep);  // 注册补偿步骤
```

---

## 17. 关键设计模式分析

### 17.1 组合模式（Composite Pattern）— IStep 树

```
IStep（抽象组件）
├── SequentialStep（容器组件 Composite）
│   └── children: [IStep, IStep, ...]
├── ParallelStep（容器组件 Composite）
│   └── children: [IStep, IStep, ...]
├── ExclusiveGateway（容器组件 Composite）
│   └── branches: [Branch{guard, IStep}, ...]
├── LoopStep（容器组件 Composite）
│   └── body: IStep
├── ScriptStep（叶子组件 Leaf）
├── DelayStep（叶子组件 Leaf）
└── DeviceStep（叶子组件 Leaf）
```

**优点**: 递归执行结构，可任意深度嵌套；容器步骤对外与叶子步骤接口相同。

### 17.2 策略模式（Strategy Pattern）— IDispatchStrategy

```cpp
class IDispatchStrategy {  // 策略接口
    virtual std::optional<PendingRequest> selectNext(...) const = 0;
};

class PriorityFIFOStrategy : public IDispatchStrategy { ... };  // 默认策略
class DeadlineFirstStrategy : public IDispatchStrategy { ... }; // 自定义扩展

// 在 Scheduler 中注入策略
scheduler.setDispatchStrategy(std::make_shared<DeadlineFirstStrategy>());
```

### 17.3 观察者模式（Observer Pattern）— EventBus

```cpp
// 发布者：FlowInstance / Scheduler / AlarmManager
eventBus->publishAsync({SchedulerEventType::FlowCompleted, ...});

// 订阅者：UI / 监控 / 日志
eventBus->subscribe(SchedulerEventType::FlowCompleted, [](const SchedulerEvent& e) {
    dashboard.updateFlowStatus(e.instanceId, "completed");
});
```

### 17.4 模板方法模式（Template Method）— IStep

```cpp
// IStep 定义了执行框架（executeWithRetry 是模板方法）
StepResult IStep::executeWithRetry(FlowContext& ctx) {
    for (int i = 0; i <= retryCount; ++i) {
        auto result = execute(ctx);  // 调用子类实现的抽象方法
        if (result.isOk()) return result;
        sleep(retryInterval);
    }
    return result;
}

// 子类只需实现 execute()，重试逻辑由父类管理
class MyStep : public IStep {
    StepResult execute(FlowContext& ctx) override { ... }
};
```

### 17.5 工厂方法模式 — FlowBuilder / StepFactory

`FlowBuilder` 是**建造者模式（Builder Pattern）**：将复杂对象的构建与表示分离，提供链式 DSL 接口。

### 17.6 RAII — 资源自动管理

```cpp
// FlowContext::pushStepScope() / popStepScope() 配对使用
// SequentialStep 中的 RAII 风格（实际代码中手动调用）：
ctx.pushStepScope();
auto result = child->executeWithRetry(ctx);
ctx.popStepScope();  // 无论成功失败都弹出

// FlowInstance 析构时确保线程安全退出
FlowInstance::~FlowInstance() {
    m_abortFlag_.store(true);
    m_pauseCv_.notify_all();
    if (m_runThread_.joinable()) {
        if (m_runThread_.get_id() == std::this_thread::get_id())
            m_runThread_.detach();  // 避免线程 join 自身
        else
            m_runThread_.join();
    }
}
```

---

## 18. 核心调用链路全景

### 18.1 流程提交 → 运行链路

```
用户代码:
  scheduler.submit({"流程名", priority:5, params:{...}})
    │
    ├── 生成 requestId (UUID)
    ├── 检查流程是否已注册（未注册 → LOG_WARN，返回 ""）
    ├── PendingRequest{req, submitTime} 加入 m_waitQueue_
    ├── cv.notify_one() → 唤醒调度线程
    └── eventBus->publishAsync(FlowSubmitted)

调度线程 (scheduleLoop / tryScheduleNext):
    │
    ├── 检查并发上限 (running.size() < maxConcurrent)
    ├── strategy.selectNext() → 选出最高优先级的最早提交请求
    ├── 从 waitQueue 移除
    ├── 检查 def.maxInstances（超出 → 放回队列）
    ├── resourceLocker.tryAcquireAll(devices, 100ms)（失败 → 放回队列）
    ├── new FlowInstance(def, eventBus)
    ├── instance.setPersistence(persistence)
    ├── instance.context.initParams = req.params
    ├── instance.onCompleted(→ onInstanceCompleted)
    ├── m_running_[instanceId] = instance
    ├── persistence.saveInstance(snap)
    └── instance.start()

FlowInstance.start():
    ├── setState(Running)
    ├── def.onStart(context)
    ├── eventBus.publishAsync(FlowStarted)
    └── m_runThread_ = thread(runLoop)  ← 启动运行线程

FlowInstance.runLoop() [独立线程]:
    ├── 遍历 steps:
    │     ├── 暂停检查
    │     ├── 断点跳过检查
    │     ├── executeStep(step, ctx)  ← 带超时
    │     └── 记录历史 / 保存检查点
    │
    └── 根据结果：
          Completed → onComplete(ctx) + publish FlowCompleted
          Failed    → onFailed(ctx)  + publish FlowFailed
          Aborted   → performCompensation() + onAbort(ctx) + publish FlowAborted

onInstanceCompleted (调度线程通过回调):
    ├── resourceLocker.releaseAll(instanceId)
    ├── m_running_.erase(instanceId)
    ├── 更新统计计数
    ├── persistence.updateInstanceState()
    └── cv.notify_one() → 唤醒调度线程，处理下一个等待请求
```

### 18.2 报警停机链路

```
设备驱动层（或步骤中）:
  alarmMgr.raise(1003, instanceId, "检测步骤")
    │
    ├── 查找规则：AlarmLevel::Stop, AlarmAction::StopAllFlows
    ├── m_active_[1003] = AlarmInfo{...}
    ├── eventBus.publish(AlarmRaised)
    └── actionCb_(StopAllFlows, info)  ← 调用 MachineFSM 注入的回调

MachineFSM.sendEvent(AlarmStop):
    └── handleEvent(AlarmStop, {})
          └── setState(AlarmStop)
                └── onEnterAlarmStop():
                      scheduler.stopAll()  ← 优雅停机（等待当前步骤完成）

操作员处理后:
  alarmMgr.clearAll()
  machine.uiResetButton()
    └── sendEvent(ResetBtn)
          └── setState(Ready) 或 Running（视前一状态决定）
```

### 18.3 断点恢复链路

```
执行中（step.isCheckpoint == true）:
  result = step.executeWithRetry(ctx)  // 成功
  snapshot = instance.takeCheckpoint()
    └── {instanceId, flowName, checkpointId=currentStepName, contextData=ctx.serialize(), ...}
  persistence.saveCheckpoint(snapshot)

断电/重启:
  Scheduler.recoverFromPersistence():
    ├── persistence.loadAllRunning() → [{instanceId, flowName, state=Running, ...}]
    │
    └── for each snap:
          def = m_registry_[snap.flowName]
          instance = new FlowInstance(def)
          checkpoint = persistence.loadLastCheckpoint(snap.instanceId)
          instance.restoreFromCheckpoint(*checkpoint):
              ├── ctx.deserialize(snap.contextData)  ← 恢复变量
              └── m_skipUntilStep_ = snap.checkpointId  ← 跳过已执行的步骤
          m_running_[snap.instanceId] = instance
          eventBus.publish(FlowRecovered)
          （此时实例已就绪，等待调度线程重新 start）
```

---

## 19. 线程模型与并发分析

### 19.1 线程一览

| 线程 | 创建位置 | 职责 |
|------|----------|------|
| 调度线程 | `Scheduler` 构造函数 | 监听等待队列，创建/启动 FlowInstance |
| 实例运行线程 | `FlowInstance::start()` | 执行步骤序列（每实例一个线程） |
| 步骤超时线程 | `FlowInstance::executeStep()` | detached，执行单个步骤并设置 promise |
| 异步并行线程 | `ParallelStep::execute()` | 并行步骤中每个子步骤一个线程（通过 `executeAsync`） |
| 事件分发线程 | `EventBus` 构造函数 | 异步事件队列的消费者 |
| 状态机事件线程 | `HierarchicalStateMachine::start()` | 处理 HSM 异步事件队列 |
| 指令超时检测线程 | `CommandDispatcher` 构造函数 | 每 100ms 检查待完成指令是否超时 |

### 19.2 锁的使用分析

| 资源 | 锁类型 | 保护范围 |
|------|--------|----------|
| `Scheduler::m_registry_/m_running_/m_waitQueue_` | `std::mutex` | 注册/提交/调度操作 |
| `ResourceLocker::m_locks_` | `std::mutex` + `condition_variable` | 资源锁表 |
| `FlowContext::m_flowData_` | `std::shared_mutex` | FLOW 级变量读写（读多写少） |
| `GlobalVariableStore::m_data_` | `std::shared_mutex` | 全局变量读写 |
| `FlowInstance::m_stepNameMutex_` | `std::mutex` | 当前步骤名（调度线程读，运行线程写） |
| `FlowInstance::m_pauseMutex_/m_pauseCv_` | `std::mutex` + `condition_variable` | 暂停/恢复控制 |
| `EventBus::m_subMutex_` | `std::shared_mutex` | 订阅者列表 |
| `AlarmManager::m_mutex_` | `std::mutex` | 报警规则和活动报警 |
| `CompensationManager::m_mutex_` | `std::mutex` | 补偿栈 |
| `StationBarrier::m_mutex_` | `std::mutex` + `condition_variable` | 屏障同步 |

### 19.3 关键并发安全分析

**FlowInstance 的生命周期安全**:
```cpp
// 问题：onCompleted 回调在 runLoop 线程中调用
// → Scheduler::onInstanceCompleted 中 m_running_.erase(instanceId)
// → shared_ptr 引用计数归零 → FlowInstance 析构
// → 析构函数中 m_runThread_.join() → 线程 join 自身 → 死锁！

// 解决方案：start() 中使用 shared_from_this()
auto self = shared_from_this();
m_runThread_ = std::thread([this, self]{ runLoop(); });
// 线程持有 self（强引用），确保 runLoop 期间 FlowInstance 不会被析构
// runLoop 结束后，self 离开 lambda 作用域，引用计数减 1
// 析构从外部线程发生，join 安全

// 析构函数额外保护：
if (m_runThread_.get_id() == std::this_thread::get_id())
    m_runThread_.detach();  // 如果确实在运行线程内析构，改为 detach
```

**步骤超时的线程泄漏风险**:
```cpp
// executeStep 中：
std::thread([step, &ctx, &prom](){
    prom.set_value(step->executeWithRetry(ctx));
}).detach();  // detached 线程持有 step 和 ctx 的引用

// 问题：如果超时发生后 ctx/step 被销毁，detached 线程访问悬空引用
// 实际上 FlowInstance 的 runLoop 线程在 executeStep 返回后才会继续
// 因此 ctx 在 runLoop 线程存活期间不会被销毁
// 但 step->cancel() 只是发送取消信号，detached 线程可能继续运行一段时间
// → 生产代码中建议 DeviceStep 子类实现可靠的 cancel() 机制
```

---

## 20. 使用示例精读

### 20.1 基础顺序流程（来自 basic_example.cpp）

```cpp
// 1. 创建基础设施
auto eventBus = std::make_shared<EventBus>();
auto scheduler = std::make_shared<Scheduler>(eventBus);

// 2. 订阅事件（用于监控）
eventBus->subscribe(SchedulerEventType::StepStarted, [](const SchedulerEvent& e) {
    std::cout << "步骤开始: " << e.stepName << "\n";
});

// 3. 使用 FlowBuilder DSL 构建流程
auto def = FlowBuilder("采样流程")
    .timeout(10000)
    .onStart([](FlowContext& ctx) {
        ctx.set("start_time", std::chrono::system_clock::now());
    })
    .script("初始化", [](FlowContext& ctx) {
        ctx.set("sample_id", std::string("SN-001"));
        return StepResult::ok();
    })
    .delay(200, "预热延时")   // 200ms 延时
    .script("执行检测", [](FlowContext& ctx) {
        auto id = ctx.get<std::string>("sample_id");  // 读取 FLOW 变量
        ctx.set("result", std::string("PASS"));
        return StepResult::ok();
    })
    .script("上报结果", [](FlowContext& ctx) {
        auto r = ctx.get<std::string>("result");
        return StepResult::ok();
    })
    .build();

// 4. 注册并提交
scheduler->registerFlow(def);
auto instanceId = scheduler->submit({"采样流程", 5, {{"lot", "Lot-2026"}}});
//                                              ↑优先级  ↑JSON参数（initParams）
```

### 20.2 补偿事务（来自 basic_example.cpp）

```cpp
// 创建带补偿的步骤（先手动构建，再用 addStep 加入）
auto grabStep = std::make_shared<ScriptStep>([](FlowContext&) {
    std::cout << "机械臂取料\n";
    return StepResult::ok();
});
StepConfig cfg1{"取料", "control.script"};

// 关键：设置 compensationStep（中止时触发）
auto releaseStep = std::make_shared<ScriptStep>([](FlowContext&) {
    std::cout << "[补偿] 放回物料\n";  // 反向操作
    return StepResult::ok();
});
cfg1.compensationStep = releaseStep;
grabStep->setConfig(cfg1);

// FlowInstance.runLoop() 中：
// 步骤成功 → compensationMgr.recordCompleted("取料", releaseStep)
// 后续步骤失败 → abort() → performCompensation()
//   → releaseStep->executeWithRetry(ctx)  ← 自动执行补偿
```

### 20.3 多工位屏障同步（来自 station_example.cpp）

```cpp
auto stationMgr = std::make_shared<StationManager>(sched, eventBus);

// 创建三工位屏障
auto barrier = stationMgr->createBarrier("装配同步点", {"st1","st2","st3"});

// 工位流程中使用 BarrierStep
auto barrierStep = std::make_shared<BarrierStep>(barrier, "st1");
// 执行时: barrier->arrive("st1") + barrier->waitAll(10s)

// 启动所有工位（并行）
stationMgr->startAll();
// 三个工位独立运行自己的流程
// 到达屏障点时等待，直到 st1、st2、st3 全部到达才继续
// 耗时由最慢的工位（st2: 250ms）决定
```

### 20.4 报警与整机状态机（来自 basic_example.cpp）

```cpp
auto alarmMgr = std::make_shared<AlarmManager>(eventBus);
auto machine  = std::make_shared<MachineFSM>(scheduler, alarmMgr, eventBus);

// 注册报警规则
alarmMgr->registerAlarm(1003, AlarmLevel::Stop, AlarmAction::StopAllFlows, "轴超限");

// 状态变化回调
machine->onStateChanged([](MachineState from, MachineState to) {
    std::cout << machineStateStr(from) << " → " << machineStateStr(to) << "\n";
});

machine->init();           // Initializing → Ready
machine->uiStartButton();  // Ready → Running
// ...正常运行...
alarmMgr->raise(1003);    // 触发 L3 报警 → StopAllFlows → Running → AlarmStop
alarmMgr->clearAll();
machine->uiResetButton(); // AlarmStop → Ready（清除报警后可复位）

machine->uiEstopButton(); // 急停 → EmergencyStop
machine->sendEvent(MachineEvent::EstopRelease);
machine->uiResetButton(); // EmergencyStop → Ready
```

---

## 21. 总结与扩展建议

### 21.1 设计优点总结

| 维度 | 亮点 |
|------|------|
| **可扩展性** | `IStep`、`IDeviceCommand`、`IFlowPersistence`、`IDispatchStrategy` 四大扩展点，新增功能无需修改核心代码 |
| **工业安全** | 五级报警 + 整机状态机 + 补偿事务 + 断点续执行，形成完整工业闭环 |
| **并发安全** | `shared_from_this` 防止 join 死锁；`shared_mutex` 读写分离；固定排序加锁防死锁 |
| **可观测性** | `EventBus` 全量事件、`FlowContext.history` 执行时间线、`CheckpointSnapshot` 状态快照 |
| **低侵入性** | `FlowBuilder DSL` 链式构建，业务代码无需依赖调度框架内部实现 |
| **零外部依赖** | 仅依赖 C++17 标准库 + `nlohmann/json`，移植性强 |

### 21.2 当前局限性与改进建议

| 问题 | 说明 | 建议 |
|------|------|------|
| 步骤超时线程泄漏 | detached 线程在超时后仍持有 ctx 引用 | 改用 `std::stop_token` (C++20) 或步骤级取消令牌 |
| FlowContext 不支持深度 clone | 断点恢复时 `std::any` 中的非序列化类型会丢失 | 为复杂类型提供自定义序列化适配器接口 |
| 无流程版本热更新 | 运行中无法切换到新版本流程定义 | 增加 `FlowDefinition::version` 版本对比逻辑 |
| 单一全局 GlobalVariableStore | 多进程部署时无法共享 | 增加 Redis/共享内存后端适配器 |
| `PriorityFIFOStrategy` 无饥饿保护 | 低优先级流程可能长期得不到调度 | 引入老化机制（等待时间增加等效优先级） |
| 无工作流可视化导出 | 调试时无法直观看到流程结构 | 增加 `FlowDefinition::toDot()` 导出 Graphviz DOT 格式 |
| `parallel_step` 共享 `ctx` 无额外锁 | 多个子步骤并发写 FLOW 级变量时存在竞争 | `FlowContext::m_flowMutex_` 已有 `shared_mutex` 保护，但建议文档化约定 |

### 21.3 扩展示例：自定义 DeviceStep

```cpp
// 1. 定义设备指令
class MoveArmCommand : public IDeviceCommand {
public:
    int dispatch() override {
        m_state_.store(CommandState::DISPATCHED);
        // 调用机械臂 SDK 异步移动
        armDriver_->moveToAsync(m_params_["x"], m_params_["y"], m_params_["z"],
            [this](bool ok, int errCode) {
                CommandResult result;
                result.state    = ok ? CommandState::COMPLETED : CommandState::FAILED;
                result.errorCode = errCode;
                notifyCompleted(result);  // 通知 CommandDispatcher
            });
        return 0;
    }
    int cancel() override {
        armDriver_->stop();
        return 0;
    }
    CommandResult waitForCompletion(int timeoutMs) override {
        // 使用 future/promise 或信号量实现阻塞等待
        ...
    }
};

// 2. 定义步骤
class MoveArmStep : public DeviceStep {
public:
    StepResult execute(FlowContext& ctx) override {
        auto cmd = std::make_shared<MoveArmCommand>();
        cmd->setParams({{"x", 100.0}, {"y", 200.0}, {"z", 50.0}});
        cmd->setTimeout(5000);

        auto fut = dispatcher().submit(cmd);
        auto result = fut.get();  // 等待完成

        if (result.state == CommandState::COMPLETED) {
            ctx.set("arm_position", result.responseData);
            return StepResult::ok();
        }
        return StepResult::fail("机械臂移动失败: " + result.message);
    }
    std::string typeName() const override { return "device.move_arm"; }
};

// 3. 在 FlowBuilder 中使用
auto def = FlowBuilder("搬运流程")
    .addStep([]() {
        auto s = std::make_shared<MoveArmStep>();
        s->setConfig(StepConfig{"移动到取料位", "device.move_arm",
                                5000, 2, 500});  // 5s超时，失败重试2次
        return s;
    }())
    .build();
```

### 21.4 扩展示例：自定义调度策略

```cpp
// 最短作业优先（SJF）策略
class ShortestJobFirstStrategy : public IDispatchStrategy {
public:
    std::optional<PendingRequest> selectNext(
        const std::vector<PendingRequest>& waitQueue,
        const std::unordered_map<std::string, std::shared_ptr<FlowInstance>>& running
    ) const override {
        if (waitQueue.empty()) return std::nullopt;

        // 选择预估时间最短的流程（需要 FlowDefinition 提供预估时间）
        return *std::min_element(waitQueue.begin(), waitQueue.end(),
            [](const PendingRequest& a, const PendingRequest& b) {
                return getEstimatedDuration(a.req.flowName) <
                       getEstimatedDuration(b.req.flowName);
            });
    }
};

scheduler.setDispatchStrategy(std::make_shared<ShortestJobFirstStrategy>());
```

---

*文档生成于 2026-04-01，基于 scheduler v5.0 源码精读分析。*

*覆盖文件：`core/`, `command/`, `step/`, `context/`, `fsm/`, `engine/`, `event/`, `alarm/`, `compensation/`, `machine/`, `station/`, `persistence/` 全部头文件及实现文件。*
