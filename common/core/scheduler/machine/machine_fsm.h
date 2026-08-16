#pragma once

#include <memory>
#include <functional>
#include <any>
#include <atomic>
#include <string>
#include <mutex>

#include "../scheduler_global.h"
#include "../core/flow_types.h"
#include "../engine/scheduler.h"
#include "../alarm/alarm_manager.h"
#include "../event/event_bus.h"
#include "machine_status.h"

namespace scheduler {

class StationManager;   // 前向声明，避免与 station_manager.h 循环依赖
class MachineStatusNotifier;

// ============================================================
// MachineEvent — 整机状态机事件
//
// 所有外部/内部输入统一经 sendEvent() → handleEvent() 串行处理。
//
// 【事件来源与映射】
//   ButtonMonitor / HomeViewModel → Start/Stop/Reset/ManualSwitch/AutoSwitch/
//                                   EmergencyStop/EstopRelease
//   AlarmManager → StopAllFlows→Alarm；EmergencyStopAll→EmergencyStop
//   EventBus 回调 → 持续接料、复位/急停工步流生命周期、FlowDrained
//
// 【物理按钮 → 事件】（ButtonMonitor / UI 薄封装）
//   启动按钮     → Start      （Paused → Running；合并旧 Resume 语义）
//   停止按钮     → Stop       （Running → Paused，Scheduler pause）
//   复位按钮     → Reset      （启动复位序列）
//   手自动拨钮   → AutoSwitch / ManualSwitch
//   急停断开     → EmergencyStop
//   急停恢复     → EstopRelease（仅记录硬件释放，不清 EmergencyStop 位）
//
// 【AlarmManager 回调 → 事件】
//   StopAllFlows      → Alarm
//   EmergencyStopAll  → EmergencyStop
//
// 【内部事件】（由 EventBus 订阅回调 sendEvent 回灌）
//   FlowDrained     Scheduler setOnIdle 回调（Running+Draining 排空完成）
//   ResetSucceeded  复位工步流成功（FlowCompleted）
//   ResetFailed     复位工步流失败（FlowFailed / 配置错误）
//   ResetAborted    复位被 Stop / 急停 / abort 中止（FlowAborted）
//
// 【handleEvent 全局优先级】（先于主态 switch 处理）
//   1. EmergencyStop — 任意态；硬急停 + submit estopFlow + |= EmergencyStop
//   2. EstopRelease    — 仅日志，不改状态
//   3. ResetSucceeded/Failed/Aborted — 复位结果终态
//   4. Alarm + Resetting — abort 复位工步流
//   5. FlowDrained     — Running+Draining → Paused（可带 ManualMode）
//   6. Stop / Reset / ClearAlarm — 统一守卫（canStop/canReset/canClearAlarm）
//   7. 按 st.main 分派其余事件
//
// 【急停工步流】任意态 + EmergencyStop → 硬急停 + submit estopFlowName
//   工步流完成/失败均保持 EmergencyStop 子标志；仅 Reset 成功可清除并启动
//
// ============================================================
enum class MachineEvent {
    Reset,            ///< 启动整机复位工步流（machine_reset.json）
    Start,            ///< 启动生产；守卫见 canStart()
    Stop,             ///< 暂停；Running→Paused；复位中则 abort 复位流
    Alarm,            ///< L3 报警停机或 Paused 挂 Error 子标志
    ClearAlarm,       ///< 清除报警；Alarm→Paused 或 Paused 清 Error
    EmergencyStop,    ///< 硬急停 + 急停工步流；任意态有效
    EstopRelease,     ///< 急停硬件释放；不清 EmergencyStop 位
    AutoSwitch,       ///< 手自动拨钮 → 自动模式（清 ManualMode）
    ManualSwitch,     ///< 手自动拨钮 → 手动；Running 时先 Draining 排空

    FlowDrained,      ///< 内部：流程排空完成（setOnIdle 回调）
    ResetSucceeded,   ///< 内部：复位工步流 FlowCompleted
    ResetFailed,      ///< 内部：复位工步流 FlowFailed / 提交失败
    ResetAborted,     ///< 内部：复位工步流 FlowAborted / Stop 中止
};

// ============================================================
// ResetFlowConfig — 整机复位工步流（如 machine_reset.json）
//
// 由 SchedulerRuntime::loadFlowDefinitions() 在 FlowLoader 注册后
// 经 setResetFlowConfig() 注入；Reset 按钮触发 performReset() submit。
// ============================================================
struct ResetFlowConfig {
    std::string flowName;              ///< FlowLoader 注册名，如 "machine_reset"
    json        params;                ///< submit 时传入 initParams
    int         priority{9};           ///< 复位优先于产线流程（默认 9）
};

// ============================================================
// EstopFlowConfig — 整机急停工步流（如 machine_estop.json）
//
// EmergencyStop 事件后 performEstopFlow() 提交；与硬急停 emergencyStop() 配合。
// 工步流成败均不改变 EmergencyStop 子标志；须 Reset 成功后才可 Start。
// ============================================================
struct EstopFlowConfig {
    std::string flowName;              ///< FlowLoader 注册名，如 "machine_estop"
    json        params;
    int         priority{10};          ///< 高于复位/产线流程（默认 10）
};

// ============================================================
// ContinuousModeConfig — 单工位持续运行（产线循环接料）
//
// 仅单工位模式（未 attach StationManager）有效。
// onEnterRunning 若 Scheduler 空闲则 submit；FlowCompleted 后按 intervalMs 重提。
// ============================================================
struct ContinuousModeConfig {
    std::string flowName;              ///< 持续执行的流程名（FlowLoader 注册名）
    json        params;                ///< 每次 submit 传入的初始参数
    int         priority{0};
    int         intervalMs{0};         ///< 两片料最小间隔（ms），0=立即
    bool        resubmitOnFail{false}; ///< 流程失败后是否继续下一片
};

// ============================================================
// MachineFSM — 整机主状态机（v0.6：四主态 + 位子标志）
//
// 【职责】
//   - 维护 MachineStatus（主态 + 子标志），对外发布 SystemReady / Qt 信号
//   - 将物理按钮、UI、AlarmManager、Scheduler 回调统一为状态转移
//   - 驱动 Scheduler（或 StationManager）启停/暂停/急停/持续接料
//   - 编排复位/急停工步流生命周期
//
// 【主态 MachineState】互斥：NotReset | Paused | Running | Alarm
// 【子标志 MachineSubFlags】可组合，见 machine_status.h
//
// 【典型操作闭环】
//   软件启动 → NotReset
//   Reset 成功 → Paused + ResettingSuccess → Start → Running
//   Running + Stop → Paused
//   Running + Alarm → Alarm（仅 Running 可进主态 Alarm）
//   Paused + Alarm → Paused + Error（须 ClearAlarm 后再 Start）
//
// 【排空】Running + ManualSwitch → Running + Draining → FlowDrained → Paused + ManualMode
//
// 【急停】任意态 + EmergencyStop → 硬急停 + 急停工步流；|= EmergencyStop；主态不变
//   EstopRelease → 仅日志；EmergencyStop 位须 Reset 成功清除
//
// 【模式】
//   单工位（默认）：直接驱动 Scheduler；enableContinuousMode 配置接料
//   多工位：attachStationManager 后启停代理给 StationManager
//
// 【线程模型】
//   - sendEvent / handleEvent：m_eventMutex_ 串行化，避免并发转移
//   - m_status_ 读写：m_statusMutex_ 保护
//   - EventBus 订阅回调可能在工作线程；回调内 sendEvent 会再次取 m_eventMutex_
//     updateStatus 使用 publishAsync 广播 SystemReady，避免同步 publish 死锁
//
// ┌─────────────────────────────────────────────────────────────┐
// │  NotReset ──Reset──> (NotReset, Resetting)                  │
// │       │复位成功──> Paused[ResettingSuccess] ──Start──> Running│
// │       │复位失败──> NotReset（清 Resetting|ResettingSuccess）  │
// │  Running ──Stop──> Paused    Running ──Alarm──> Alarm       │
// │  Paused  ──Alarm──> Paused[Error]  Alarm ──ClearAlarm──> Paused│
// │  Running + ManualSwitch ──Draining──> Paused[ManualMode]      │
// └─────────────────────────────────────────────────────────────┘
// ============================================================
class SCHEDULER_EXPORT MachineFSM {
public:
    MachineFSM(std::shared_ptr<Scheduler>    scheduler,
               std::shared_ptr<AlarmManager> alarmMgr,
               std::shared_ptr<EventBus>     eventBus);
    ~MachineFSM();

    /// 软件启动：进入 NotReset，子标志全清；不自动复位（现场须按复位键）
    void init();

    /// 统一事件入口（ButtonMonitor / UI / AlarmManager / 内部回调）
    void sendEvent(MachineEvent evt, std::any payload = {});

    MachineStatus currentStatus() const;
    MachineState  currentState() const { return currentStatus().main; }

    /** 状态变更 Qt 信号（主/子态变化）；UI 应 connect 此对象 */
    MachineStatusNotifier *statusNotifier() const { return m_statusNotifier_.get(); }

    /// 绑定多工位协调器后，启停/持续模式改由 StationManager 代理
    void attachStationManager(std::shared_ptr<StationManager> stationMgr);
    bool isMultiStationMode() const { return m_stationMgr_ != nullptr; }

    /// 单工位持续接料；多工位模式下调用无效（须走 StationManager）
    void enableContinuousMode(ContinuousModeConfig cfg);
    void disableContinuousMode();
    bool isContinuousMode() const { return m_continuousMode_.load(); }

    /// 配置复位工步流；须在 FlowLoader 注册完成后调用（SchedulerRuntime::loadFlowDefinitions）
    void setResetFlowConfig(ResetFlowConfig cfg);
    const std::string& resetFlowName() const;

    /// 配置急停工步流；须在 FlowLoader 注册完成后调用
    void setEstopFlowConfig(EstopFlowConfig cfg);
    const std::string& estopFlowName() const;

    /// 主态或子标志任一变化时回调（UI 订阅一次即可；与 statusNotifier 并行）
    void onStatusChanged(std::function<void(MachineStatus, MachineStatus)> cb) {
        m_statusChangedCb_ = std::move(cb);
    }

    // ── UI / 物理按钮薄封装（内部均 sendEvent，与 ButtonMonitor 路径一致）──
    void uiStartButton()     { sendEvent(MachineEvent::Start); }
    void uiStopButton()      { sendEvent(MachineEvent::Stop); }
    void uiResetButton()     { sendEvent(MachineEvent::Reset); }
    void uiClearAlarm()      { sendEvent(MachineEvent::ClearAlarm); }
    void uiEstopButton()     { sendEvent(MachineEvent::EmergencyStop); }
    void uiEstopRelease()    { sendEvent(MachineEvent::EstopRelease); }
    void uiResumeButton()    { uiStartButton(); }  ///< 语义已合并为 Start
    void uiManualAutoToggle();                      ///< 按当前 ManualMode 发 Auto/ManualSwitch

    /// 软件停机（班次结束/退出）：Running/Paused 映射为 Stop → Paused
    void softShutdown();

private:
    // ── 状态读写与校验 ──────────────────────────────────────────
    void updateStatus(const MachineStatus& newStatus);
    void handleEvent(MachineEvent evt, const std::any& payload);

    // ── 复位工步流（Reset 按钮 → beginReset → performReset → EventBus 回调）──
    void beginReset();              ///< canReset 守卫 + 置 Resetting + performReset
    void performReset();            ///< stopAll → submit resetFlow；失败则 completeResetFail
    void completeResetSuccess();    ///< Paused + ResettingSuccess；清 EmergencyStop/报警
    void completeResetFail();       ///< NotReset；清 Resetting|ResettingSuccess
    void completeResetAborted();    ///< NotReset；Stop/急停中止复位

    void abortResetFlow();          ///< abort(instanceId) 或 stopAll
    void clearResetTracking();      ///< 清 m_resetInstanceId_

    bool isActiveResetFlow(const SchedulerEvent& evt) const;
    void onResetFlowStarted(const SchedulerEvent& evt);
    void onResetFlowCompleted(const SchedulerEvent& evt);
    void onResetFlowFailed(const SchedulerEvent& evt);
    void onResetFlowAborted(const SchedulerEvent& evt);
    void onResetFlowStepEvent(const SchedulerEvent& evt);

    // ── 急停工步流（EmergencyStop → performEstopFlow → EventBus 回调）──────
    void performEstopFlow();
    void clearEstopTracking();

    bool isActiveEstopFlow(const SchedulerEvent& evt) const;
    bool matchesEstopInstance(const SchedulerEvent& evt) const; ///< 防重复触发 instance 竞态
    void onEstopFlowStarted(const SchedulerEvent& evt);
    void onEstopFlowCompleted(const SchedulerEvent& evt);
    void onEstopFlowFailed(const SchedulerEvent& evt);
    void onEstopFlowAborted(const SchedulerEvent& evt);
    void onEstopFlowStepEvent(const SchedulerEvent& evt);

    // ── 启停与排空 ──────────────────────────────────────────────
    bool tryStart();                ///< canStart 守卫 + onEnterRunning + main→Running
    void enterDraining(bool forManualSwitch); ///< Running→Draining；排空后进 Paused（可 ManualMode）

    void onEnterRunning();          ///< resumeAll / startAll；持续模式 idle 时 submit
    void onEnterPaused();           ///< pauseAll / setAcceptingRequests(false)
    void onEnterAlarm();            ///< stopAll；清持续模式与排空回调
    void onEmergencyStopAction();   ///< emergencyStop + EventBus EmergencyStop 广播

    // ── 持续接料（单工位）──────────────────────────────────────
    void onFlowCompleted(const SchedulerEvent& evt);
    void submitContinuousFlow();
    void clearIdleCallback();

    MachineStatus loadStatus() const;
    /// 强制 R1～R4 子标志与主态一致（见 machine_status.h 注释）
    void validateSubFlags(MachineStatus& st) const;

    // ── 成员：状态与并发 ────────────────────────────────────────
    mutable std::mutex m_statusMutex_;
    MachineStatus        m_status_{MachineState::NotReset, MachineSubFlags::None};
    std::mutex           m_eventMutex_;   ///< 串行化 handleEvent，避免并发转移

    std::shared_ptr<Scheduler>      m_scheduler_;
    std::shared_ptr<AlarmManager>   m_alarmMgr_;
    std::shared_ptr<EventBus>       m_eventBus_;
    std::shared_ptr<StationManager> m_stationMgr_;  ///< 非空 = 多工位模式

    std::function<void(MachineStatus, MachineStatus)> m_statusChangedCb_;

    std::unique_ptr<MachineStatusNotifier> m_statusNotifier_;

    // ── 持续模式 ──────────────────────────────────────────────────
    std::atomic<bool>    m_continuousMode_{false};
    ContinuousModeConfig m_continuousCfg_;
    mutable std::mutex   m_continuousMutex_;

    /// Running→ManualSwitch 排空完成后置 ManualMode（FlowDrained 时 exchange 消费）
    std::atomic<bool> m_pendingManualSwitch_{false};

    // ── EventBus 订阅（构造时注册，析构 unsubscribe）──────────────
    EventBus::SubscriberId m_flowCompletedSubId_{-1};
    EventBus::SubscriberId m_flowFailedSubId_{-1};

    // ── 复位流跟踪 ────────────────────────────────────────────────
    ResetFlowConfig      m_resetCfg_;
    mutable std::mutex   m_resetMutex_;
    std::string          m_resetInstanceId_;   ///< FlowStarted 时写入，abort 用

    EventBus::SubscriberId m_resetFlowStartedSubId_{-1};
    EventBus::SubscriberId m_resetFlowCompletedSubId_{-1};
    EventBus::SubscriberId m_resetFlowFailedSubId_{-1};
    EventBus::SubscriberId m_resetFlowAbortedSubId_{-1};
    EventBus::SubscriberId m_resetStepEventSubId_{-1};

    // ── 急停流跟踪 ────────────────────────────────────────────────
    EstopFlowConfig      m_estopCfg_;
    mutable std::mutex   m_estopMutex_;
    std::string          m_estopInstanceId_;
    std::atomic<bool>    m_estopFlowActive_{false}; ///< performEstopFlow 至终态回调

    EventBus::SubscriberId m_estopFlowStartedSubId_{-1};
    EventBus::SubscriberId m_estopFlowCompletedSubId_{-1};
    EventBus::SubscriberId m_estopFlowFailedSubId_{-1};
    EventBus::SubscriberId m_estopFlowAbortedSubId_{-1};
    EventBus::SubscriberId m_estopStepEventSubId_{-1};
};

} // namespace scheduler
