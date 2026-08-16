#pragma once

#include "service/flow/scan_flow_progress_bridge.h"

#include <memory>

namespace scheduler {
class EventBus;
struct FlowDefinition;
class MachineFSM;
class Scheduler;
} // namespace scheduler

class ISchedulerService;

namespace aoi::flow_host {

class FlowLifecycleHandler;

/**
 * @brief 调度流程宿主：绑定 Scheduler/EventBus、挂接 FlowDefinition 钩子、驱动 submit
 *
 * 不含检测业务；业务通过 FlowLifecycleHandler 注入。
 */
class FlowHostCoordinator final
{
public:
    FlowHostCoordinator() = default;

    /** 插件 extensionsInitialized：绑定 Scheduler/EventBus，流程钩子待 flows 加载后挂接 */
    bool attachToSchedulerService(ISchedulerService *schedulerService);

    /** flows 加载完成后由 addFlowsLoadedListener 触发 */
    bool attachFlowHooks(ISchedulerService *schedulerService);

    void setLifecycleHandler(std::shared_ptr<FlowLifecycleHandler> handler);
    void setActiveBridge(aoi::flow::ScanFlowProgressBridge *bridge);
    aoi::flow::ScanFlowProgressBridge *activeBridge() const { return m_activeBridge; }

    bool submitSingleShot(QString *errorOut = nullptr);
    bool isFlowRunning() const { return m_flowRunning; }

    /** 产线扫描进行中或整机 Running 时为 true（禁止配方页虚拟调测） */
    bool isProductionRunActive() const;

private:
    void handleFlowFinished(bool success, const QString &message);

    std::shared_ptr<scheduler::Scheduler>      m_scheduler;
    std::shared_ptr<scheduler::MachineFSM>     m_machineFsm;
    std::shared_ptr<scheduler::EventBus>       m_eventBus;
    std::shared_ptr<scheduler::FlowDefinition> m_flowDef;
    std::shared_ptr<FlowLifecycleHandler>      m_lifecycleHandler;

    aoi::flow::ScanFlowProgressBridge *m_activeBridge = nullptr;

    bool m_flowRunning       = false;
    bool m_flowHooksAttached = false;

    int m_flowCompletedSubId_{-1};
    int m_flowFailedSubId_{-1};
    int m_flowStartedSubId_{-1};
};

/** 插件内全局流程宿主（单例） */
FlowHostCoordinator &flowHostCoordinator();

} // namespace aoi::flow_host
