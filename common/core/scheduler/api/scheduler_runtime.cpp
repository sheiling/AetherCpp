#include "scheduler_runtime.h"

#include <scheduler/alarm/alarm_manager.h>
#include <scheduler/engine/flow_loader.h>
#include <scheduler/engine/scheduler.h>
#include <scheduler/machine/button_monitor.h>
#include <scheduler/machine/machine_fsm.h>
#include <scheduler/step/step_factory.h>

#include <Logger.h>

#include <filesystem>
#include <utility>

SchedulerRuntime::~SchedulerRuntime()
{
    shutdown();
}

bool SchedulerRuntime::init(const Options &options)
{
    if (m_ready)
        return true;

    m_options_   = options;
    m_flowsName_ = options.flowsName;

    scheduler::registerBuiltinSteps();

    m_eventBus_  = std::make_shared<scheduler::EventBus>();
    m_scheduler_ = std::make_shared<scheduler::Scheduler>(m_eventBus_);
    m_alarmMgr_  = std::make_shared<scheduler::AlarmManager>(m_eventBus_);
    m_machineFsm_ =
        std::make_shared<scheduler::MachineFSM>(m_scheduler_, m_alarmMgr_, m_eventBus_);

    m_scheduler_->setMaxConcurrent(options.maxConcurrent > 0 ? options.maxConcurrent : 1);

    // 流程 JSON 在 loadFlowDefinitions() 中加载（插件 DLL 工步注册完成之后）
    m_machineFsm_->init();
    m_ready = true;
    LOG_INFO("[SchedulerRuntime] engine initialized (flows pending, flowsDir={}, flowsName={})",
             m_options_.flowsDir, m_flowsName_);
    return true;
}

bool SchedulerRuntime::loadFlowDefinitions()
{
    if (!m_ready) {
        LOG_WARN("[SchedulerRuntime] loadFlowDefinitions: engine not initialized");
        return false;
    }
    if (m_flowsLoaded_)
        return true;

    m_flowDefs_.clear();

    const std::string &flowsDir = m_options_.flowsDir;
    if (!flowsDir.empty()) {
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists(flowsDir, ec) && fs::is_directory(flowsDir, ec)) {
            scheduler::FlowLoader loader;
            const auto defs = loader.loadDirectory(flowsDir);
            for (const auto &def : defs) {
                if (!def)
                    continue;
                m_flowDefs_[def->name] = def;
                m_scheduler_->registerFlow(def);
                LOG_INFO("[SchedulerRuntime] registered flow '{}'", def->name);
            }
        } else {
            LOG_WARN("[SchedulerRuntime] flows directory missing: {}", flowsDir);
        }
    }

    if (m_flowsName_.empty()) {
        LOG_WARN("[SchedulerRuntime] flowsName not configured, continuous mode disabled");
    } else if (!m_flowDefs_.count(m_flowsName_)) {
        LOG_WARN("[SchedulerRuntime] flow '{}' not loaded from {}",
                 m_flowsName_, flowsDir);
    } else if (m_machineFsm_) {
        scheduler::ContinuousModeConfig continuous;
        continuous.flowName       = m_flowsName_;
        continuous.intervalMs     = m_options_.continuousIntervalMs;
        continuous.resubmitOnFail = m_options_.continuousResubmitOnFail;
        m_machineFsm_->enableContinuousMode(std::move(continuous));
    }

    if (m_options_.resetFlowName.empty()) {
        LOG_WARN("[SchedulerRuntime] resetFlowName not configured, machine reset disabled");
    } else if (!m_flowDefs_.count(m_options_.resetFlowName)) {
        LOG_WARN("[SchedulerRuntime] reset flow '{}' not loaded from {}",
                 m_options_.resetFlowName, flowsDir);
    } else if (m_machineFsm_) {
        scheduler::ResetFlowConfig resetCfg;
        resetCfg.flowName = m_options_.resetFlowName;
        resetCfg.priority = 9;
        m_machineFsm_->setResetFlowConfig(std::move(resetCfg));
    }

    if (m_options_.estopFlowName.empty()) {
        LOG_WARN("[SchedulerRuntime] estopFlowName not configured, estop flow disabled");
    } else if (!m_flowDefs_.count(m_options_.estopFlowName)) {
        LOG_WARN("[SchedulerRuntime] estop flow '{}' not loaded from {}",
                 m_options_.estopFlowName, flowsDir);
    } else if (m_machineFsm_) {
        scheduler::EstopFlowConfig estopCfg;
        estopCfg.flowName = m_options_.estopFlowName;
        estopCfg.priority = 10;
        m_machineFsm_->setEstopFlowConfig(std::move(estopCfg));
    }

    m_flowsLoaded_ = true;
    LOG_INFO("[SchedulerRuntime] flow definitions loaded, count={}", m_flowDefs_.size());
    return true;
}

bool SchedulerRuntime::startButtonMonitor(scheduler::ButtonMonitor::IoReader reader)
{
    if (!m_ready || !m_machineFsm_) {
        LOG_WARN("[SchedulerRuntime] startButtonMonitor: engine not ready");
        return false;
    }

    const auto &bmOpt = m_options_.buttonMonitor;
    if (!bmOpt.enabled) {
        LOG_INFO("[SchedulerRuntime] ButtonMonitor disabled by config");
        return true;
    }

    if (!bmOpt.physicalButtonsEnabled) {
        LOG_INFO("[SchedulerRuntime] 虚拟运控已启用，不监听物理按钮（ButtonMonitor 未启动）");
        return true;
    }

    if (!reader) {
        LOG_ERROR("[SchedulerRuntime] ButtonMonitor enabled but IoReader is empty");
        return false;
    }

    stopButtonMonitor();

    scheduler::ButtonMonitor::Config cfg;
    cfg.startBtn       = bmOpt.startBtn;
    cfg.stopBtn        = bmOpt.stopBtn;
    cfg.resetBtn       = bmOpt.resetBtn;
    cfg.modeSwitch     = bmOpt.modeSwitch;
    cfg.estop          = bmOpt.estop;
    cfg.pollIntervalMs = bmOpt.pollIntervalMs > 0 ? bmOpt.pollIntervalMs : 20;
    cfg.physicalButtonsEnabled = bmOpt.physicalButtonsEnabled;

    m_buttonMonitor_ =
        std::make_shared<scheduler::ButtonMonitor>(m_machineFsm_, std::move(reader), cfg);
    m_buttonMonitor_->start();
    LOG_INFO("[SchedulerRuntime] ButtonMonitor started");
    return true;
}

void SchedulerRuntime::stopButtonMonitor()
{
    if (!m_buttonMonitor_)
        return;
    m_buttonMonitor_->stop();
    m_buttonMonitor_.reset();
}

void SchedulerRuntime::shutdown()
{
    if (!m_ready)
        return;

    stopButtonMonitor();

    if (m_machineFsm_)
        m_machineFsm_->disableContinuousMode();

    m_machineFsm_.reset();
    m_scheduler_.reset();
    m_alarmMgr_.reset();
    m_eventBus_.reset();
    m_flowDefs_.clear();
    m_flowsLoaded_ = false;
    m_ready        = false;
}

std::shared_ptr<scheduler::FlowDefinition> SchedulerRuntime::flowDefinition(
    const std::string &name) const
{
    const auto it = m_flowDefs_.find(name);
    return it != m_flowDefs_.end() ? it->second : nullptr;
}
