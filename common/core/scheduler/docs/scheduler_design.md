# 自动化设备流程执行调度引擎设计文档

> **模块路径**: `common/core/scheduler`
> **版本**: v5.0（重构整理版）
> **日期**: 2026-03-30
> **适用平台**: Aether Platform V2

---

## 目录

**Part I — 需求与架构**
1. [背景与核心目标](#1-背景与核心目标)
2. [功能需求全景](#2-功能需求全景)
3. [整体架构与分层](#3-整体架构与分层)

**Part II — 核心层设计**
4. [原子指令层（IDeviceCommand）](#4-原子指令层)
5. [步骤与流程控制（IStep / 网关 / 边界事件）](#5-步骤与流程控制)
6. [流程实例与状态机（FlowInstance / HSM）](#6-流程实例与状态机)
7. [流程上下文与变量作用域（FlowContext）](#7-流程上下文与变量作用域)
8. [调度引擎（Scheduler）](#8-调度引擎)
9. [事件总线（EventBus）](#9-事件总线)

**Part III — 工业场景层设计**
10. [整机操作面板与主状态机（MachineFSM）](#10-整机操作面板与主状态机)
11. [报警停机与人工介入（AlarmManager / HumanTask）](#11-报警停机与人工介入)
12. [断点续执行与补偿事务（Checkpoint / Compensation）](#12-断点续执行与补偿事务)
13. [多工位并行与同步（StationManager）](#13-多工位并行与同步)
14. [外部触发与持久化（TriggerManager / Persistence）](#14-外部触发与持久化)

**Part IV — 集成与落地**
15. [与 DeviceManager 集成](#15-与-devicemanager-集成)
16. [配置驱动与 FlowBuilder DSL](#16-配置驱动与-flowbuilder-dsl)
17. [目录结构规划](#17-目录结构规划)
18. [完整工业场景示例（端到端）](#18-完整工业场景示例)

---

## 1. 背景与核心目标

### 1.1 背景

Aether Platform V2 已具备完整的设备抽象层（`DeviceManager`），管理相机、光源、扫码器、运动控制器等多类硬件。但单纯的设备驱动无法满足自动化产线的复杂需求：

- 设备操作需**严格按顺序或条件分支**执行，存在重试、超时、异常三种走向
- 多流程并发时存在**设备资源竞争**，需要防死锁机制
- 报警发生时需要**暂停→人工处理→从断点继续**，不能每次从头重跑
- 多工位（机械臂 + AGV + 视觉 + PLC）需要**并行执行 + 同步等待**
- 设备指令（机械臂移动、AGV 行进）是**异步完成**的，需要回调驱动
- 生产切换时只改配置，**不改代码**
- 系统断电重启后需要**自动恢复中断的流程**

### 1.2 核心目标

| 目标 | 描述 |
|------|------|
| 🎯 **流程编排** | 将多步设备操作编排为可配置的工作流（JSON DSL / 代码 DSL） |
| 🎯 **异步调度** | 设备指令异步下发，回调驱动流程继续，不阻塞调度线程 |
| 🎯 **状态闭环** | 每条流程由 HSM 状态机驱动，生命周期清晰可追溯 |
| 🎯 **工业安全** | 急停、报警停机、断点续执行、补偿回滚，形成完整工业闭环 |
| 🎯 **多工位协同** | 多工位并行执行 + 同步原语（屏障/互斥/信号），支持联动 |
| 🎯 **可观测** | 运行状态快照、活动日志、变量变更记录、可视化导出 |
| 🎯 **可扩展** | 策略注入（设备选择/路径规划/调度算法），无需修改核心代码 |

---

## 2. 功能需求全景

### 2.1 流程控制需求

| 编号 | 功能 | 说明 |
|------|------|------|
| FC-01 | 顺序执行 | 步骤按定义顺序串行执行 |
| FC-02 | 并行执行（And-Split/Join） | 并行网关：所有分支同时启动，全部完成后汇聚 |
| FC-03 | 条件分支（XOR-Split） | 排他网关：仅执行第一个满足条件的分支 |
| FC-04 | 包容分支（OR-Split） | 包容网关：执行所有满足条件的分支并汇聚 |
| FC-05 | 事件网关 | Race：等待第一个到来的事件，触发对应分支 |
| FC-06 | 循环 | 按条件或固定次数重复执行子步骤组 |
| FC-07 | 子流程嵌套 | 步骤可调用其他已注册的独立子流程 |
| FC-08 | 边界错误事件 | 步骤执行失败时跳转到错误处理子流程 |
| FC-09 | 边界超时事件 | 步骤执行超时时跳转到超时处理子流程 |
| FC-10 | 边界信号事件 | 外部信号到来时中断步骤，执行信号处理分支 |
| FC-11 | 补偿事务 | 流程取消时按反向顺序执行已完成步骤的补偿操作 |

### 2.2 指令与步骤需求

| 编号 | 功能 | 说明 |
|------|------|------|
| IS-01 | 原子指令抽象 | 设备操作封装为原子指令（`IDeviceCommand`），统一生命周期管理 |
| IS-02 | 异步下发 | 指令下发后立即返回，设备完成后通过回调/future 通知 |
| IS-03 | 指令超时 | 每条指令有独立超时，超时自动取消并上报 |
| IS-04 | 指令重试 | 失败后按配置的次数和间隔自动重试 |
| IS-05 | 步骤取消 | 支持运行中取消指令，设备侧发送 Stop |
| IS-06 | 步骤钩子 | 全局 `beforeStep` / `afterStep`，用于日志、监控 |
| IS-07 | 人工任务节点 | 流程暂停等待操作员在 UI 确认后继续 |

### 2.3 状态机需求

| 编号 | 功能 | 说明 |
|------|------|------|
| SM-01 | 层级状态机（HSM） | 支持复合状态（父子层级）、正交区域、历史伪状态 |
| SM-02 | 转移规则 | 条件转移（Guard）、无条件转移、事件驱动转移 |
| SM-03 | 进入/退出动作 | 每个状态支持 `onEnter` / `onExit` 回调 |
| SM-04 | 流程生命周期状态机 | `CREATED→QUEUED→RUNNING→PAUSED/COMPLETED/FAILED/ABORTED` |
| SM-05 | 整机主状态机 | 管理启动/停止/复位/急停/手自动的整机状态 |
| SM-06 | 状态快照与恢复 | 序列化状态机状态，用于断点恢复 |
| SM-07 | 可视化导出 | 导出 PlantUML/Mermaid 格式状态图 |

### 2.4 调度器需求

| 编号 | 功能 | 说明 |
|------|------|------|
| SCH-01 | 流程注册与实例化 | 按名称注册流程定义，按需创建隔离的运行实例 |
| SCH-02 | 优先级队列 | 支持 0-9 优先级，高优先级优先调度 |
| SCH-03 | 并发限制 | 配置最大并发流程数，超限进入等待队列 |
| SCH-04 | 设备资源锁 | 按设备名称加互斥锁，防止并发竞争同一设备 |
| SCH-05 | 定时/周期触发 | 按 cron 表达式或固定周期自动启动流程 |
| SCH-06 | 外部消息触发 | PLC 信号、MES HTTP、MQTT 消息触发流程 |
| SCH-07 | 调度策略注入 | 可替换设备选择策略、路径规划策略、优先级策略 |
| SCH-08 | 全局停机 | 等待当前步骤完成后停止所有流程 |
| SCH-09 | 紧急停机 | 立即中止所有流程，向所有设备发送 Stop 指令 |

### 2.5 工业安全需求

| 编号 | 功能 | 说明 |
|------|------|------|
| WS-01 | 物理按钮状态机 | 启动/停止/复位/急停/手自动按钮驱动整机状态 |
| WS-02 | 报警分级 | 5 级报警（信息/警告/可恢复故障/停机故障/急停级），对应不同处置策略 |
| WS-03 | 报警处置动作 | 忽略/记录/暂停流程/停机/急停，可配置 |
| WS-04 | 人工介入恢复 | 报警后支持：重试当前步骤/跳过当前步骤/指定步骤恢复/中止 |
| WS-05 | 断点快照 | 在检查点步骤处自动保存上下文+状态机快照 |
| WS-06 | 从检查点恢复 | 系统重启后，从最近检查点恢复流程，无需从头执行 |
| WS-07 | 补偿回滚 | 流程中止时，已完成步骤按反向顺序执行补偿（归位/关光源等） |

### 2.6 多工位需求

| 编号 | 功能 | 说明 |
|------|------|------|
| MS-01 | 工位独立运行 | 每个工位有独立的 `FlowInstance`，互不干扰 |
| MS-02 | 屏障同步 | 多个工位在同步点等待，全部到达后一起继续 |
| MS-03 | 工位互斥 | 多工位访问共享资源时的互斥锁 |
| MS-04 | 工位信号 | 一个工位完成某阶段后向其他工位发送信号 |
| MS-05 | 共享资源管理 | 跨工位共享的设备（如公共传送带）的访问仲裁 |

### 2.7 持久化与可观测需求

| 编号 | 功能 | 说明 |
|------|------|------|
| PO-01 | 活动日志 | 记录每个步骤的执行结果、耗时、错误码 |
| PO-02 | 变量变更日志 | 记录上下文变量的变更历史（用于问题追溯） |
| PO-03 | 运行时快照 | 定期/在检查点保存流程实例完整状态 |
| PO-04 | 重启恢复 | 系统重启时自动加载上次未完成的流程实例并恢复 |
| PO-05 | 统计信息 | 流程执行次数、成功率、平均耗时 |

---

## 3. 整体架构与分层

### 3.1 分层架构图

```
┌─────────────────────────────────────────────────────────────────────┐
│                     业务层 / UI / 插件层                              │
│   ┌───────────────┐  ┌──────────────────┐  ┌─────────────────────┐  │
│   │  MachineCtrl  │  │  HumanTaskPanel  │  │   AlarmUI / 统计    │  │
│   └───────┬───────┘  └────────┬─────────┘  └──────────┬──────────┘  │
└───────────┼────────────────────┼───────────────────────┼─────────────┘
            │ 控制指令             │ 任务确认               │ 事件订阅
            ▼                    ▼                        ▼
┌─────────────────────────────────────────────────────────────────────┐
│                         调度控制层                                    │
│   ┌──────────────┐   ┌─────────────────┐   ┌────────────────────┐   │
│   │  MachineFSM  │   │    Scheduler    │   │   TriggerManager   │   │
│   │（整机状态机） │   │  （流程调度器）  │   │  （外部触发管理）   │   │
│   └──────────────┘   └────────┬────────┘   └────────────────────┘   │
│   ┌──────────────┐   ┌────────┴────────┐   ┌────────────────────┐   │
│   │ AlarmManager │   │  StationManager │   │  IFlowPersistence  │   │
│   │ （报警管理）  │   │  （工位管理器）  │   │   （持久化层）      │   │
│   └──────────────┘   └─────────────────┘   └────────────────────┘   │
└─────────────────────────────────────┬───────────────────────────────┘
                                      │ 创建 / 驱动
                                      ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        流程执行层                                     │
│   ┌──────────────────────────────────────────────────────────────┐  │
│   │                  FlowInstance（流程实例）                      │  │
│   │  ┌──────────────┐ ┌─────────────────┐ ┌───────────────────┐  │  │
│   │  │  FlowContext  │ │  StateMachine   │ │  CompensationMgr  │  │  │
│   │  │ （变量作用域）│ │  （HSM 状态机）  │ │   （补偿管理器）   │  │  │
│   │  └──────────────┘ └─────────────────┘ └───────────────────┘  │  │
│   └──────────────────────────────────────────────────────────────┘  │
│                                                                      │
│   步骤层（IStep 体系）:                                               │
│   顺序/并行/排他网关/包容网关/事件网关/循环/子流程/边界事件包装/人工任务  │
└──────────────────────────────────────┬──────────────────────────────┘
                                       │ 委托
                                       ▼
┌─────────────────────────────────────────────────────────────────────┐
│                       原子指令层                                      │
│   ┌─────────────────────────────────────────────────────────────┐   │
│   │  CommandDispatcher（指令调度器：异步下发、超时、重试、回调）   │   │
│   └──────────────────────────────┬──────────────────────────────┘   │
│   IDeviceCommand: MoveToCmd / GripCmd / SetIOCmd / NavigateCmd ...   │
└──────────────────────────────────┬──────────────────────────────────┘
                                   │ 调用设备 API
                                   ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      DeviceManager（设备层）                          │
│  Camera | Lamphouse | Scanner | MotionCtrl | PLC/IO | AGV | ...      │
└─────────────────────────────────────────────────────────────────────┘

    ← EventBus 贯穿所有层，解耦状态变化通知 →
```

### 3.2 层次职责

| 层次 | 核心组件 | 职责 |
|------|----------|------|
| **调度控制层** | `Scheduler` / `MachineFSM` / `AlarmManager` | 整机状态管理、流程排队调度、报警处置、外部触发 |
| **流程执行层** | `FlowInstance` / `FlowContext` / `StateMachine` | 驱动状态机、执行步骤序列、管理上下文、补偿事务 |
| **步骤层** | `IStep` 及各子类 | 封装流程控制逻辑（顺序/并行/分支/循环/人工任务等） |
| **原子指令层** | `IDeviceCommand` / `CommandDispatcher` | 封装设备操作指令，管理异步生命周期（下发/超时/重试） |
| **设备层** | `DeviceManager` | 管理物理设备连接和基础 API |
| **事件层** | `EventBus` | 贯穿全层，发布/订阅状态变化事件，解耦通信 |

---

## 4. 原子指令层

> 工业设备指令（机械臂移动、AGV 行进、PLC 输出）通常是**异步完成**的：下发后立即返回，设备执行完成后才回调通知。原子指令层专门管理单条设备指令的完整异步生命周期。

### 4.1 指令状态机

```
CREATED ──(dispatch)──► DISPATCHED ──(ack)──► EXECUTING
                                                  │
                         TIMEOUT ◄──(timeout)─────┤
                         CANCELLED ◄──(cancel)────┤
                         FAILED ◄──(error)─────────┤
                         COMPLETED ◄──(done)────────┘
```

### 4.2 IDeviceCommand — 原子指令接口

```cpp
enum class CommandState {
    CREATED, DISPATCHED, EXECUTING, COMPLETED, FAILED, TIMEOUT, CANCELLED
};

struct CommandResult {
    CommandState    state;
    int             errorCode{0};
    std::string     message;
    nlohmann::json  responseData;   // 设备回包数据
    int64_t         durationMs{0};  // 指令耗时
};

struct RetryPolicy {
    int maxRetries{0};          // 最大重试次数
    int retryIntervalMs{500};   // 重试间隔
};

class IDeviceCommand {
public:
    virtual ~IDeviceCommand() = default;

    const std::string& commandId()   const { return m_commandId_; }
    const std::string& commandType() const { return m_type_; }
    CommandState       state()        const { return m_state_; }

    // ── 核心接口（子类实现）─────────────────────────────────
    virtual int dispatch()  = 0;    // 下发到设备（立即返回）
    virtual int cancel()    = 0;    // 取消（若设备支持）

    // ── 等待完成（阻塞，或设置回调后非阻塞）─────────────────
    virtual CommandResult waitForCompletion(int timeoutMs = 0) = 0;
    void onCompleted(std::function<void(const CommandResult&)> cb) {
        m_completedCb_ = std::move(cb);
    }

    void setParams(nlohmann::json params)  { m_params_ = std::move(params); }
    void setTimeout(int ms)                { m_timeoutMs_ = ms; }
    void setRetryPolicy(RetryPolicy p)     { m_retry_ = p; }

protected:
    void notifyCompleted(CommandResult result) {
        m_state_ = result.state;
        if (m_completedCb_) m_completedCb_(result);
    }

    std::string  m_commandId_  = generateUUID();
    std::string  m_type_;
    nlohmann::json m_params_;
    int          m_timeoutMs_{5000};
    RetryPolicy  m_retry_;
    std::atomic<CommandState> m_state_{CommandState::CREATED};
    std::function<void(const CommandResult&)> m_completedCb_;
};
```

### 4.3 内置原子指令类型

```
IDeviceCommand
├── MotionCommands（运动控制）
│   ├── MoveAbsCommand       绝对位移：move_abs(axis, target, velocity)
│   ├── MoveRelCommand       相对位移：move_rel(axis, delta, velocity)
│   ├── HomeCommand          回原点：home(axis)
│   └── WaitInPosCommand     等待到位：wait_in_pos(axis, tolerance)
│
├── GripperCommands（夹爪）
│   ├── GripCommand          夹取：grip(force)
│   └── ReleaseCommand       释放：release()
│
├── IOCommands（PLC / IO）
│   ├── SetOutputCommand     设置输出：set_output(addr, value)
│   ├── ReadInputCommand     读取输入：read_input(addr)
│   └── WaitInputCommand     等待信号：wait_input(addr, expect, timeout)
│
├── AGVCommands（AGV）
│   ├── NavigateToCommand    导航：navigate_to(station_id)
│   └── DockCommand          对接：dock(position)
│
└── SensorCommands（传感器）
    ├── TriggerMeasureCommand 触发测量
    └── ReadSensorCommand     读取传感器值
```

### 4.4 CommandDispatcher — 异步指令调度器

```cpp
class CommandDispatcher {
public:
    // 异步提交，返回 future（可 wait 或 then）
    std::future<CommandResult> submit(std::shared_ptr<IDeviceCommand> cmd);

    // 批量提交：parallel=true 全部并发下发
    std::vector<std::future<CommandResult>> submitBatch(
        std::vector<std::shared_ptr<IDeviceCommand>> cmds,
        bool parallel = true);

    // 纯异步，完成时回调（不占用调用线程）
    void submitWithCallback(std::shared_ptr<IDeviceCommand> cmd,
                             std::function<void(const CommandResult&)> cb);

    bool cancel(const std::string& commandId);
    void cancelAll();

    CommandState getState(const std::string& commandId) const;
    int          pendingCount(const std::string& deviceKey) const;

    // 设备驱动层回调（设备完成时调用此接口）
    void onDeviceResponse(const std::string& commandId,
                          nlohmann::json responseData,
                          int errorCode = 0);

private:
    std::unordered_map<std::string, std::shared_ptr<IDeviceCommand>> m_pending_;
    std::unordered_map<std::string, std::promise<CommandResult>>      m_promises_;
    std::mutex   m_mutex_;
    std::thread  m_timeoutChecker_;
};
```

### 4.5 步骤层与指令层的桥接

`DeviceStep` 子类通过 `CommandDispatcher` 发起异步指令：

```cpp
class MoveAbsStep : public DeviceStep {
public:
    StepResult execute(FlowContext& ctx) override {
        auto cmd = std::make_shared<MoveAbsCommand>();
        cmd->setParams({
            {"axis",     m_config_.params.value("axis", "")},
            {"target",   m_config_.params.value("target", 0.0)},
            {"velocity", m_config_.params.value("velocity", 100.0)}
        });
        cmd->setTimeout(m_config_.timeoutMs);
        cmd->setRetryPolicy({m_config_.retryCount, m_config_.retryIntervalMs});

        auto future = commandDispatcher().submit(cmd);
        auto status = future.wait_for(std::chrono::milliseconds(m_config_.timeoutMs + 500));

        if (status != std::future_status::ready) {
            commandDispatcher().cancel(cmd->commandId());
            return StepResult::fail("MoveAbsStep timeout", ALARM_CMD_TIMEOUT);
        }

        auto result = future.get();
        if (result.state != CommandState::COMPLETED)
            return StepResult::fail(result.message, result.errorCode);

        ctx.set("last_move_duration_ms", result.durationMs);
        return StepResult::ok();
    }
    std::string typeName() const override { return "device.motion.moveAbs"; }
};
```

---

## 5. 步骤与流程控制

### 5.1 IStep — 步骤基类接口

```cpp
struct StepResult {
    enum class Status { Success, Failed, Skipped, Timeout };
    Status      status;
    std::string message;
    int         errorCode{0};
    std::any    output;

    static StepResult ok(std::any out = {})           { return {Status::Success, "OK", 0, std::move(out)}; }
    static StepResult fail(std::string msg, int c=-1) { return {Status::Failed, std::move(msg), c}; }
    static StepResult skip()                          { return {Status::Skipped, "Skipped"}; }
};

struct StepConfig {
    std::string     name;
    std::string     type;
    int             timeoutMs{5000};
    int             retryCount{0};
    int             retryIntervalMs{500};
    bool            skipOnFail{false};
    bool            optional{false};
    bool            isCheckpoint{false};        // 是否为断点恢复检查点
    nlohmann::json  params;
    // 补偿步骤（流程中止时执行反向操作）
    std::shared_ptr<IStep> compensationStep;
};

class IStep {
public:
    virtual ~IStep() = default;

    void setConfig(StepConfig cfg)    { m_config_ = std::move(cfg); }
    const StepConfig& config() const  { return m_config_; }

    // 同步执行（主接口，子类必须实现）
    virtual StepResult execute(FlowContext& ctx) = 0;

    // 异步执行（可选，默认用线程包装同步版本）
    virtual std::future<StepResult> executeAsync(FlowContext& ctx) {
        return std::async(std::launch::async,
            [this, &ctx]{ return execute(ctx); });
    }

    virtual void cancel()                {}   // 取消执行中的指令
    virtual void cleanup(FlowContext& ctx) {}  // 执行后清理

    virtual std::string typeName() const = 0;

protected:
    StepConfig m_config_;
};
```

### 5.2 流程控制步骤类型

#### 5.2.1 控制流步骤

```cpp
// ── 顺序步骤（默认，串行执行所有子步骤）──────────────────────────
class SequentialStep : public IStep {
public:
    void addChild(std::shared_ptr<IStep> step);
    StepResult execute(FlowContext& ctx) override;
    std::string typeName() const override { return "control.sequential"; }
};

// ── 并行步骤（And-Split/Join：全部并发，全部完成后汇聚）────────────
class ParallelStep : public IStep {
public:
    void addChild(std::shared_ptr<IStep> step);
    StepResult execute(FlowContext& ctx) override {
        std::vector<std::future<StepResult>> futures;
        for (auto& child : m_children_)
            futures.push_back(child->executeAsync(ctx));
        bool allOk = true;
        for (auto& f : futures) {
            auto r = f.get();
            if (r.status == StepResult::Status::Failed) allOk = false;
        }
        return allOk ? StepResult::ok()
                     : StepResult::fail("ParallelStep: some branches failed");
    }
    std::string typeName() const override { return "control.parallel"; }
};

// ── 排他网关（XOR-Split：仅执行第一个满足条件的分支）──────────────
class ExclusiveGateway : public IStep {
public:
    struct Branch {
        std::function<bool(const FlowContext&)> guard;
        std::shared_ptr<IStep> step;
    };
    void addBranch(Branch b)                     { m_branches_.push_back(std::move(b)); }
    void setDefault(std::shared_ptr<IStep> def)  { m_default_ = def; }

    StepResult execute(FlowContext& ctx) override {
        for (auto& b : m_branches_) {
            if (b.guard(ctx)) return b.step->execute(ctx);
        }
        return m_default_ ? m_default_->execute(ctx) : StepResult::skip();
    }
    std::string typeName() const override { return "gateway.exclusive"; }

private:
    std::vector<Branch>    m_branches_;
    std::shared_ptr<IStep> m_default_;
};

// ── 包容网关（OR-Split：执行所有满足条件的分支并汇聚）──────────────
class InclusiveGateway : public IStep {
public:
    // 同 ExclusiveGateway 的 Branch 结构
    StepResult execute(FlowContext& ctx) override {
        std::vector<std::future<StepResult>> futures;
        for (auto& b : m_branches_) {
            if (b.guard(ctx))
                futures.push_back(b.step->executeAsync(ctx));
        }
        if (futures.empty() && m_default_)
            return m_default_->execute(ctx);
        bool allOk = true;
        for (auto& f : futures) {
            if (f.get().status == StepResult::Status::Failed) allOk = false;
        }
        return allOk ? StepResult::ok()
                     : StepResult::fail("InclusiveGateway: some branches failed");
    }
    std::string typeName() const override { return "gateway.inclusive"; }
};

// ── 事件网关（Race：等待第一个到来的事件，触发对应分支）───────────
class EventGateway : public IStep {
public:
    struct EventBranch { int eventId; std::shared_ptr<IStep> step; };
    void addEventBranch(EventBranch b) { m_branches_.push_back(std::move(b)); }

    StepResult execute(FlowContext& ctx) override;  // 注册监听 → Race → 执行赢家分支
    std::string typeName() const override { return "gateway.event"; }
};

// ── 循环步骤 ────────────────────────────────────────────────────────
class LoopStep : public IStep {
public:
    void setCondition(std::function<bool(const FlowContext&)> cond) { m_cond_ = cond; }
    void setMaxIterations(int n) { m_max_ = n; }
    void setBody(std::shared_ptr<IStep> body)  { m_body_ = body; }

    StepResult execute(FlowContext& ctx) override {
        int iter = 0;
        while ((!m_cond_ || m_cond_(ctx)) && (m_max_ <= 0 || iter < m_max_)) {
            ctx.set("loop_iteration", iter++, VariableScope::STEP);
            auto r = m_body_->execute(ctx);
            if (r.status == StepResult::Status::Failed) return r;
        }
        return StepResult::ok();
    }
    std::string typeName() const override { return "control.loop"; }
};

// ── 子流程步骤 ───────────────────────────────────────────────────────
class SubFlowStep : public IStep {
public:
    void setSubFlowName(std::string name) { m_subFlowName_ = name; }
    StepResult execute(FlowContext& ctx) override;  // 实例化子流程并同步执行
    std::string typeName() const override { return "control.subflow"; }
};

// ── 延时步骤 ─────────────────────────────────────────────────────────
class DelayStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config_.params.value("ms", 0)));
        return StepResult::ok();
    }
    std::string typeName() const override { return "control.delay"; }
};

// ── 等待事件步骤 ─────────────────────────────────────────────────────
class WaitEventStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override;  // 阻塞等待指定 eventId
    std::string typeName() const override { return "control.waitEvent"; }
};
```

#### 5.2.2 边界事件包装器

边界事件附加在**某个步骤上**，当该步骤执行期间满足特定条件时，**中断步骤并跳转到边界处理子流程**：

```
主步骤执行中
    │
    ├── 正常完成 ──────────────────────────────────────► 继续下一步
    ├── 触发错误边界（errorCode 匹配） ────────────────► ErrorHandlerFlow
    ├── 触发超时边界（执行超过 N ms） ─────────────────► TimeoutHandlerFlow
    └── 触发信号边界（收到外部信号） ─────────────────► SignalHandlerFlow
```

```cpp
enum class BoundaryEventType { ERROR, TIMER, SIGNAL, COMPENSATION };

struct BoundaryEventDesc {
    BoundaryEventType       type;
    int                     matchErrorCode{0};  // 0=匹配任意错误码
    int                     timerMs{0};         // TIMER 类型
    int                     signalId{0};        // SIGNAL 类型
    bool                    cancelMainStep{true};
    std::shared_ptr<IStep>  handlerStep;
};

// 用法：将任意步骤包装，附加边界事件
class BoundaryEventWrapper : public IStep {
public:
    BoundaryEventWrapper(std::shared_ptr<IStep>         mainStep,
                          std::vector<BoundaryEventDesc> boundaries);

    StepResult execute(FlowContext& ctx) override {
        // 1. 注册信号边界监听（SIGNAL 类型）
        // 2. 执行主步骤
        // 3. 根据执行结果匹配错误边界
        // 4. 检查信号边界是否触发
        // → 任一边界触发则执行对应 handlerStep
    }
    std::string typeName() const override { return "boundary.wrapper"; }
};
```

#### 5.2.3 人工任务步骤

```cpp
class HumanTaskStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        HumanTaskRequest req;
        req.taskId      = generateUUID();
        req.instanceId  = ctx.instanceId;
        req.stepName    = m_config_.name;
        req.title       = m_config_.params.value("title", "等待人工确认");
        req.formSchema  = m_config_.params.value("formSchema", nlohmann::json{});
        req.timeoutMs   = m_config_.timeoutMs;

        humanTaskManager().submit(req);     // 发布到 UI

        auto result = humanTaskManager().waitForCompletion(
            req.taskId, std::chrono::milliseconds(req.timeoutMs));

        if (!result) return StepResult::fail("HumanTask timeout", ALARM_HUMAN_TASK_TIMEOUT);
        if (result->action == "reject") return StepResult::fail("HumanTask rejected");

        ctx.set("human_task_result",   result->formData);
        ctx.set("human_task_operator", result->operatorId);
        return StepResult::ok();
    }
    std::string typeName() const override { return "human.task"; }
};
```

### 5.3 步骤完整类型体系

```
IStep
├── 控制流
│   ├── SequentialStep          串行（默认）
│   ├── ParallelStep            并行（And-Split/Join）
│   ├── ExclusiveGateway        排他网关（XOR-Split）
│   ├── InclusiveGateway        包容网关（OR-Split/Join）
│   ├── EventGateway            事件网关（Race）
│   ├── LoopStep                循环
│   ├── SubFlowStep             子流程嵌套
│   ├── DelayStep               延时
│   └── WaitEventStep           等待事件
│
├── 边界事件
│   └── BoundaryEventWrapper    [包装任意步骤，附加边界事件]
│
├── 设备操作（DeviceStep）
│   ├── CameraGrabStep          相机抓图
│   ├── LamphouseCtrlStep       光源控制
│   ├── ScannerReadStep         扫码
│   ├── MoveAbsStep             运动控制（绝对位移）
│   ├── MoveRelStep             运动控制（相对位移）
│   ├── HomeStep                运动回原点
│   ├── SetIOStep               PLC IO 输出
│   ├── WaitIOStep              等待 IO 信号
│   ├── AGVNavigateStep         AGV 导航
│   ├── RangefinderStep         测距
│   └── ColorimeterStep         色度计测量
│
├── 交互
│   └── HumanTaskStep           人工任务（暂停等待确认）
│
└── 脚本
    └── ScriptStep              Lambda / 自定义逻辑
```

---

## 6. 流程实例与状态机

### 6.1 流程生命周期状态机

```
                             ┌──────────────────────────────────────────────────┐
                             │                    RUNNING（复合状态）             │
[注册流程定义]               │  ┌─────────┐    ┌──────────┐    ┌──────────────┐  │
       │                     │  │  IDLE   │──► │EXECUTING │──► │  CLEANUP     │  │
       ▼                     │  └─────────┘    └────┬─────┘    └──────────────┘  │
   CREATED ──(submit)──► QUEUED ──(资源就绪)──►│    │ (step fail + retry)        │
                              │              │  │    ▼                            │
                              │              │  │ ┌──────────┐                   │
                              │              └──┤ │ RETRYING │                   │
                              │                 │ └──────────┘                   │
                              └─────────────────────────────────────────────────┘
                                        │pause          │done          │abort
                                        ▼               ▼              ▼
                                     PAUSED         COMPLETED        ABORTED
                                        │
                          (alarm_stop)  │  (alarm + waiting)
                                        ▼
                                  ALARM_PAUSED ──(human_ack)──► RUNNING
```

### 6.2 层级状态机（HSM）设计

```cpp
struct Event {
    int         id;
    std::string name;
    std::any    payload;
    std::chrono::steady_clock::time_point timestamp;

    template<typename T>
    T data() const { return std::any_cast<T>(payload); }
};

using Guard  = std::function<bool(const Event&)>;
using Action = std::function<void(const Event&)>;

struct StateDesc {
    int                  id;
    std::string          name;
    Action               onEnter;
    Action               onExit;
    std::vector<int>     childStates;
    int                  initialChild{-1};
    int                  historyChild{-1};   // 浅历史记录
    bool                 isParallel{false};  // 正交区域
};

struct TransitionDesc {
    int     from;
    int     to;
    int     eventId;        // -1 = 自动（无事件）转移
    Guard   guard;
    Action  action;
    bool    isInternal{false};  // 内部转移（不触发 onEnter/onExit）
};

class IStateMachine {
public:
    virtual void init()  = 0;
    virtual void start() = 0;
    virtual void stop()  = 0;
    virtual bool processEvent(const Event& evt) = 0;
    virtual void postEvent(Event evt) = 0;          // 异步投递

    virtual int              currentState()     const = 0;
    virtual std::string      currentStateName() const = 0;
    virtual bool             isInState(int id)  const = 0;
    virtual std::vector<int> activeStates()     const = 0;

    virtual nlohmann::json takeSnapshot()  const              = 0;
    virtual void           restoreSnapshot(const nlohmann::json&) = 0;
    virtual std::string    exportPlantUML() const              = 0;
};

class HierarchicalStateMachine : public IStateMachine {
public:
    void addState(StateDesc desc);
    void addTransition(TransitionDesc trans);
    void setInitialState(int stateId);
    // ... 实现 LCA 算法的状态进入/退出、历史状态恢复等
};
```

### 6.3 FlowDefinition — 流程定义

```cpp
struct FlowDefinition {
    std::string                      name;
    std::string                      description;
    int                              version{1};
    std::shared_ptr<IStep>           rootStep;
    std::vector<std::string>         requiredDevices;   // 需要加锁的设备
    int                              maxInstances{1};
    int                              priority{0};
    int                              timeoutMs{0};
    bool                             enableCompensation{true};
    std::function<void(FlowContext&)> onStart;
    std::function<void(FlowContext&)> onComplete;
    std::function<void(FlowContext&)> onFailed;
    std::function<void(FlowContext&)> onAbort;
};
```

### 6.4 FlowInstance — 流程运行实例

```cpp
class FlowInstance {
public:
    explicit FlowInstance(std::shared_ptr<FlowDefinition> def);

    // ── 生命周期控制 ─────────────────────────────────────────
    void start();
    void pause();
    void resume();
    void abort();           // 触发补偿事务后中止

    // ── 状态查询 ─────────────────────────────────────────────
    FlowState          state()      const;
    const FlowContext& context()    const;
    const std::string& instanceId() const;

    // ── 断点恢复 ─────────────────────────────────────────────
    CheckpointSnapshot takeCheckpoint() const;
    void restoreFromCheckpoint(const CheckpointSnapshot& snap);
    void resumeFromStep(const std::string& stepName);

    // ── 事件注入 ─────────────────────────────────────────────
    void injectEvent(Event evt);

    // ── 回调注册 ─────────────────────────────────────────────
    void onStateChanged(std::function<void(FlowState, FlowState)> cb);
    void onStepStarted(std::function<void(const std::string&)> cb);
    void onStepCompleted(std::function<void(const std::string&, StepResult)> cb);
    void onCompleted(std::function<void(bool success)> cb);

private:
    void runLoop();
    StepResult executeStep(IStep* step, FlowContext& ctx);
    void performCompensation();   // 中止时执行补偿

    std::shared_ptr<FlowDefinition>   m_def_;
    std::unique_ptr<FlowContext>      m_context_;
    std::unique_ptr<IStateMachine>    m_hsm_;
    std::unique_ptr<CompensationManager> m_compensationMgr_;
    std::atomic<FlowState>            m_state_{FlowState::Created};
    std::atomic<bool>                 m_abortFlag_{false};
    std::atomic<bool>                 m_pauseFlag_{false};
    std::string                       m_instanceId_;
    std::string                       m_currentStepName_;
};
```

---

## 7. 流程上下文与变量作用域

### 7.1 三级变量作用域

```
┌──────────────────────────────────────────────────────┐
│  Global Scope（全局作用域）                            │
│  跨流程实例共享，生命周期 = 系统运行期间                │
│  用途：批次号、系统参数、跨工位共享数据                  │
│  ┌────────────────────────────────────────────────┐  │
│  │  Flow Scope（流程作用域）                        │  │
│  │  每个 FlowInstance 独立，生命周期 = 流程实例     │  │
│  │  用途：产品 SN、检测结果、步骤间传递数据          │  │
│  │  ┌──────────────────────────────────────────┐  │  │
│  │  │  Step Scope（步骤作用域）                  │  │  │
│  │  │  单步执行期间有效，步骤退出后自动销毁        │  │  │
│  │  │  用途：循环计数器、中间计算结果              │  │  │
│  │  └──────────────────────────────────────────┘  │  │
│  └────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────┘
查找顺序：Step Scope → Flow Scope → Global Scope
```

### 7.2 FlowContext 实现

```cpp
enum class VariableScope { STEP, FLOW, GLOBAL };

class FlowContext {
public:
    // ── 写入（指定作用域，默认 FLOW）─────────────────────────
    template<typename T>
    void set(const std::string& key, T&& value,
             VariableScope scope = VariableScope::FLOW);

    // ── 读取（按 STEP → FLOW → GLOBAL 作用域链查找）──────────
    template<typename T>
    T get(const std::string& key) const;

    template<typename T>
    std::optional<T> tryGet(const std::string& key) const;

    bool has(const std::string& key) const;
    void remove(const std::string& key, VariableScope scope = VariableScope::FLOW);

    // ── 步骤作用域生命周期（由 StepRunner 管理）──────────────
    void pushStepScope();   // 进入步骤
    void popStepScope();    // 退出步骤（自动销毁局部变量）

    // ── 元信息 ────────────────────────────────────────────────
    std::string    flowName;
    std::string    instanceId;
    int            priority{0};
    nlohmann::json initParams;

    // ── 执行历史（用于调试）──────────────────────────────────
    struct StepRecord {
        std::string         name;
        StepResult::Status  status;
        int64_t             durationMs;
        std::string         message;
    };
    std::vector<StepRecord> history;

private:
    // Step scope 栈（支持嵌套）
    std::vector<std::unordered_map<std::string, std::any>> m_stepScopes_;
    // Flow scope
    mutable std::shared_mutex m_flowMutex_;
    std::unordered_map<std::string, std::any> m_flowData_;
};

// 全局变量存储（单例，线程安全）
class GlobalVariableStore {
public:
    static GlobalVariableStore& instance();
    template<typename T> void set(const std::string& key, T&& value);
    template<typename T> T    get(const std::string& key) const;
    template<typename T> std::optional<T> tryGet(const std::string& key) const;
    void remove(const std::string& key);
private:
    mutable std::shared_mutex m_mutex_;
    std::unordered_map<std::string, std::any> m_data_;
};
```

---

## 8. 调度引擎

### 8.1 Scheduler 核心设计

```cpp
// 流程提交请求
struct SubmitRequest {
    std::string     flowName;
    int             priority{0};        // 0-9，越大越高
    nlohmann::json  params;             // 传入 FlowContext.initParams
    std::string     requestId;          // 唯一请求 ID（用于追踪）
};

// 等待队列中的请求
struct PendingRequest : SubmitRequest {
    std::chrono::steady_clock::time_point submitTime;
};

class Scheduler {
public:
    // ── 流程注册 ─────────────────────────────────────────────
    void registerFlow(std::shared_ptr<FlowDefinition> def);
    bool hasFlow(const std::string& name) const;

    // ── 流程提交与控制 ───────────────────────────────────────
    std::string submit(SubmitRequest req);   // 返回 instanceId
    void pause(const std::string& instanceId);
    void resume(const std::string& instanceId);
    void abort(const std::string& instanceId);

    // ── 全局控制 ─────────────────────────────────────────────
    void stopAll();         // 优雅停机（等待步骤完成）
    void emergencyStop();   // 紧急停机（立即中止+发 Stop 指令）

    // ── 策略注入 ─────────────────────────────────────────────
    void setDispatchStrategy(std::shared_ptr<IDispatchStrategy> s);
    void setDeviceSelectionStrategy(std::shared_ptr<IDeviceSelectionStrategy> s);

    // ── 查询 ─────────────────────────────────────────────────
    std::vector<FlowInstanceInfo> runningInstances() const;
    FlowInstanceInfo              getInstanceInfo(const std::string& id) const;
    SchedulerStats                stats() const;

    // ── 持久化（可选）────────────────────────────────────────
    void setPersistence(std::shared_ptr<IFlowPersistence> p);
    void recoverFromPersistence();  // 系统启动时调用

private:
    void scheduleLoop();
    void tryScheduleNext();
    void onInstanceCompleted(const std::string& instanceId, bool success);

    std::unordered_map<std::string,
        std::shared_ptr<FlowDefinition>> m_registry_;
    std::unordered_map<std::string,
        std::shared_ptr<FlowInstance>>   m_running_;
    std::vector<PendingRequest>          m_waitQueue_;

    std::unique_ptr<ResourceLocker>      m_resourceLocker_;
    std::shared_ptr<IDispatchStrategy>   m_dispatchStrategy_;
    std::shared_ptr<IFlowPersistence>    m_persistence_;
    std::shared_ptr<EventBus>            m_eventBus_;

    int         m_maxConcurrent_{4};
    std::mutex  m_mutex_;
    std::thread m_scheduleThread_;
};
```

### 8.2 ResourceLocker — 设备资源锁

防止多个流程同时使用同一设备（死锁预防：固定排序加锁）：

```cpp
class ResourceLocker {
public:
    // 批量申请多个资源（固定顺序排序，防止死锁）
    bool tryAcquireAll(const std::string& instanceId,
                       std::vector<std::string> resources,
                       int timeoutMs = 5000);

    // 释放该实例持有的所有资源
    void releaseAll(const std::string& instanceId);

    // 查询资源被哪个实例持有
    std::optional<std::string> holder(const std::string& resource) const;

private:
    std::unordered_map<std::string, std::string> m_locks_;  // resource -> instanceId
    std::mutex  m_mutex_;
    std::condition_variable m_cv_;
};
```

### 8.3 调度策略接口

```cpp
// 策略1：从等待队列选取下一个调度的请求
class IDispatchStrategy {
public:
    virtual std::optional<PendingRequest> selectNext(
        const std::vector<PendingRequest>&   waitQueue,
        const std::unordered_map<std::string,
            std::shared_ptr<FlowInstance>>& running) const = 0;
};
// 内置：优先级 + FIFO
class PriorityFIFOStrategy : public IDispatchStrategy { ... };

// 策略2：多台同类型设备时，选择哪台执行
class IDeviceSelectionStrategy {
public:
    virtual std::string selectDevice(
        const std::string& deviceType,
        const std::vector<std::string>& candidates,
        const FlowContext& ctx) const = 0;
};
// 内置：轮询 / 最短队列
class RoundRobinStrategy       : public IDeviceSelectionStrategy { ... };
class LeastLoadStrategy        : public IDeviceSelectionStrategy { ... };

// 策略3：运动轴路径规划
class IPathPlanningStrategy {
public:
    virtual std::vector<Waypoint> planPath(
        const Position& from, const Position& to,
        const FlowContext& ctx) const = 0;
};
```

### 8.4 定时触发

```cpp
class TimerTrigger {
public:
    // 固定周期
    void scheduleFixed(const std::string& flowName,
                       int intervalMs,
                       nlohmann::json params = {});

    // Cron 表达式（"0 */1 * * * *" = 每分钟）
    void scheduleCron(const std::string& flowName,
                      const std::string& cronExpr,
                      nlohmann::json params = {});

    void cancel(const std::string& flowName);
    void start();
    void stop();
};
```

---

## 9. 事件总线

### 9.1 EventBus 设计

```cpp
// 调度器/流程引擎层面的事件类型
enum class SchedulerEventType {
    // 调度器事件
    FlowSubmitted, FlowStarted, FlowCompleted, FlowFailed,
    FlowAborted, FlowPaused, FlowResumed, FlowRecovered,
    // 步骤事件
    StepStarted, StepCompleted, StepFailed, StepSkipped,
    StepRetrying, StepTimeout,
    // 报警事件
    AlarmRaised, AlarmAcknowledged, AlarmCleared,
    // 系统事件
    EmergencyStop, SafeStop, SystemReady,
    // 人工任务事件
    HumanTaskCreated, HumanTaskCompleted, HumanTaskTimeout,
};

struct SchedulerEvent {
    SchedulerEventType  type;
    std::string         instanceId;
    std::string         flowName;
    std::string         stepName;
    std::any            payload;
    std::chrono::system_clock::time_point timestamp;
};

class EventBus {
public:
    using SubscriberId = int;
    using Handler = std::function<void(const SchedulerEvent&)>;

    // 订阅指定类型事件
    SubscriberId subscribe(SchedulerEventType type, Handler handler);
    // 订阅所有事件
    SubscriberId subscribeAll(Handler handler);
    void         unsubscribe(SubscriberId id);

    // 同步发布（调用线程直接触发所有订阅者）
    void publish(SchedulerEvent evt);
    // 异步发布（投入队列，由独立线程分发）
    void publishAsync(SchedulerEvent evt);

private:
    std::unordered_multimap<int, std::pair<SubscriberId, Handler>> m_handlers_;
    std::shared_mutex  m_mutex_;
    // 异步队列
    std::queue<SchedulerEvent> m_asyncQueue_;
    std::mutex         m_queueMutex_;
    std::thread        m_dispatchThread_;
    std::condition_variable m_cv_;
};
```

---

## 10. 整机操作面板与主状态机

### 10.1 物理按钮与整机状态

自动化设备操作面板通常有以下按钮，对应整机生命周期的不同状态转移：

| 按钮 | 信号 | 作用 |
|------|------|------|
| 启动（Start）| IO 上升沿 | 从 Ready/Stopped 进入运行 |
| 停止（Stop） | IO 上升沿 | 优雅停机，等待当前步骤完成 |
| 复位（Reset）| IO 上升沿 | 从报警/停止恢复到初始化状态 |
| 急停（E-Stop）| 常闭触点断开 | 立即中止所有流程+设备 Stop |
| 手/自动切换 | IO 电平 | 手动模式：禁止自动流程；自动模式：允许自动调度 |

### 10.2 MachineFSM — 整机主状态机

```
           ┌──────────────────────────────────────────────────────────┐
           │                    整机主状态机                           │
           │                                                          │
  ●───►  INITIALIZING ──(init_ok)──► READY ──(start+auto)──► RUNNING  │
                │                      │                      │   │   │
           (init_fail)             (start+manual)          (stop)(alarm)│
                │                      │                      │   │   │
                ▼                      ▼                      ▼   ▼   │
              ERROR              MANUAL_MODE              STOPPING   │
                                   │   ▲              (steps done)   │
                                   │   │                    │        │
                             (auto_switch)             STOPPED       │
                                                           │         │
                                                       (reset)  ALARM_STOP
                                                           │         │
                                                        READY    (reset)
                                                                     │
                                                                   READY
           │                          │
           │  急停（任意状态下有效）    │
           └──────── E-STOP ──────────┘
                       │
                  EMERGENCY_STOP
                       │
                   (reset_estop)
                       │
                    STOPPED
```

### 10.3 MachineFSM 实现

```cpp
// 整机状态枚举
enum class MachineState {
    Initializing,
    Ready,
    ManualMode,
    Running,
    Stopping,
    Stopped,
    AlarmStop,
    EmergencyStop,
    Error,
};

// 整机事件枚举
enum class MachineEvent {
    InitOk, InitFail,
    StartBtn, StopBtn, ResetBtn,
    EstopTrigger, EstopRelease,
    AutoSwitch, ManualSwitch,
    AlarmStop,              // 报警触发停机
    AllFlowsStopped,        // 所有流程已安全停止
};

class MachineFSM {
public:
    explicit MachineFSM(std::shared_ptr<Scheduler> scheduler,
                         std::shared_ptr<AlarmManager> alarmMgr,
                         std::shared_ptr<EventBus> eventBus);

    void init();
    void sendEvent(MachineEvent evt, std::any payload = {});
    MachineState currentState() const;

    void onStateChanged(std::function<void(MachineState, MachineState)> cb);

private:
    void buildFSM();

    std::unique_ptr<HierarchicalStateMachine> m_hsm_;
    std::shared_ptr<Scheduler>    m_scheduler_;
    std::shared_ptr<AlarmManager> m_alarmMgr_;
    std::shared_ptr<EventBus>     m_eventBus_;
};
```

### 10.4 MachineController — 整机控制中心

```cpp
// MachineController 是业务层与调度层的桥接枢纽
class MachineController {
public:
    MachineController(
        std::shared_ptr<DeviceManager>  dm,
        std::shared_ptr<Scheduler>      scheduler,
        std::shared_ptr<AlarmManager>   alarmMgr,
        std::shared_ptr<EventBus>       eventBus);

    void init();
    void shutdown();

    // 物理 IO 轮询（由 IO 线程调用）
    void onIOChanged(const std::string& address, int value);

    // 手动控制接口（UI 层调用）
    void uiStartButton();
    void uiStopButton();
    void uiResetButton();
    void uiEstopButton();
    void uiManualAutoToggle();

    MachineState currentState() const;

private:
    void ioPollingLoop();    // 定时轮询 IO 信号

    std::shared_ptr<DeviceManager>  m_dm_;
    std::shared_ptr<Scheduler>      m_scheduler_;
    std::shared_ptr<MachineFSM>     m_fsm_;
    std::shared_ptr<AlarmManager>   m_alarmMgr_;
    std::shared_ptr<EventBus>       m_eventBus_;
    std::thread m_ioThread_;
};
```

---

## 11. 报警停机与人工介入

### 11.1 报警级别与处置策略

| 级别 | 名称 | 触发条件示例 | 处置动作 |
|------|------|------------|---------|
| L0 | 信息（Info） | 产品计数到达 | 记录日志，不影响流程 |
| L1 | 警告（Warning） | 相机 NG 率偏高 | 记录+界面提示，流程继续 |
| L2 | 可恢复故障（Fault） | 扫码失败超过重试次数 | 暂停当前流程，等待人工确认 |
| L3 | 停机故障（Stop） | 机械臂运动超时 | 停止所有流程，等待复位 |
| L4 | 急停级（Emergency） | 安全光幕触发 | 立即急停，所有设备 Stop |

```cpp
enum class AlarmLevel { Info, Warning, Fault, Stop, Emergency };

enum class AlarmAction {
    LogOnly,            // 仅记录
    Notify,             // 通知 UI
    PauseFlow,          // 暂停当前流程
    StopAllFlows,       // 停止所有流程
    EmergencyStopAll,   // 立即急停
};

struct AlarmInfo {
    int         code;
    AlarmLevel  level;
    std::string message;
    std::string instanceId;     // 触发报警的流程实例
    std::string stepName;       // 触发报警的步骤
    int         retryCount{0};  // 已重试次数
    std::chrono::system_clock::time_point timestamp;
    nlohmann::json context;     // 附加诊断信息
};

class AlarmManager {
public:
    // 注册报警规则
    void registerAlarm(int code, AlarmLevel level, AlarmAction action,
                       std::string message);

    // 触发报警（由步骤/指令层调用）
    void raise(int code, const std::string& instanceId,
               const std::string& stepName, nlohmann::json ctx = {});

    // 确认报警（操作员处理后调用）
    void acknowledge(int code, const std::string& operatorId);
    void clear(int code);

    std::vector<AlarmInfo> activeAlarms() const;

    void onAlarmRaised(std::function<void(const AlarmInfo&)> cb);

private:
    std::unordered_map<int, AlarmInfo>        m_active_;
    std::unordered_map<int, AlarmLevel>       m_levelMap_;
    std::unordered_map<int, AlarmAction>      m_actionMap_;
    std::shared_ptr<EventBus>                 m_eventBus_;
    mutable std::mutex                        m_mutex_;
};
```

### 11.2 报警处理完整闭环

```
步骤执行失败（errorCode=E-1023）
        │
        ▼
AlarmManager::raise(1023, instanceId, stepName)
        │
        ├── Level=L2(Fault) ──► FlowInstance::pause()
        │                           │
        │                           ▼
        │                   发布 AlarmRaised 事件 ──► UI 弹出报警面板
        │                           │
        │                   操作员看到报警信息 + 4 个选项：
        │                   ① 重试（Retry）     ─► 继续执行当前失败步骤
        │                   ② 跳过（Skip）      ─► 跳过当前步骤继续执行
        │                   ③ 指定步骤（GoTo）  ─► 从指定步骤继续执行
        │                   ④ 中止（Abort）     ─► 触发补偿事务后中止
        │
        └── Level=L3(Stop) ──► MachineFSM::sendEvent(AlarmStop)
                                    │
                                    ▼
                             整机进入 ALARM_STOP 状态
                                    │
                             操作员排查故障 → 按复位按钮
                                    │
                             MachineFSM::sendEvent(Reset)
                                    │
                             重新进入 READY 状态
```

### 11.3 人工介入恢复接口

```cpp
// 操作员在 UI 上的选择
enum class RecoveryMode {
    RetryCurrentStep,   // 重试当前失败的步骤
    SkipCurrentStep,    // 跳过当前步骤
    ResumeFromStep,     // 从指定步骤名称恢复
    ResumeFromCheckpoint, // 从最近检查点恢复
    AbortFlow,          // 中止流程（触发补偿）
};

struct RecoveryRequest {
    std::string  instanceId;
    RecoveryMode mode;
    std::string  targetStepName;  // ResumeFromStep 时有效
};

// Scheduler 提供的恢复接口
void Scheduler::handleRecovery(const RecoveryRequest& req) {
    auto it = m_running_.find(req.instanceId);
    if (it == m_running_.end()) return;
    auto& instance = it->second;

    switch (req.mode) {
    case RecoveryMode::RetryCurrentStep:
        instance->resume();           // 重新执行当前失败步骤
        break;
    case RecoveryMode::SkipCurrentStep:
        instance->skipCurrentStep();  // 标记跳过，继续下一步
        instance->resume();
        break;
    case RecoveryMode::ResumeFromStep:
        instance->resumeFromStep(req.targetStepName);
        break;
    case RecoveryMode::ResumeFromCheckpoint:
        {
            auto snap = m_persistence_->loadLastCheckpoint(req.instanceId);
            if (snap) instance->restoreFromCheckpoint(*snap);
            instance->resume();
        }
        break;
    case RecoveryMode::AbortFlow:
        instance->abort();            // 触发补偿事务
        break;
    }
}
```

---

## 12. 断点续执行与补偿事务

### 12.1 断点快照机制

**检查点（Checkpoint）** 是流程中标记为 `isCheckpoint=true` 的步骤。每次到达检查点时，引擎自动保存快照，用于后续的断点恢复。

```cpp
// 流程实例快照（可序列化为 JSON）
struct CheckpointSnapshot {
    std::string             instanceId;
    std::string             flowName;
    std::string             checkpointId;       // 检查点步骤名
    nlohmann::json          contextData;        // FlowContext 序列化
    nlohmann::json          hsmSnapshot;        // HSM 状态快照
    std::vector<std::string> completedSteps;    // 已完成步骤列表
    std::vector<std::string> compensationStack; // 补偿栈（已完成步骤，反向顺序）
    std::chrono::system_clock::time_point savedAt;
};

// FlowInstance 中的检查点逻辑
void FlowInstance::runLoop() {
    for (auto& step : m_def_->rootStep->children()) {
        // 执行前：压 Step Scope
        m_context_->pushStepScope();

        // 执行
        auto result = executeStep(step.get(), *m_context_);

        // 退出：弹 Step Scope
        m_context_->popStepScope();

        if (result.status == StepResult::Status::Success) {
            // 记录补偿栈
            if (step->config().compensationStep) {
                m_compensationMgr_->recordCompleted(
                    step->config().name,
                    step->config().compensationStep);
            }
            // 自动检查点保存
            if (step->config().isCheckpoint && m_persistence_) {
                auto snap = takeCheckpoint();
                m_persistence_->saveCheckpoint(snap);
                LOG_INFO("[Checkpoint] 已保存检查点: {}", step->config().name);
            }
        }
    }
}
```

### 12.2 从指定步骤恢复

```cpp
void FlowInstance::resumeFromStep(const std::string& stepName) {
    // 跳过 stepName 之前的所有步骤
    m_skipUntilStep_ = stepName;
    m_state_ = FlowState::Running;
    // runLoop 中判断：if (m_skipUntilStep_ && currentStep != *m_skipUntilStep_) skip;
}
```

### 12.3 补偿事务设计

当流程**中止**时，`CompensationManager` 按**反向顺序**执行已完成步骤的补偿操作：

```
正向执行（已完成）：
  Step-1(开光源) → Step-2(运动到位) → Step-3(抓图) → Step-4(测量) ← 在此中止

补偿执行（反向）：
  Step-4 无补偿 ← Step-3 无补偿 ← Step-2: 回原点 ← Step-1: 关光源
```

```cpp
class CompensationManager {
public:
    void recordCompleted(const std::string& stepName,
                         std::shared_ptr<IStep> compensationStep);

    // 中止时反向执行所有补偿
    void compensateAll(FlowContext& ctx);

    // 仅补偿从某步骤之后的操作
    void compensateFrom(const std::string& fromStep, FlowContext& ctx);

private:
    // LIFO 栈：后完成的步骤先补偿
    std::vector<std::pair<std::string, std::shared_ptr<IStep>>> m_stack_;
    std::mutex m_mutex_;
};
```

补偿步骤在 JSON 配置中内联：

```json
{
  "name": "MoveToDetect",
  "type": "device.motion.moveAbs",
  "params": { "axis": "Y", "target": 150.0 },
  "compensation": {
    "name": "ReturnHome",
    "type": "device.motion.moveAbs",
    "params": { "axis": "Y", "target": 0.0, "velocity": 200.0 }
  }
}
```

---

## 13. 多工位并行与同步

### 13.1 多工位拓扑

```
                      ┌────────────────────────────────────────┐
                      │            StationManager               │
                      │                                        │
  Station-1（上料）    │  ┌───────────┐  ┌───────────┐          │
  Station-2（检测）    │  │ Station-1 │  │ Station-2 │  ...    │
  Station-3（下料）    │  │FlowInst A │  │FlowInst B │          │
                      │  └─────┬─────┘  └─────┬─────┘          │
                      │        │               │                │
                      │   同步原语（BarrierStep / SignalStep）    │
                      └────────────────────────────────────────┘
                                        │
                                   Scheduler（共享）
                                        │
                                ResourceLocker（共享设备仲裁）
```

### 13.2 Station — 工位定义

```cpp
struct StationConfig {
    std::string              stationId;
    std::string              name;
    std::vector<std::string> ownedDevices;   // 专属设备
    std::string              mainFlowName;   // 主流程名称
    nlohmann::json           params;
};

class Station {
public:
    const std::string& stationId() const;
    FlowState          flowState()  const;

    void startFlow(nlohmann::json params = {});
    void stopFlow();
    void pauseFlow();
    void resumeFlow();

    const FlowContext& context() const;

private:
    StationConfig                        m_config_;
    std::shared_ptr<FlowInstance>        m_flowInstance_;
    std::shared_ptr<Scheduler>           m_scheduler_;
};
```

### 13.3 同步原语

#### BarrierStep — 屏障（多工位等待点）

所有参与工位都到达屏障后，才一起继续执行：

```cpp
class BarrierStep : public IStep {
public:
    BarrierStep(std::shared_ptr<StationBarrier> barrier,
                const std::string& stationId);

    StepResult execute(FlowContext& ctx) override {
        LOG_INFO("[Barrier] 工位 {} 到达屏障 {}", m_stationId_, m_barrier_->name());
        m_barrier_->arrive(m_stationId_);
        m_barrier_->waitAll();  // 阻塞，直到所有工位都到达
        LOG_INFO("[Barrier] 所有工位就绪，继续执行");
        return StepResult::ok();
    }
    std::string typeName() const override { return "sync.barrier"; }

private:
    std::shared_ptr<StationBarrier> m_barrier_;
    std::string                     m_stationId_;
};

class StationBarrier {
public:
    explicit StationBarrier(std::string name,
                             std::vector<std::string> participantIds);
    void arrive(const std::string& stationId);
    void waitAll(int timeoutMs = 30000);
    void reset();

private:
    std::string              m_name_;
    std::set<std::string>    m_participants_;
    std::set<std::string>    m_arrived_;
    std::mutex               m_mutex_;
    std::condition_variable  m_cv_;
};
```

#### SignalStep / WaitSignalStep — 工位间信号

```cpp
// 发送信号
class SignalStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        stationSignalBus().signal(m_signalName_, ctx.instanceId);
        return StepResult::ok();
    }
    std::string typeName() const override { return "sync.signal"; }
};

// 等待信号
class WaitSignalStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        bool ok = stationSignalBus().waitSignal(
            m_signalName_, std::chrono::milliseconds(m_config_.timeoutMs));
        return ok ? StepResult::ok()
                  : StepResult::fail("WaitSignal timeout: " + m_signalName_,
                                     ALARM_SIGNAL_TIMEOUT);
    }
    std::string typeName() const override { return "sync.waitSignal"; }
};
```

#### MutexStep — 共享资源互斥

```cpp
// 共享资源互斥访问（如公共传送带）
class MutexStep : public IStep {
public:
    StepResult execute(FlowContext& ctx) override {
        std::unique_lock<std::timed_mutex> lock(
            sharedResourceMutex(m_resourceName_),
            std::chrono::milliseconds(m_config_.timeoutMs));
        if (!lock.owns_lock())
            return StepResult::fail("MutexStep timeout: " + m_resourceName_);
        return m_body_->execute(ctx);   // 持有互斥锁执行体步骤
    }
    std::string typeName() const override { return "sync.mutex"; }

private:
    std::string            m_resourceName_;
    std::shared_ptr<IStep> m_body_;
};
```

### 13.4 StationManager

```cpp
class StationManager {
public:
    void addStation(StationConfig config);
    void removeStation(const std::string& stationId);

    Station*       getStation(const std::string& stationId);
    std::vector<Station*> allStations();

    // 批量控制
    void startAll(nlohmann::json params = {});
    void stopAll();
    void pauseAll();
    void resumeAll();

    // 同步原语工厂
    std::shared_ptr<StationBarrier> createBarrier(
        std::string name, std::vector<std::string> participantIds);

    // 监控
    std::map<std::string, FlowState> allStationStates() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Station>> m_stations_;
    std::shared_ptr<Scheduler>    m_scheduler_;
    std::shared_ptr<EventBus>     m_eventBus_;
};
```

---

## 14. 外部触发与持久化

### 14.1 外部触发

工业现场需要响应外部系统的触发信号（PLC 传感器、MES 工单、MQTT 消息）：

```cpp
// 触发事件
struct TriggerEvent {
    std::string    source;       // "PLC_SIGNAL" / "MES_HTTP" / "MQTT"
    std::string    flowName;     // 要触发的流程名称
    nlohmann::json params;
};

// 外部触发器接口
class IExternalTrigger {
public:
    virtual std::string sourceType() const = 0;
    virtual void start() = 0;
    virtual void stop()  = 0;
    void onTriggered(std::function<void(const TriggerEvent&)> cb) {
        m_callback_ = std::move(cb);
    }
protected:
    std::function<void(const TriggerEvent&)> m_callback_;
};

// PLC IO 信号触发器（轮询 IO 地址）
class PlcSignalTrigger : public IExternalTrigger {
    // 配置：{ address: "X0.3", triggerValue: 1,
    //         targetFlow: "ProductionFlow", pollIntervalMs: 20 }
};

// MES HTTP 触发器（监听 HTTP 接口接收工单）
class MesHttpTrigger : public IExternalTrigger {
    // POST /api/start-flow → 解析 flowName + params → 提交到调度器
};

// MQTT 触发器（订阅主题消息）
class MqttTrigger : public IExternalTrigger {
    // 订阅 topic → 解析 payload → 提交到调度器
};

// 触发器管理器（统一管理所有触发器）
class TriggerManager {
public:
    void registerTrigger(std::shared_ptr<IExternalTrigger> trigger) {
        trigger->onTriggered([this](const TriggerEvent& evt) {
            m_scheduler_->submit({evt.flowName, 0, evt.params});
        });
        m_triggers_.push_back(trigger);
        trigger->start();
    }
    void stopAll() { for (auto& t : m_triggers_) t->stop(); }
};
```

### 14.2 持久化层

```cpp
// 活动日志（每步执行记录）
struct ActivityLog {
    std::string     instanceId;
    std::string     flowName;
    std::string     stepName;
    std::string     stepType;
    StepResult::Status status;
    int64_t         durationMs;
    int             errorCode{0};
    std::string     message;
    std::chrono::system_clock::time_point timestamp;
};

// 变量变更日志
struct VarChangeLog {
    std::string   instanceId;
    std::string   key;
    VariableScope scope;
    std::string   oldValue;   // JSON 序列化
    std::string   newValue;
    std::string   changedByStep;
    std::chrono::system_clock::time_point timestamp;
};

// 持久化接口
class IFlowPersistence {
public:
    // 流程实例快照
    virtual void saveInstance(const FlowInstanceSnapshot& snap)    = 0;
    virtual void updateInstanceState(const std::string& id, FlowState s) = 0;
    virtual std::vector<FlowInstanceSnapshot> loadAllRunning() const = 0;

    // 检查点
    virtual void saveCheckpoint(const CheckpointSnapshot& snap)     = 0;
    virtual std::optional<CheckpointSnapshot>
                 loadLastCheckpoint(const std::string& instanceId)  const = 0;

    // 日志
    virtual void appendActivityLog(ActivityLog log)                 = 0;
    virtual void appendVarChangeLog(VarChangeLog log)               = 0;
    virtual std::vector<ActivityLog>
                 queryActivityLogs(const std::string& instanceId,
                                   int limit = 100) const           = 0;
};

// SQLite 实现（轻量级，无需额外部署）
class SqliteFlowPersistence : public IFlowPersistence { ... };
// JSON 文件实现（调试场景）
class JsonFilePersistence    : public IFlowPersistence { ... };
```

### 14.3 系统重启后自动恢复

```cpp
void Scheduler::recoverFromPersistence() {
    if (!m_persistence_) return;

    auto snapshots = m_persistence_->loadAllRunning();
    LOG_INFO("[Scheduler] 发现 {} 个待恢复流程实例", snapshots.size());

    for (auto& snap : snapshots) {
        auto def = m_registry_.find(snap.flowName);
        if (!def) {
            LOG_WARN("[Scheduler] 流程定义不存在，无法恢复: {}", snap.flowName);
            continue;
        }
        auto instance = std::make_shared<FlowInstance>(def->second);

        // 从最近检查点恢复状态
        auto checkpoint = m_persistence_->loadLastCheckpoint(snap.instanceId);
        if (checkpoint) {
            instance->restoreFromCheckpoint(*checkpoint);
            LOG_INFO("[Scheduler] 实例 {} 从检查点 {} 恢复",
                snap.instanceId, checkpoint->checkpointId);
        }

        m_running_[snap.instanceId] = instance;
        m_eventBus_->publish({SchedulerEventType::FlowRecovered,
                               snap.instanceId, snap.flowName});
    }
}
```

---

## 15. 与 DeviceManager 集成

### 15.1 依赖关系与注入

`DeviceStep` 子类通过 IoC 容器（`Container`）获取 `DeviceManager`，避免硬依赖：

```cpp
class DeviceStep : public IStep {
protected:
    // 从容器中解析 DeviceManager
    std::shared_ptr<DeviceManager> deviceManager() const {
        return Container::instance().make<DeviceManager>();
    }

    // 常用设备快捷访问
    std::shared_ptr<common::device::CameraBase>
    camera(const std::string& key) const {
        return deviceManager()->CameraHandler(key);
    }

    std::shared_ptr<common::device::LamphouseBase>
    lamphouse(const std::string& key) const {
        return deviceManager()->LamphouseHandler(key);
    }
    // ... 其他设备
};
```

### 15.2 各设备步骤实现示例

```cpp
// 相机抓图步骤
class CameraGrabStep : public DeviceStep {
public:
    StepResult execute(FlowContext& ctx) override {
        auto cam = camera(m_config_.params.value("cameraKey", ""));
        if (!cam) return StepResult::fail("Camera not found");

        // 构造原子指令
        auto cmd = std::make_shared<CameraGrabCommand>(cam);
        cmd->setTimeout(m_config_.timeoutMs);
        auto result = commandDispatcher().submit(cmd).get();

        if (result.state != CommandState::COMPLETED)
            return StepResult::fail(result.message, result.errorCode);

        // 将抓图结果存入上下文
        ctx.set("grabbed_image", result.responseData["image"]);
        ctx.set("grab_timestamp", result.responseData["timestamp"]);
        return StepResult::ok();
    }
    std::string typeName() const override { return "device.camera.grab"; }
};

// 扫码步骤
class ScannerReadStep : public DeviceStep {
public:
    StepResult execute(FlowContext& ctx) override {
        auto scanner = deviceManager()->ScannerHandler(
            m_config_.params.value("scannerKey", ""));

        auto cmd = std::make_shared<ScannerReadCommand>(scanner);
        cmd->setTimeout(m_config_.timeoutMs);
        auto result = commandDispatcher().submit(cmd).get();

        if (result.state != CommandState::COMPLETED)
            return StepResult::fail("Scanner read failed", result.errorCode);

        auto barcode = result.responseData.value("barcode", "");
        if (barcode.empty()) return StepResult::fail("Barcode empty", -1);

        ctx.set("barcode", barcode);
        return StepResult::ok();
    }
    std::string typeName() const override { return "device.scanner.read"; }
};
```

### 15.3 步骤工厂注册

所有步骤类型通过工厂注册，JSON 中通过 `type` 字段实例化：

```cpp
class StepFactory {
public:
    using Creator = std::function<std::shared_ptr<IStep>()>;

    static StepFactory& instance();

    void registerType(const std::string& typeName, Creator creator);

    std::shared_ptr<IStep> create(const std::string& typeName) const;
};

// 注册所有内置步骤
void registerBuiltinSteps(StepFactory& factory) {
    factory.registerType("control.sequential",  []{ return std::make_shared<SequentialStep>(); });
    factory.registerType("control.parallel",    []{ return std::make_shared<ParallelStep>(); });
    factory.registerType("gateway.exclusive",   []{ return std::make_shared<ExclusiveGateway>(); });
    factory.registerType("gateway.inclusive",   []{ return std::make_shared<InclusiveGateway>(); });
    factory.registerType("gateway.event",       []{ return std::make_shared<EventGateway>(); });
    factory.registerType("control.loop",        []{ return std::make_shared<LoopStep>(); });
    factory.registerType("control.subflow",     []{ return std::make_shared<SubFlowStep>(); });
    factory.registerType("control.delay",       []{ return std::make_shared<DelayStep>(); });
    factory.registerType("control.waitEvent",   []{ return std::make_shared<WaitEventStep>(); });
    factory.registerType("human.task",          []{ return std::make_shared<HumanTaskStep>(); });
    factory.registerType("device.camera.grab",  []{ return std::make_shared<CameraGrabStep>(); });
    factory.registerType("device.scanner.read", []{ return std::make_shared<ScannerReadStep>(); });
    factory.registerType("device.motion.moveAbs",[]{ return std::make_shared<MoveAbsStep>(); });
    // ...
}
```

---

## 16. 配置驱动与 FlowBuilder DSL

### 16.1 JSON 配置示例（完整光学检测流程）

```json
{
  "name": "OpticalInspectionFlow",
  "description": "单工位光学检测流程",
  "version": 2,
  "priority": 1,
  "timeoutMs": 60000,
  "requiredDevices": ["camera_top", "lamphouse_ring", "motion_y"],
  "enableCompensation": true,
  "steps": {
    "type": "control.sequential",
    "name": "MainFlow",
    "children": [
      {
        "type": "device.scanner.read",
        "name": "ScanProductSN",
        "timeoutMs": 3000,
        "retryCount": 2,
        "params": { "scannerKey": "scanner_main" }
      },
      {
        "type": "device.motion.moveAbs",
        "name": "MoveToDetect",
        "timeoutMs": 8000,
        "isCheckpoint": true,
        "params": { "axis": "Y", "target": 150.0, "velocity": 100.0 },
        "compensation": {
          "type": "device.motion.moveAbs",
          "name": "ReturnHome",
          "params": { "axis": "Y", "target": 0.0, "velocity": 200.0 }
        }
      },
      {
        "type": "control.sequential",
        "name": "LightAndGrab",
        "children": [
          {
            "type": "device.lamphouse.control",
            "name": "LightOn",
            "params": { "lamphouseKey": "lamphouse_ring", "brightness": 80 },
            "compensation": {
              "type": "device.lamphouse.control",
              "name": "LightOff",
              "params": { "lamphouseKey": "lamphouse_ring", "brightness": 0 }
            }
          },
          {
            "type": "device.camera.grab",
            "name": "GrabImage",
            "timeoutMs": 3000,
            "params": { "cameraKey": "camera_top" },
            "boundaryEvents": [
              {
                "type": "ERROR",
                "matchErrorCode": 1023,
                "handler": {
                  "type": "human.task",
                  "name": "CameraErrorConfirm",
                  "timeoutMs": 300000,
                  "params": {
                    "title": "相机抓图失败",
                    "description": "相机超时，请检查光源和镜头后确认重试"
                  }
                }
              }
            ]
          },
          {
            "type": "device.lamphouse.control",
            "name": "LightOff",
            "params": { "lamphouseKey": "lamphouse_ring", "brightness": 0 }
          }
        ]
      },
      {
        "type": "gateway.exclusive",
        "name": "JudgeResult",
        "branches": [
          {
            "condition": "ctx.get<std::string>('ng_reason') == ''",
            "step": {
              "type": "control.sequential",
              "name": "PassFlow",
              "children": [
                { "type": "device.io.set", "name": "GreenLight",
                  "params": { "address": "Y0.1", "value": 1 } }
              ]
            }
          }
        ],
        "default": {
          "type": "control.sequential",
          "name": "FailFlow",
          "children": [
            { "type": "device.io.set", "name": "RedLight",
              "params": { "address": "Y0.2", "value": 1 } },
            { "type": "human.task", "name": "ConfirmNG",
              "params": { "title": "NG 品确认", "description": "请处理 NG 品后确认" } }
          ]
        }
      }
    ]
  }
}
```

### 16.2 FlowBuilder DSL（代码方式定义流程）

```cpp
auto flow = FlowBuilder("OpticalInspectionFlow")
    .version(2)
    .priority(1)
    .timeout(60000)
    .requireDevices({"camera_top", "lamphouse_ring", "motion_y"})
    .enableCompensation(true)

    // 步骤1：扫码
    .sequential("MainFlow")
        .step<ScannerReadStep>("ScanProductSN")
            .timeout(3000).retry(2)
            .param("scannerKey", "scanner_main")
        .end()

        // 步骤2：运动到检测位（带补偿：回原点）
        .step<MoveAbsStep>("MoveToDetect")
            .timeout(8000).checkpoint(true)
            .param("axis", "Y").param("target", 150.0).param("velocity", 100.0)
            .compensate<MoveAbsStep>("ReturnHome")
                .param("axis", "Y").param("target", 0.0).param("velocity", 200.0)
            .endCompensate()
        .end()

        // 步骤3：并行预处理（开光源 + 相机准备）
        .parallel("PrepareParallel")
            .step<LamphouseCtrlStep>("LightOn")
                .param("lamphouseKey", "lamphouse_ring").param("brightness", 80)
                .compensate<LamphouseCtrlStep>("LightOff")
                    .param("brightness", 0).endCompensate()
            .end()
            .step<CameraSetParamStep>("SetExposure")
                .param("cameraKey", "camera_top").param("exposure", 10)
            .end()
        .endParallel()

        // 步骤4：抓图（带边界事件：抓图失败→人工确认）
        .step<CameraGrabStep>("GrabImage")
            .timeout(3000)
            .param("cameraKey", "camera_top")
            .onError(1023, FlowBuilder::humanTask("CameraErrorConfirm")
                .title("相机抓图失败")
                .description("请检查光源和镜头后确认重试"))
        .end()

        // 步骤5：条件分支（OK/NG）
        .exclusive("JudgeResult")
            .branch([](const FlowContext& ctx) {
                return ctx.tryGet<std::string>("ng_reason").value_or("").empty();
            }, FlowBuilder::sequential("PassFlow")
                .step<SetIOStep>("GreenLight")
                    .param("address", "Y0.1").param("value", 1)
                .end()
                .build())
            .defaultBranch(FlowBuilder::sequential("FailFlow")
                .step<SetIOStep>("RedLight")
                    .param("address", "Y0.2").param("value", 1)
                .end()
                .step<HumanTaskStep>("ConfirmNG")
                    .param("title", "NG 品确认")
                .end()
                .build())
        .endExclusive()
    .endSequential()

    .build();
```

---

## 17. 目录结构规划

```
common/core/scheduler/
│
├── CMakeLists.txt
│
├── core/                               # 核心公共定义
│   ├── flow_types.h                    # FlowState, FlowDefinition, StepResult 等基础类型
│   ├── event.h                         # Event 结构体
│   └── uuid_gen.h                      # UUID 生成工具
│
├── command/                            # 原子指令层
│   ├── i_device_command.h              # IDeviceCommand 接口 + CommandState + CommandResult
│   ├── command_dispatcher.h/cpp        # CommandDispatcher 异步调度器
│   ├── motion/
│   │   ├── move_abs_command.h/cpp
│   │   ├── move_rel_command.h/cpp
│   │   └── home_command.h/cpp
│   ├── io/
│   │   ├── set_output_command.h/cpp
│   │   └── wait_input_command.h/cpp
│   ├── gripper/
│   │   ├── grip_command.h/cpp
│   │   └── release_command.h/cpp
│   └── agv/
│       └── navigate_command.h/cpp
│
├── step/                               # 步骤层
│   ├── i_step.h                        # IStep 基类接口 + StepConfig + StepResult
│   ├── step_factory.h/cpp              # 步骤工厂（类型注册与实例化）
│   ├── control/                        # 控制流步骤
│   │   ├── sequential_step.h/cpp
│   │   ├── parallel_step.h/cpp
│   │   ├── exclusive_gateway.h/cpp
│   │   ├── inclusive_gateway.h/cpp
│   │   ├── event_gateway.h/cpp
│   │   ├── loop_step.h/cpp
│   │   ├── sub_flow_step.h/cpp
│   │   ├── delay_step.h/cpp
│   │   └── wait_event_step.h/cpp
│   ├── boundary/                       # 边界事件
│   │   └── boundary_event_wrapper.h/cpp
│   ├── device/                         # 设备步骤
│   │   ├── device_step.h               # DeviceStep 基类
│   │   ├── camera_grab_step.h/cpp
│   │   ├── lamphouse_ctrl_step.h/cpp
│   │   ├── scanner_read_step.h/cpp
│   │   ├── move_abs_step.h/cpp
│   │   ├── move_rel_step.h/cpp
│   │   ├── home_step.h/cpp
│   │   ├── set_io_step.h/cpp
│   │   ├── wait_io_step.h/cpp
│   │   └── agv_navigate_step.h/cpp
│   └── human/                          # 人工任务
│       ├── human_task_step.h/cpp
│       ├── human_task_manager.h/cpp
│       └── human_task_types.h
│
├── fsm/                                # 状态机层
│   ├── i_state_machine.h               # IStateMachine 接口
│   ├── hsm.h/cpp                       # HierarchicalStateMachine 实现
│   ├── state_desc.h                    # StateDesc / TransitionDesc
│   └── machine_fsm.h/cpp              # MachineFSM（整机主状态机）
│
├── context/                            # 流程上下文与变量
│   ├── flow_context.h/cpp              # FlowContext（三级作用域）
│   └── global_variable_store.h/cpp    # GlobalVariableStore 单例
│
├── compensation/                       # 补偿事务
│   └── compensation_manager.h/cpp
│
├── engine/                             # 流程执行引擎
│   ├── flow_definition.h               # FlowDefinition 结构体
│   ├── flow_instance.h/cpp             # FlowInstance（含 HSM、Context、Compensation）
│   ├── flow_builder.h/cpp              # FlowBuilder DSL
│   └── checkpoint_snapshot.h          # CheckpointSnapshot 结构体
│
├── scheduler/                          # 调度引擎
│   ├── scheduler.h/cpp                 # Scheduler 主类
│   ├── resource_locker.h/cpp          # ResourceLocker 设备资源锁
│   ├── timer_trigger.h/cpp            # TimerTrigger 定时触发
│   └── strategy/                      # 调度策略接口
│       ├── i_dispatch_strategy.h
│       ├── i_device_selection_strategy.h
│       ├── i_path_planning_strategy.h
│       ├── priority_fifo_strategy.h/cpp
│       ├── round_robin_strategy.h/cpp
│       └── least_load_strategy.h/cpp
│
├── machine/                            # 整机控制层
│   ├── machine_controller.h/cpp       # MachineController（物理按钮→FSM→Scheduler）
│   └── alarm_manager.h/cpp            # AlarmManager（报警分级+处置）
│
├── station/                            # 多工位支持
│   ├── station.h/cpp                   # Station（工位封装）
│   ├── station_manager.h/cpp          # StationManager（工位调度中心）
│   ├── station_barrier.h/cpp          # StationBarrier（屏障同步）
│   ├── station_signal_bus.h/cpp       # 工位间信号总线
│   ├── barrier_step.h/cpp             # BarrierStep
│   ├── signal_step.h/cpp              # SignalStep / WaitSignalStep
│   └── mutex_step.h/cpp               # MutexStep（共享资源互斥）
│
├── event/                              # 事件总线
│   └── event_bus.h/cpp
│
├── trigger/                            # 外部触发
│   ├── i_external_trigger.h
│   ├── trigger_manager.h/cpp
│   ├── plc_signal_trigger.h/cpp
│   ├── mes_http_trigger.h/cpp
│   └── mqtt_trigger.h/cpp
│
├── persistence/                        # 持久化
│   ├── i_flow_persistence.h
│   ├── activity_log.h
│   ├── var_change_log.h
│   ├── sqlite_persistence.h/cpp
│   └── json_file_persistence.h/cpp
│
└── examples/                           # 使用示例
    ├── single_station_example.cpp      # 单工位光学检测示例
    └── multi_station_example.cpp       # 多工位装配线示例
```

---

## 18. 完整工业场景示例

### 18.1 场景描述

**双工位光学检测装配线**：
- **工位 A（上料扫码）**：传送带到位→扫码→等待工位 B 就绪→放行
- **工位 B（光学检测）**：等待工位 A 放行→移动到检测位→开光源抓图→判断 OK/NG→下料

两工位通过**屏障同步**协调，共享传送带资源通过**互斥步骤**保护。

### 18.2 主程序初始化

```cpp
int main() {
    // ── 1. 初始化设备层 ─────────────────────────────────────
    auto dm = std::make_shared<DeviceManager>();
    dm->Init();

    // ── 2. 注册到 IoC 容器 ──────────────────────────────────
    auto& container = Container::instance();
    container.instance<DeviceManager>(dm);

    // ── 3. 创建核心组件 ─────────────────────────────────────
    auto eventBus    = std::make_shared<EventBus>();
    auto alarmMgr    = std::make_shared<AlarmManager>(eventBus);
    auto persistence = std::make_shared<SqliteFlowPersistence>("./flow_data.db");
    auto scheduler   = std::make_shared<Scheduler>(eventBus);
    scheduler->setPersistence(persistence);
    scheduler->setDispatchStrategy(std::make_shared<PriorityFIFOStrategy>());

    // ── 4. 注册报警规则 ─────────────────────────────────────
    alarmMgr->registerAlarm(1001, AlarmLevel::Fault,  AlarmAction::PauseFlow,   "扫码失败超限");
    alarmMgr->registerAlarm(1002, AlarmLevel::Stop,   AlarmAction::StopAllFlows,"运动超时");
    alarmMgr->registerAlarm(1003, AlarmLevel::Warning,AlarmAction::Notify,      "相机曝光异常");
    alarmMgr->registerAlarm(9000, AlarmLevel::Emergency, AlarmAction::EmergencyStopAll, "急停触发");

    // ── 5. 初始化整机控制器 ─────────────────────────────────
    auto machineCtrl = std::make_shared<MachineController>(dm, scheduler, alarmMgr, eventBus);
    machineCtrl->init();

    // ── 6. 注册步骤类型 ─────────────────────────────────────
    registerBuiltinSteps(StepFactory::instance());

    // ── 7. 构建流程定义 ─────────────────────────────────────
    auto stationAFlow = FlowBuilder("StationA_LoadFlow")
        .requireDevices({"scanner_a", "conveyor"})
        // ... 步骤定义见 18.3
        .build();

    auto stationBFlow = FlowBuilder("StationB_InspectFlow")
        .requireDevices({"camera_top", "lamphouse_ring", "motion_y"})
        .enableCompensation(true)
        // ... 步骤定义见 18.4
        .build();

    scheduler->registerFlow(stationAFlow);
    scheduler->registerFlow(stationBFlow);

    // ── 8. 创建工位管理器 ───────────────────────────────────
    auto stationMgr = std::make_shared<StationManager>(scheduler, eventBus);
    stationMgr->addStation({"station_a", "工位A_上料", {"scanner_a"}, "StationA_LoadFlow"});
    stationMgr->addStation({"station_b", "工位B_检测", {"camera_top","motion_y"}, "StationB_InspectFlow"});

    // 创建屏障（A、B 两个工位共同参与）
    auto syncBarrier = stationMgr->createBarrier("AB_Sync", {"station_a", "station_b"});

    // ── 9. 注册外部触发 ─────────────────────────────────────
    auto triggerMgr = std::make_shared<TriggerManager>(scheduler);
    triggerMgr->registerTrigger(std::make_shared<PlcSignalTrigger>(dm,
        PlcSignalConfig{"X0.0", 1, "StationA_LoadFlow", 20}));
    triggerMgr->registerTrigger(std::make_shared<MesHttpTrigger>(8080));

    // ── 10. 系统重启恢复 ────────────────────────────────────
    scheduler->recoverFromPersistence();

    // ── 11. 启动 ────────────────────────────────────────────
    machineCtrl->uiStartButton();    // 等待操作员按启动
    // 或 eventBus 订阅 SystemReady 后自动启动

    // 主循环（实际项目中为 Qt 事件循环或 ROS 节点）
    std::this_thread::sleep_for(std::chrono::hours(24));

    machineCtrl->shutdown();
    dm->Release();
    return 0;
}
```

### 18.3 工位 A 流程（上料扫码）

```cpp
auto stationAFlow = FlowBuilder("StationA_LoadFlow")
    .requireDevices({"scanner_a"})

    .sequential("StationA_Main")

        // Step 1：等待传送带到位信号
        .step<WaitIOStep>("WaitConveyorArrival")
            .timeout(30000)
            .param("address", "X0.1").param("expect", 1)
        .end()

        // Step 2：扫码（互斥访问传送带区域）
        .mutex("ConveyorMutex", "conveyor_zone")
            .step<ScannerReadStep>("ScanSN")
                .timeout(3000).retry(2)
                .param("scannerKey", "scanner_a")
            .end()
        .endMutex()

        // Step 3：等待工位 B 就绪（屏障同步）
        .barrier("AB_Sync", "station_a")    // 到达同步点，等待工位 B

        // Step 4：放行传送带
        .step<SetIOStep>("ReleaseConveyor")
            .param("address", "Y0.1").param("value", 1)
        .end()

        // Step 5：写入 MES（全局变量更新）
        .script("UpdateMES", [](FlowContext& ctx) {
            auto sn = ctx.get<std::string>("barcode");
            GlobalVariableStore::instance().set("last_scan_sn", sn);
            return StepResult::ok();
        })

    .endSequential()
    .build();
```

### 18.4 工位 B 流程（光学检测）

```cpp
auto stationBFlow = FlowBuilder("StationB_InspectFlow")
    .requireDevices({"camera_top", "lamphouse_ring", "motion_y"})
    .enableCompensation(true)

    .sequential("StationB_Main")

        // Step 1：等待工位 A 就绪（屏障同步）
        .barrier("AB_Sync", "station_b")

        // Step 2：运动到检测位（有补偿：回原点）
        .step<MoveAbsStep>("MoveToDetect")
            .timeout(8000).checkpoint(true)
            .param("axis", "Y").param("target", 150.0).param("velocity", 100.0)
            .compensate<MoveAbsStep>("ReturnHome")
                .param("axis", "Y").param("target", 0.0).param("velocity", 200.0)
            .endCompensate()
        .end()

        // Step 3：并行处理（开光源 + 读取上一工位的 SN）
        .parallel("ParallelPrep")
            .step<LamphouseCtrlStep>("LightOn")
                .param("lamphouseKey", "lamphouse_ring").param("brightness", 80)
                .compensate<LamphouseCtrlStep>("LightOff")
                    .param("brightness", 0).endCompensate()
            .end()
            .script("ReadSN", [](FlowContext& ctx) {
                auto sn = GlobalVariableStore::instance()
                    .get<std::string>("last_scan_sn");
                ctx.set("product_sn", sn);
                return StepResult::ok();
            })
        .endParallel()

        // Step 4：抓图（错误边界：相机失败→人工确认）
        .step<CameraGrabStep>("GrabImage")
            .timeout(3000)
            .param("cameraKey", "camera_top")
            .onError(1023, humanTask("CameraErrorConfirm")
                .title("相机抓图失败，请检查后确认"))
        .end()

        // Step 5：关光源
        .step<LamphouseCtrlStep>("LightOff")
            .param("lamphouseKey", "lamphouse_ring").param("brightness", 0)
        .end()

        // Step 6：算法检测（ScriptStep 调用算法库）
        .script("RunAlgorithm", [](FlowContext& ctx) {
            auto img = ctx.get<ImageData>("grabbed_image");
            auto [ok, reason, defects] = AlgorithmLib::inspect(img);
            ctx.set("inspect_ok",     ok);
            ctx.set("ng_reason",      reason);
            ctx.set("defect_regions", defects);
            return StepResult::ok();
        })

        // Step 7：OK/NG 分支处理
        .exclusive("JudgeResult")
            .branch([](const FlowContext& ctx) {
                    return ctx.get<bool>("inspect_ok"); },
                sequential("OK_Flow")
                    .step<SetIOStep>("GreenLight")
                        .param("address", "Y0.1").param("value", 1)
                    .end()
                    .build())
            .defaultBranch(
                sequential("NG_Flow")
                    .step<SetIOStep>("RedLight")
                        .param("address", "Y0.2").param("value", 1)
                    .end()
                    .step<HumanTaskStep>("ConfirmNG")
                        .timeout(300000)
                        .param("title", "NG 品确认")
                        .param("description", "请处理 NG 品后确认继续")
                    .end()
                    .build())
        .endExclusive()

        // Step 8：运动回原位（检查点：保存快照）
        .step<MoveAbsStep>("ReturnToOrigin")
            .timeout(8000).checkpoint(true)
            .param("axis", "Y").param("target", 0.0).param("velocity", 200.0)
        .end()

    .endSequential()
    .build();
```

### 18.5 六大典型工业场景

| # | 场景 | 触发条件 | 引擎行为 |
|---|------|---------|---------|
| 1 | **正常生产** | PLC 到位信号 → TriggerManager | 按流程顺序执行，检查点自动保存快照 |
| 2 | **设备故障（可恢复）** | 扫码失败超过重试次数 | AlarmManager::raise(L2) → 流程暂停 → UI 弹出报警 → 操作员选"重试" → 流程继续 |
| 3 | **急停触发** | 安全光幕触发 → E-Stop IO | MachineFSM → EmergencyStop → CommandDispatcher::cancelAll() → 所有设备发 Stop |
| 4 | **断点续执行** | 系统重启（电源故障后） | recoverFromPersistence() → 从最近检查点（MoveToDetect后）恢复 → 继续执行抓图 |
| 5 | **人工指定步骤恢复** | 操作员选"从第 N 步继续" | handleRecovery(ResumeFromStep) → resumeFromStep("GrabImage") → 跳过前序步骤直接执行 |
| 6 | **流程中止补偿** | 操作员选"中止流程" | FlowInstance::abort() → CompensationManager::compensateAll() → 先关光源，再回原点 |

---

*文档结束 —— Aether Platform V2 · Scheduler Module Design v5.0*
