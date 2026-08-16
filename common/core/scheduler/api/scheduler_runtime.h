#pragma once

#include <scheduler/scheduler_global.h>
#include <scheduler/machine/button_monitor.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace scheduler {
class AlarmManager;
class EventBus;
struct FlowDefinition;
class MachineFSM;
class Scheduler;
} // namespace scheduler

/**
 * @brief Bootstrap 持有的调度栈（EventBus / Scheduler / MachineFSM / 流程注册）
 *
 * Container 单例 + SchedulerServiceImpl 共享同一实例。
 */
class SCHEDULER_EXPORT SchedulerRuntime
{
public:
    /// 物理面板按钮 IO 配置（与 device_config.json DeviceIo 名称一致）
    struct ButtonMonitorOptions {
        bool        enabled{false};
        bool        physicalButtonsEnabled{true}; ///< false=虚拟运控等场景，不监听 DI
        int         pollIntervalMs{20};
        std::string startBtn;
        std::string stopBtn;
        std::string resetBtn;
        std::string modeSwitch;
        std::string estop;
    };

    struct Options {
        std::string flowsDir;
        std::string flowsName;
        std::string resetFlowName;         // 整机复位工步流名，如 machine_reset
        std::string estopFlowName;         // 整机急停工步流名，如 machine_estop
        int         maxConcurrent          = 1;
        int         continuousIntervalMs   = 0;
        bool        continuousResubmitOnFail = false;
        ButtonMonitorOptions buttonMonitor;
    };

    SchedulerRuntime()  = default;
    ~SchedulerRuntime();

    SchedulerRuntime(const SchedulerRuntime &)            = delete;
    SchedulerRuntime &operator=(const SchedulerRuntime &) = delete;

    bool init(const Options &options);
    /// 从 options.flowsDir 加载并注册流程；须在业务插件 DLL 加载（工步静态注册）之后调用
    bool loadFlowDefinitions();
    bool areFlowsLoaded() const { return m_flowsLoaded_; }
    void shutdown();

    /// 启动物理按钮监控（须在 DeviceManager Init 之后；reader 由 Bootstrap 注入）
    bool startButtonMonitor(scheduler::ButtonMonitor::IoReader reader);
    void stopButtonMonitor();

    bool isReady() const { return m_ready; }

    std::shared_ptr<scheduler::Scheduler> scheduler() const { return m_scheduler_; }
    std::shared_ptr<scheduler::MachineFSM> machineFsm() const { return m_machineFsm_; }
    std::shared_ptr<scheduler::EventBus> eventBus() const { return m_eventBus_; }
    std::shared_ptr<scheduler::AlarmManager> alarmManager() const { return m_alarmMgr_; }
    std::shared_ptr<scheduler::ButtonMonitor> buttonMonitor() const { return m_buttonMonitor_; }

    const std::string &flowsName() const { return m_flowsName_; }

    std::shared_ptr<scheduler::FlowDefinition> flowDefinition(const std::string &name) const;

private:
    bool m_ready = false;

    std::shared_ptr<scheduler::EventBus>     m_eventBus_;
    std::shared_ptr<scheduler::Scheduler>    m_scheduler_;
    std::shared_ptr<scheduler::AlarmManager> m_alarmMgr_;
    std::shared_ptr<scheduler::MachineFSM>   m_machineFsm_;
    std::shared_ptr<scheduler::ButtonMonitor> m_buttonMonitor_;

    std::string m_flowsName_;
    Options     m_options_;
    bool        m_flowsLoaded_ = false;
    std::unordered_map<std::string, std::shared_ptr<scheduler::FlowDefinition>> m_flowDefs_;
};
