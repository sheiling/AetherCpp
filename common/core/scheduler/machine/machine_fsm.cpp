// ============================================================
// machine_fsm.cpp — 整机主状态机实现（v0.6）
//
// 主态 + 子标志转移见 machine_fsm.h；守卫函数见 machine_status.cpp（canStart 等）。
//
// 【线程】
//   handleEvent 由 m_eventMutex_ 串行化；状态读写由 m_statusMutex_ 保护。
//   EventBus 订阅回调在工作线程触发时，通过 sendEvent 再次进入 handleEvent。
//   updateStatus 内 publishAsync(SystemReady)，避免在持锁路径上同步 publish。
//
// 【模块划分】
//   构造/析构     — AlarmManager 与 EventBus 订阅注册
//   状态管理      — updateStatus / validateSubFlags
//   持续接料      — onFlowCompleted / submitContinuousFlow（单工位）
//   复位工步流    — beginReset → performReset → onResetFlow* 回调
//   急停工步流    — performEstopFlow → onEstopFlow* 回调
//   状态入口动作  — onEnterRunning/Paused/Alarm、onEmergencyStopAction
//   事件分派      — handleEvent（全局优先 + 主态 switch）
// ============================================================

#include "machine_fsm.h"
#include "machine_status_notifier.h"
#include "../station/station_manager.h"
#include <common/include/macros.h>
#include <Logger.h>
#include <thread>
#include <chrono>

namespace scheduler {

// ── 构造 / 析构：外部回调 → 状态机事件 ───────────────────────────

MachineFSM::MachineFSM(std::shared_ptr<Scheduler>    scheduler,
                       std::shared_ptr<AlarmManager> alarmMgr,
                       std::shared_ptr<EventBus>     eventBus)
    : m_scheduler_(std::move(scheduler))
    , m_alarmMgr_(std::move(alarmMgr))
    , m_eventBus_(std::move(eventBus))
{
    // AlarmManager 报警处置 → 状态机事件（L3 停机 / L4 急停）
    // PauseFlow 暂不映射；StopAllFlows→Alarm，EmergencyStopAll→EmergencyStop
    if (m_alarmMgr_) {
        m_alarmMgr_->onActionRequired([this](AlarmAction action, const AlarmInfo& info) {
            switch (action) {
            case AlarmAction::StopAllFlows:
                sendEvent(MachineEvent::Alarm);
                break;
            case AlarmAction::EmergencyStopAll:
                sendEvent(MachineEvent::EmergencyStop);
                break;
            case AlarmAction::PauseFlow:
                break;
            default:
                break;
            }
            (void)info;
        });
    }

    // EventBus：持续接料 + 复位/急停工步流生命周期
    // 注意：回调内最终 sendEvent 会取 m_eventMutex_；勿在 handleEvent 内同步 publish
    if (m_eventBus_) {
        // 单工位：产线 flow 完成 → 按 intervalMs 重提（排除复位/急停流）
        m_flowCompletedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowCompleted,
            [this](const SchedulerEvent& evt) { onFlowCompleted(evt); });
        m_flowFailedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowFailed,
            [this](const SchedulerEvent& evt) {
                if (m_stationMgr_)
                    return;
                if (m_continuousMode_.load()) {
                    std::lock_guard<std::mutex> lk(m_continuousMutex_);
                    // resubmitOnFail=true 时与成功路径相同，继续接料
                    if (m_continuousCfg_.resubmitOnFail)
                        onFlowCompleted(evt);
                    else
                        LOG_WARN("[状态机] 流程 {} 失败，持续模式已暂停", evt.flowName);
                }
            });

        // 复位工步流：Started 记录 instanceId；Completed/Failed/Aborted → 内部 MachineEvent
        m_resetFlowStartedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowStarted,
            [this](const SchedulerEvent& evt) { onResetFlowStarted(evt); });
        m_resetFlowCompletedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowCompleted,
            [this](const SchedulerEvent& evt) { onResetFlowCompleted(evt); });
        m_resetFlowFailedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowFailed,
            [this](const SchedulerEvent& evt) { onResetFlowFailed(evt); });
        m_resetFlowAbortedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowAborted,
            [this](const SchedulerEvent& evt) { onResetFlowAborted(evt); });
        m_resetStepEventSubId_ = m_eventBus_->subscribeAll(
            [this](const SchedulerEvent& evt) { onResetFlowStepEvent(evt); });

        // 急停工步流：matchesEstopInstance 过滤重复触发时的旧 instance 事件
        m_estopFlowStartedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowStarted,
            [this](const SchedulerEvent& evt) { onEstopFlowStarted(evt); });
        m_estopFlowCompletedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowCompleted,
            [this](const SchedulerEvent& evt) { onEstopFlowCompleted(evt); });
        m_estopFlowFailedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowFailed,
            [this](const SchedulerEvent& evt) { onEstopFlowFailed(evt); });
        m_estopFlowAbortedSubId_ = m_eventBus_->subscribe(
            SchedulerEventType::FlowAborted,
            [this](const SchedulerEvent& evt) { onEstopFlowAborted(evt); });
        m_estopStepEventSubId_ = m_eventBus_->subscribeAll(
            [this](const SchedulerEvent& evt) { onEstopFlowStepEvent(evt); });
    }
}

MachineFSM::~MachineFSM()
{
    if (m_eventBus_) {
        if (m_flowCompletedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_flowCompletedSubId_);
        if (m_flowFailedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_flowFailedSubId_);
        if (m_resetFlowStartedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_resetFlowStartedSubId_);
        if (m_resetFlowCompletedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_resetFlowCompletedSubId_);
        if (m_resetFlowFailedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_resetFlowFailedSubId_);
        if (m_resetFlowAbortedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_resetFlowAbortedSubId_);
        if (m_resetStepEventSubId_ >= 0)
            m_eventBus_->unsubscribe(m_resetStepEventSubId_);
        if (m_estopFlowStartedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_estopFlowStartedSubId_);
        if (m_estopFlowCompletedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_estopFlowCompletedSubId_);
        if (m_estopFlowFailedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_estopFlowFailedSubId_);
        if (m_estopFlowAbortedSubId_ >= 0)
            m_eventBus_->unsubscribe(m_estopFlowAbortedSubId_);
        if (m_estopStepEventSubId_ >= 0)
            m_eventBus_->unsubscribe(m_estopStepEventSubId_);
    }
}

// ── 初始化与状态读写 ────────────────────────────────────────────

void MachineFSM::init()
{
    std::lock_guard<std::mutex> lock(m_eventMutex_);
    if (!m_statusNotifier_) {
        m_statusNotifier_ = std::make_unique<MachineStatusNotifier>();
        m_statusNotifier_->bind(this);
    }
    // 软件启动默认 NotReset；须现场按复位后再 Start（见 canStart）
    m_status_ = {MachineState::NotReset, MachineSubFlags::None};
    LOG_INFO("[状态机] 软件已启动，处于未复位状态，请按复位后再启动");
    if (m_statusNotifier_)
        m_statusNotifier_->postStatusChange(m_status_, m_status_);
}

MachineStatus MachineFSM::currentStatus() const
{
    std::lock_guard<std::mutex> lock(m_statusMutex_);
    return m_status_;
}

MachineStatus MachineFSM::loadStatus() const
{
    return currentStatus();
}

void MachineFSM::validateSubFlags(MachineStatus& st) const
{
    // R1：复位中与已成功标记互斥（优先保留 Resetting）
    if (hasFlag(st.sub, MachineSubFlags::Resetting)
        && hasFlag(st.sub, MachineSubFlags::ResettingSuccess)) {
        st.sub = clearFlag(st.sub, MachineSubFlags::ResettingSuccess);
    }
    // R2：非 Running 不得保留 Draining（防止 UI 显示「运行中排空」假象）
    if (st.main != MachineState::Running)
        st.sub = clearFlag(st.sub, MachineSubFlags::Draining);
    // R3：Error 仅允许挂在 Paused
    if (st.main != MachineState::Paused)
        st.sub = clearFlag(st.sub, MachineSubFlags::Error);
    // R4：Alarm 主态与 Error 子标志互斥
    if (st.main == MachineState::Alarm)
        st.sub = clearFlag(st.sub, MachineSubFlags::Error);
}

void MachineFSM::updateStatus(const MachineStatus& newStatus)
{
    MachineStatus normalized = newStatus;
    validateSubFlags(normalized);

    MachineStatus old;
    {
        std::lock_guard<std::mutex> lock(m_statusMutex_);
        old = m_status_;
        if (old == normalized)
            return;
        m_status_ = normalized;
    }

    LOG_INFO("[状态机] 状态迁移: {} → {}",
             statusToString(old), statusToString(normalized));

    if (m_statusChangedCb_)
        m_statusChangedCb_(old, normalized);

    if (m_statusNotifier_)
        m_statusNotifier_->postStatusChange(old, normalized);

    if (m_eventBus_) {
        // SystemReady：供非 Qt 模块订阅整机状态；payload 为 MachineStatus
        SchedulerEvent evt;
        evt.type    = SchedulerEventType::SystemReady;
        evt.payload = normalized;
        m_eventBus_->publishAsync(std::move(evt));
    }
}

// ── 公开薄封装 ──────────────────────────────────────────────────

void MachineFSM::sendEvent(MachineEvent evt, std::any payload)
{
    handleEvent(evt, payload);
}

void MachineFSM::uiManualAutoToggle()
{
    const auto st = currentStatus();
    if (hasFlag(st.sub, MachineSubFlags::ManualMode))
        sendEvent(MachineEvent::AutoSwitch);
    else
        sendEvent(MachineEvent::ManualSwitch);
}

void MachineFSM::softShutdown()
{
    const auto st = currentStatus();
    if (st.main == MachineState::Running || st.main == MachineState::Paused) {
        LOG_INFO("[状态机] 软件停机：映射为暂停");
        sendEvent(MachineEvent::Stop);
    } else {
        LOG_WARN("[状态机] softShutdown 在当前状态 [{}] 下无效",
                 statusToString(st).c_str());
    }
}

// ── 模式配置：多工位 / 持续接料 / 复位·急停流 ───────────────────

void MachineFSM::attachStationManager(std::shared_ptr<StationManager> stationMgr)
{
    m_stationMgr_ = std::move(stationMgr);
    LOG_INFO("[状态机] 已绑定 StationManager，切换为多工位模式");
}

void MachineFSM::enableContinuousMode(ContinuousModeConfig cfg)
{
    if (m_stationMgr_) {
        LOG_WARN("[状态机] 多工位模式下请通过 StationManager 配置各工位持续运行");
        return;
    }
    std::lock_guard<std::mutex> lk(m_continuousMutex_);
    m_continuousCfg_ = std::move(cfg);
    m_continuousMode_.store(true);
    LOG_INFO("[状态机] 持续运行模式已启用，流程: {}", m_continuousCfg_.flowName);
}

void MachineFSM::disableContinuousMode()
{
    m_continuousMode_.store(false);
    LOG_INFO("[状态机] 持续运行模式已关闭");
}

void MachineFSM::setResetFlowConfig(ResetFlowConfig cfg)
{
    std::lock_guard<std::mutex> lk(m_resetMutex_);
    m_resetCfg_ = std::move(cfg);
    LOG_INFO("[状态机] 复位工步流已配置: {}", m_resetCfg_.flowName);
}

const std::string& MachineFSM::resetFlowName() const
{
    std::lock_guard<std::mutex> lk(m_resetMutex_);
    return m_resetCfg_.flowName;
}

void MachineFSM::setEstopFlowConfig(EstopFlowConfig cfg)
{
    std::lock_guard<std::mutex> lk(m_estopMutex_);
    m_estopCfg_ = std::move(cfg);
    LOG_INFO("[状态机] 急停工步流已配置: {}", m_estopCfg_.flowName);
}

const std::string& MachineFSM::estopFlowName() const
{
    std::lock_guard<std::mutex> lk(m_estopMutex_);
    return m_estopCfg_.flowName;
}

// ── 持续接料（单工位，flowName 须与 continuousCfg 一致）──────────

void MachineFSM::onFlowCompleted(const SchedulerEvent& evt)
{
    if (m_stationMgr_)
        return;
    // 复位/急停系统流不参与接料逻辑
    if (isActiveResetFlow(evt))
        return;
    if (isActiveEstopFlow(evt))
        return;
    if (!m_continuousMode_.load())
        return;

    const auto st = currentStatus();
    // 仅 Running 且非 Draining 时接料；Stop/Alarm 后不再 submit
    if (st.main != MachineState::Running || hasFlag(st.sub, MachineSubFlags::Draining))
        return;

    {
        std::lock_guard<std::mutex> lk(m_continuousMutex_);
        if (evt.flowName != m_continuousCfg_.flowName)
            return;
    }

    int interval = 0;
    {
        std::lock_guard<std::mutex> lk(m_continuousMutex_);
        interval = m_continuousCfg_.intervalMs;
    }
    // 阻塞当前 EventBus 回调线程；interval 过大时考虑改为 Scheduler 定时 submit
    if (interval > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

    submitContinuousFlow();
}

void MachineFSM::submitContinuousFlow()
{
    std::lock_guard<std::mutex> lk(m_continuousMutex_);
    if (!m_scheduler_ || !m_continuousMode_.load())
        return;
    LOG_INFO("[状态机] 持续模式：自动重新提交流程 '{}'", m_continuousCfg_.flowName);
    m_scheduler_->submit({
        m_continuousCfg_.flowName,
        m_continuousCfg_.priority,
        m_continuousCfg_.params,
        ""});
}

void MachineFSM::clearIdleCallback()
{
    if (m_scheduler_)
        m_scheduler_->setOnIdle(nullptr);
}

// ── 复位工步流 ───────────────────────────────────────────────────

void MachineFSM::beginReset()
{
    auto st = loadStatus();
    if (!canReset(st)) {
        if (isResetting(st.sub))
            LOG_WARN_REPORT("[状态机] 复位已在进行中，忽略重复请求");
        else if (st.main == MachineState::Running)
            LOG_WARN_REPORT("[状态机] 运行中不允许复位，请先停止");
        return;
    }

    // 进入复位：主态 NotReset + Resetting；清历史 ResettingSuccess
    st.main = MachineState::NotReset;
    st.sub  = clearFlag(st.sub, MachineSubFlags::ResettingSuccess);
    st.sub  = setFlag(st.sub, MachineSubFlags::Resetting);
    updateStatus(st);
    performReset();
}

void MachineFSM::performReset()
{
    LOG_INFO_REPORT("[状态机] 开始整机复位...");

    if (!m_scheduler_) {
        LOG_ERROR_REPORT("[状态机] Scheduler 未就绪，复位失败");
        completeResetFail();
        return;
    }

    std::string flowName;
    json params;
    int priority = 9;
    {
        std::lock_guard<std::mutex> lk(m_resetMutex_);
        flowName = m_resetCfg_.flowName;
        params   = m_resetCfg_.params;
        priority = m_resetCfg_.priority;
    }

    if (flowName.empty()) {
        LOG_ERROR_REPORT("[状态机] 未配置复位工步流（scheduler.resetFlowName）");
        completeResetFail();
        return;
    }

    if (!m_scheduler_->hasFlow(flowName)) {
        LOG_ERROR_REPORT("[状态机] 复位工步流 '{}' 未注册", flowName);
        completeResetFail();
        return;
    }

    clearResetTracking();
    // 中止产线/残留流程，再独占提交复位流
    m_scheduler_->stopAll();

    // stopAll 会 setAcceptingRequests(false)；系统复位流须临时放行
    m_scheduler_->setAcceptingRequests(true);
    const std::string requestId = m_scheduler_->submit({
        flowName,
        priority,
        params,
        "machine_reset"});

    if (requestId.empty()) {
        LOG_ERROR_REPORT("[状态机] 提交复位工步流 '{}' 失败", flowName);
        m_scheduler_->setAcceptingRequests(false);
        completeResetFail();
        return;
    }

    m_scheduler_->setAcceptingRequests(false);
    LOG_INFO_REPORT("[状态机] 已提交复位工步流 '{}'，requestId={}", flowName, requestId);
}

void MachineFSM::clearResetTracking()
{
    std::lock_guard<std::mutex> lk(m_resetMutex_);
    m_resetInstanceId_.clear();
}

void MachineFSM::abortResetFlow()
{
    std::string instanceId;
    {
        std::lock_guard<std::mutex> lk(m_resetMutex_);
        instanceId = m_resetInstanceId_;
    }

    if (!m_scheduler_)
        return;

    if (!instanceId.empty())
        m_scheduler_->abort(instanceId);
    else
        m_scheduler_->stopAll();
}

bool MachineFSM::isActiveResetFlow(const SchedulerEvent& evt) const
{
    if (!isResetting(loadStatus().sub))
        return false;

    std::lock_guard<std::mutex> lk(m_resetMutex_);
    return !m_resetCfg_.flowName.empty() && evt.flowName == m_resetCfg_.flowName;
}

void MachineFSM::onResetFlowStarted(const SchedulerEvent& evt)
{
    if (!isActiveResetFlow(evt))
        return;

    {
        std::lock_guard<std::mutex> lk(m_resetMutex_);
        m_resetInstanceId_ = evt.instanceId;
    }
    LOG_INFO_REPORT("[状态机] 复位工步流已启动，instanceId={}", evt.instanceId);
}

void MachineFSM::onResetFlowCompleted(const SchedulerEvent& evt)
{
    if (!isActiveResetFlow(evt))
        return;

    LOG_INFO_REPORT("[状态机] 复位工步流执行成功");
    clearResetTracking();
    sendEvent(MachineEvent::ResetSucceeded);
}

void MachineFSM::onResetFlowFailed(const SchedulerEvent& evt)
{
    if (!isActiveResetFlow(evt))
        return;

    LOG_ERROR_REPORT("[状态机] 复位工步流执行失败，instanceId={}", evt.instanceId);
    clearResetTracking();
    sendEvent(MachineEvent::ResetFailed);
}

void MachineFSM::onResetFlowAborted(const SchedulerEvent& evt)
{
    if (!isActiveResetFlow(evt))
        return;

    LOG_WARN_REPORT("[状态机] 复位工步流已中止，instanceId={}", evt.instanceId);
    sendEvent(MachineEvent::ResetAborted);
}

void MachineFSM::onResetFlowStepEvent(const SchedulerEvent& evt)
{
    if (!isActiveResetFlow(evt))
        return;

    switch (evt.type) {
    case SchedulerEventType::StepStarted:
        LOG_INFO("[状态机] 复位工步开始: {}", evt.stepName);
        break;
    case SchedulerEventType::StepCompleted:
        LOG_INFO("[状态机] 复位工步完成: {}", evt.stepName);
        break;
    case SchedulerEventType::StepFailed:
        LOG_WARN_REPORT("[状态机] 复位工步失败: {}", evt.stepName);
        break;
    case SchedulerEventType::AlarmRaised:
        LOG_WARN_REPORT("[状态机] 复位过程中报警: step={}", evt.stepName);
        break;
    default:
        break;
    }
}

// ── 急停工步流 ───────────────────────────────────────────────────

void MachineFSM::clearEstopTracking()
{
    std::lock_guard<std::mutex> lk(m_estopMutex_);
    m_estopInstanceId_.clear();
    m_estopFlowActive_.store(false);
}

bool MachineFSM::isActiveEstopFlow(const SchedulerEvent& evt) const
{
    if (!m_estopFlowActive_.load())
        return false;

    std::lock_guard<std::mutex> lk(m_estopMutex_);
    return !m_estopCfg_.flowName.empty() && evt.flowName == m_estopCfg_.flowName;
}

bool MachineFSM::matchesEstopInstance(const SchedulerEvent& evt) const
{
    if (!isActiveEstopFlow(evt))
        return false;

    std::lock_guard<std::mutex> lk(m_estopMutex_);
    if (m_estopInstanceId_.empty())
        return false;
    return evt.instanceId == m_estopInstanceId_;
}

void MachineFSM::performEstopFlow()
{
    if (!m_scheduler_) {
        LOG_ERROR_REPORT("[状态机] Scheduler 未就绪，无法执行急停工步流");
        return;
    }

    std::string flowName;
    json params;
    int priority = 10;
    {
        std::lock_guard<std::mutex> lk(m_estopMutex_);
        flowName = m_estopCfg_.flowName;
        params   = m_estopCfg_.params;
        priority = m_estopCfg_.priority;
    }

    if (flowName.empty()) {
        LOG_WARN("[状态机] 未配置急停工步流（scheduler.estopFlowName），仅执行硬急停");
        return;
    }

    if (!m_scheduler_->hasFlow(flowName)) {
        LOG_ERROR_REPORT("[状态机] 急停工步流 '{}' 未注册", flowName);
        return;
    }

    clearEstopTracking();

    m_estopFlowActive_.store(true);
    // onEmergencyStopAction 已 emergencyStop() → accepting=false；急停系统流须临时放行
    m_scheduler_->setAcceptingRequests(true);
    const std::string requestId = m_scheduler_->submit({
        flowName,
        priority,
        params,
        "machine_estop"});

    if (requestId.empty()) {
        LOG_ERROR_REPORT("[状态机] 提交急停工步流 '{}' 失败", flowName);
        m_estopFlowActive_.store(false);
        m_scheduler_->setAcceptingRequests(false);
        return;
    }

    m_scheduler_->setAcceptingRequests(false);
    LOG_INFO_REPORT("[状态机] 已提交急停工步流 '{}'，requestId={}", flowName, requestId);
}

void MachineFSM::onEstopFlowStarted(const SchedulerEvent& evt)
{
    if (!isActiveEstopFlow(evt))
        return;

    {
        std::lock_guard<std::mutex> lk(m_estopMutex_);
        m_estopInstanceId_ = evt.instanceId;
    }
    LOG_INFO_REPORT("[状态机] 急停工步流已启动，instanceId={}", evt.instanceId);
}

void MachineFSM::onEstopFlowCompleted(const SchedulerEvent& evt)
{
    if (!matchesEstopInstance(evt))
        return;

    clearEstopTracking();
    LOG_INFO_REPORT("[状态机] 急停工步流执行完成，须复位后可启动");
}

void MachineFSM::onEstopFlowFailed(const SchedulerEvent& evt)
{
    if (!matchesEstopInstance(evt))
        return;

    clearEstopTracking();
    LOG_ERROR_REPORT("[状态机] 急停工步流执行失败，instanceId={}；仍须复位后可启动",
                     evt.instanceId);
}

void MachineFSM::onEstopFlowAborted(const SchedulerEvent& evt)
{
    if (!matchesEstopInstance(evt))
        return;

    clearEstopTracking();
    LOG_WARN("[状态机] 急停工步流已中止，instanceId={}", evt.instanceId);
}

void MachineFSM::onEstopFlowStepEvent(const SchedulerEvent& evt)
{
    if (!matchesEstopInstance(evt))
        return;

    switch (evt.type) {
    case SchedulerEventType::StepStarted:
        LOG_INFO("[状态机] 急停工步开始: {}", evt.stepName);
        break;
    case SchedulerEventType::StepCompleted:
        LOG_INFO("[状态机] 急停工步完成: {}", evt.stepName);
        break;
    case SchedulerEventType::StepFailed:
        LOG_WARN_REPORT("[状态机] 急停工步失败: {}", evt.stepName);
        break;
    case SchedulerEventType::AlarmRaised:
        LOG_WARN_REPORT("[状态机] 急停过程中报警: step={}", evt.stepName);
        break;
    default:
        break;
    }
}

// ── 复位终态（由 ResetSucceeded/Failed/Aborted 内部事件触发）──────

void MachineFSM::completeResetSuccess()
{
    clearResetTracking();
    clearEstopTracking();
    auto st = loadStatus();
    st.main = MachineState::Paused;
    st.sub  = clearFlag(st.sub, MachineSubFlags::Resetting);
    st.sub  = clearFlag(st.sub, MachineSubFlags::Error);
    st.sub  = clearFlag(st.sub, MachineSubFlags::EmergencyStop);  // 急停须复位成功才清
    st.sub  = setFlag(st.sub, MachineSubFlags::ResettingSuccess);

    if (m_alarmMgr_)
        m_alarmMgr_->clearAll();
    if (m_scheduler_)
        m_scheduler_->setAcceptingRequests(true);

    updateStatus(st);
    LOG_INFO_REPORT("[状态机] 复位成功，已进入暂停态，可按启动");
}

void MachineFSM::completeResetFail()
{
    clearResetTracking();
    auto st = loadStatus();
    st.main = MachineState::NotReset;
    // R6：不设 ResettingFailed 位；失败时清 Resetting|ResettingSuccess，EmergencyStop 保持
    st.sub  = clearFlags(st.sub,
                         MachineSubFlags::Resetting | MachineSubFlags::ResettingSuccess);
    updateStatus(st);
    LOG_ERROR_REPORT("[状态机] 复位失败，请检查设备后重新复位");
}

void MachineFSM::completeResetAborted()
{
    clearResetTracking();
    auto st = loadStatus();
    st.main = MachineState::NotReset;
    st.sub  = clearFlags(st.sub,
                         MachineSubFlags::Resetting | MachineSubFlags::ResettingSuccess);
    updateStatus(st);
    LOG_WARN_REPORT("[状态机] 复位已中止，可重新按复位");
}

// ── 启停、排空、状态入口动作 ─────────────────────────────────────

bool MachineFSM::tryStart()
{
    const auto st = loadStatus();
    if (!canStart(st)) {
        if (hasFlag(st.sub, MachineSubFlags::ManualMode)) {
            LOG_WARN_REPORT("[状态机] 当前为手动模式，请先切换到自动模式后再启动");
        } else if (hasFlag(st.sub, MachineSubFlags::EmergencyStop)) {
            LOG_WARN_REPORT("[状态机] 急停有效，请先释放急停并完成复位");
        } else if (hasFlag(st.sub, MachineSubFlags::Error)) {
            LOG_WARN_REPORT("[状态机] 存在挂起报警，请先清除报警后再启动");
        } else if (st.main == MachineState::NotReset) {
            LOG_WARN_REPORT("[状态机] 设备未复位，请先按复位");
        } else if (st.main == MachineState::Alarm) {
            LOG_WARN_REPORT("[状态机] 设备处于报警停机，请先清除报警或复位");
        } else if (st.main == MachineState::Running) {
            LOG_WARN_REPORT("[状态机] 已在运行中，忽略启动");
        } else {
            LOG_WARN_REPORT("[状态机] 当前状态不允许启动: {}", statusToString(st));
        }
        return false;
    }

    onEnterRunning();
    auto next = st;
    next.main = MachineState::Running;
    // 启动后 ResettingSuccess 仅作「已复位」记忆，Running 下可清掉
    next.sub  = clearFlag(next.sub, MachineSubFlags::ResettingSuccess);
    updateStatus(next);
    return true;
}

void MachineFSM::enterDraining(bool forManualSwitch)
{
    auto st = loadStatus();
    if (st.main != MachineState::Running) {
        LOG_WARN("[状态机] enterDraining 仅在 Running 下有效");
        return;
    }
    if (hasFlag(st.sub, MachineSubFlags::Draining))
        return;

    m_pendingManualSwitch_.store(forManualSwitch);
    m_continuousMode_.store(false);
    st.sub = setFlag(st.sub, MachineSubFlags::Draining);
    updateStatus(st);

    LOG_INFO("[状态机] 进入排空：等待流程安全停止...");

    // setOnIdle：全部 flow 结束后 sendEvent(FlowDrained)
    if (m_stationMgr_) {
        m_stationMgr_->disableContinuousMode();
        if (m_scheduler_) {
            m_scheduler_->setOnIdle([this]() {
                LOG_INFO("[状态机] 所有流程已排空");
                sendEvent(MachineEvent::FlowDrained);
            });
            m_scheduler_->setAcceptingRequests(false);
        }
        m_stationMgr_->stopAll();
    } else if (m_scheduler_) {
        m_scheduler_->setOnIdle([this]() {
            LOG_INFO("[状态机] 所有流程已排空");
            sendEvent(MachineEvent::FlowDrained);
        });
        m_scheduler_->stopAll();
    }
}

void MachineFSM::onEnterRunning()
{
    LOG_INFO("[状态机] 进入 Running");

    if (m_stationMgr_) {
        m_stationMgr_->startAll();
        return;
    }

    if (!m_scheduler_)
        return;

    m_scheduler_->setAcceptingRequests(true);
    m_scheduler_->resumeAll();

    if (m_continuousMode_.load() && m_scheduler_->isIdle())
        submitContinuousFlow();
}

void MachineFSM::onEnterPaused()
{
    LOG_INFO("[状态机] 进入 Paused：暂停接受新请求");

    if (m_stationMgr_) {
        m_stationMgr_->pauseAll();
        return;
    }

    if (!m_scheduler_)
        return;

    m_scheduler_->setAcceptingRequests(false);
    m_scheduler_->pauseAll();
}

void MachineFSM::onEnterAlarm()
{
    LOG_WARN("[状态机] 报警停机：停止所有流程");
    m_continuousMode_.store(false);
    m_pendingManualSwitch_.store(false);
    clearIdleCallback();

    if (m_stationMgr_) {
        m_stationMgr_->disableContinuousMode();
        m_stationMgr_->stopAll();
    }
    if (m_scheduler_)
        m_scheduler_->stopAll();
}

void MachineFSM::onEmergencyStopAction()
{
    LOG_ERROR_REPORT("[状态机] 急停触发！立即中止所有流程");
    m_continuousMode_.store(false);
    m_pendingManualSwitch_.store(false);
    clearIdleCallback();

    if (m_stationMgr_) {
        m_stationMgr_->disableContinuousMode();
        m_stationMgr_->stopAll();
    }
    if (m_scheduler_)
        m_scheduler_->emergencyStop();
    if (m_eventBus_)
        // 广播急停，供 UI/日志/外部模块同步（与 MachineEvent::EmergencyStop 并行）
        m_eventBus_->publishAsync({SchedulerEventType::EmergencyStop, "", ""});
}

// ── handleEvent：全局优先事件 + 主态分派 ─────────────────────────
//
// 分派顺序见 machine_fsm.h「handleEvent 全局优先级」。
// 主态 switch 仅处理 Start/Alarm/ManualSwitch/AutoSwitch；
// Stop/Reset/ClearAlarm 在 switch 前统一处理（与 canStop/canReset/canClearAlarm 一致）。

void MachineFSM::handleEvent(MachineEvent evt, const std::any& /*payload*/)
{
    std::lock_guard<std::mutex> lock(m_eventMutex_);
    auto st = loadStatus();

    // ── 1. 急停：任意态最高优先级 ────────────────────────────────
    if (evt == MachineEvent::EmergencyStop) {
        onEmergencyStopAction();
        st = loadStatus();
        st.sub = setFlag(st.sub, MachineSubFlags::EmergencyStop);
        // 复位中被急停：清 Resetting 并 abort 跟踪（主态保持 NotReset 等）
        if (hasFlag(st.sub, MachineSubFlags::Resetting)) {
            st.sub = clearFlag(st.sub, MachineSubFlags::Resetting);
            clearResetTracking();
        }
        updateStatus(st);
        performEstopFlow();
        return;
    }

    // ── 2. 急停硬件释放：仅提示，不改 EmergencyStop 位 ───────────
    if (evt == MachineEvent::EstopRelease) {
        // 仅记录硬件释放；EmergencyStop 位保持，仍不可 Start，须 Reset 成功清除
        LOG_ERROR_REPORT("[状态机] 急停硬件已释放,须重新复位后启动");
        return;
    }

    // ── 3. 复位工步流异步结果 ────────────────────────────────────
    if (evt == MachineEvent::ResetSucceeded) {
        completeResetSuccess();
        return;
    }
    if (evt == MachineEvent::ResetFailed) {
        completeResetFail();
        return;
    }
    if (evt == MachineEvent::ResetAborted) {
        completeResetAborted();
        return;
    }

    // ── 4. 复位中遇报警：abort 流，终态由 FlowFailed/Aborted 回调驱动 ──
    if (evt == MachineEvent::Alarm && isResetting(st.sub)) {
        LOG_WARN_REPORT("[状态机] 复位过程中报警，终止复位工步流");
        abortResetFlow();
        return;
    }

    // ── 5. 排空完成：Running+Draining → Paused（可选 ManualMode）──
    if (evt == MachineEvent::FlowDrained) {
        if (st.main != MachineState::Running || !hasFlag(st.sub, MachineSubFlags::Draining))
            return;

        clearIdleCallback();
        st.sub = clearFlag(st.sub, MachineSubFlags::Draining);
        if (m_pendingManualSwitch_.exchange(false))
            st.sub = setFlag(st.sub, MachineSubFlags::ManualMode);
        st.main = MachineState::Paused;
        updateStatus(st);
        LOG_INFO("[状态机] 排空完成，进入暂停");
        return;
    }

    // ── 6. 统一守卫事件（canStop/canReset/canClearAlarm，与 UI 共用）──
    if (evt == MachineEvent::Stop) {
        if (isResetting(st.sub)) {
            LOG_INFO_REPORT("[状态机] 暂停/停止：中止复位工步流");
            abortResetFlow();
            return;
        }
        if (!canStop(st)) {
            LOG_INFO_REPORT("[状态机] 已处于暂停，忽略停止");
            return;
        }
        onEnterPaused();
        st = loadStatus();
        st.main = MachineState::Paused;
        st.sub  = clearFlag(st.sub, MachineSubFlags::Draining);
        clearIdleCallback();
        updateStatus(st);
        return;
    }

    if (evt == MachineEvent::Reset) {
        beginReset();
        return;
    }

    if (evt == MachineEvent::ClearAlarm) {
        if (!canClearAlarm(st))
            return;
        if (m_alarmMgr_)
            m_alarmMgr_->clearAll();
        if (st.main == MachineState::Alarm) {
            if (m_scheduler_)
                m_scheduler_->setAcceptingRequests(true);
            st.main = MachineState::Paused;
        }
        st.sub = clearFlag(st.sub, MachineSubFlags::Error);
        updateStatus(st);
        return;
    }

    switch (st.main) {
    // ── 7. 按主态分派 Start / Alarm / ManualSwitch / AutoSwitch ───
    case MachineState::NotReset:
        // Start 拒绝；手自动仅置/清 ManualMode 位（不影响主态）
        if (evt == MachineEvent::Start) {
            LOG_WARN_REPORT("[状态机] 未复位，无法启动");
        } else if (evt == MachineEvent::ManualSwitch) {
            st.sub = setFlag(st.sub, MachineSubFlags::ManualMode);
            updateStatus(st);
        } else if (evt == MachineEvent::AutoSwitch) {
            st.sub = clearFlag(st.sub, MachineSubFlags::ManualMode);
            updateStatus(st);
        }
        break;

    case MachineState::Paused:
        // Start→tryStart；Alarm→Error 子标志（主态仍为 Paused，与 Running+Alarm→Alarm 区分）
        if (evt == MachineEvent::Start) {
            tryStart();
        } else if (evt == MachineEvent::Alarm) {
            st.sub = setFlag(st.sub, MachineSubFlags::Error);
            updateStatus(st);
        } else if (evt == MachineEvent::ManualSwitch) {
            // 暂停态切手动：停流程并禁止后续 Start（canStart 检查 ManualMode）
            st.sub = setFlag(st.sub, MachineSubFlags::ManualMode);
            m_continuousMode_.store(false);
            if (m_stationMgr_)
                m_stationMgr_->stopAll();
            else if (m_scheduler_)
                m_scheduler_->stopAll();
            updateStatus(st);
        } else if (evt == MachineEvent::AutoSwitch) {
            st.sub = clearFlag(st.sub, MachineSubFlags::ManualMode);
            updateStatus(st);
        }
        break;

    case MachineState::Running:
        // Alarm→主态 Alarm；ManualSwitch→enterDraining(true) 安全排空后进 Paused+ManualMode
        if (evt == MachineEvent::Start) {
            LOG_INFO_REPORT("[状态机] 已在运行中，忽略启动");
        } else if (evt == MachineEvent::Alarm) {
            onEnterAlarm();
            st = loadStatus();
            st.main = MachineState::Alarm;
            st.sub  = clearFlag(st.sub, MachineSubFlags::Draining);
            updateStatus(st);
        } else if (evt == MachineEvent::ManualSwitch) {
            if (hasFlag(st.sub, MachineSubFlags::Draining)) {
                LOG_INFO("[状态机] 已在排空中，忽略重复手动切换");
            } else {
                enterDraining(true);
            }
        }
        break;

    case MachineState::Alarm:
        // 报警停机态：Start 拒绝；重复 Alarm 仅刷新日志
        if (evt == MachineEvent::Start) {
            LOG_WARN_REPORT("[状态机] 报警停机中，无法启动");
        } else if (evt == MachineEvent::Alarm) {
            LOG_INFO_REPORT("[状态机] 已在报警停机，刷新报警信息");
        }
        break;

    default:
        break;
    }
}

} // namespace scheduler
