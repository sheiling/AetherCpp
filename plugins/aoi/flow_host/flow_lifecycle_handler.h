#pragma once

#include <QString>

namespace scheduler {
class FlowContext;
} // namespace scheduler

namespace aoi::flow {
class ScanFlowProgressBridge;
} // namespace aoi::flow

namespace aoi::flow_host {

/**
 * @brief 流程生命周期业务扩展点（由 service/flow 等业务模块实现，挂到 FlowHostCoordinator）
 *
 * 通用协调器在 onStart/onComplete/onFailed/onAbort 时委托此接口；
 * 检测会话、设备急停等逻辑应放在实现类中，而非 flow_host 层。
 */
class FlowLifecycleHandler
{
public:
    virtual ~FlowLifecycleHandler() = default;

    virtual void setProgressBridge(aoi::flow::ScanFlowProgressBridge *bridge)
    {
        m_progressBridge = bridge;
    }

    /** 流程启动：向 FlowContext 注入业务运行时对象 */
    virtual void onFlowStart(scheduler::FlowContext &ctx) = 0;

    virtual QString completeMessage(const scheduler::FlowContext &ctx) const = 0;
    virtual QString failedMessage(const scheduler::FlowContext &ctx) const   = 0;
    virtual QString abortMessage() const                                     = 0;

    /** 流程中止时的业务清理（如急停、置 abort 标志） */
    virtual void onFlowAbort(scheduler::FlowContext &ctx) = 0;

    /** onStart 后可选日志；默认空表示不写 */
    virtual QString startedLogMessage() const { return {}; }

protected:
    aoi::flow::ScanFlowProgressBridge *m_progressBridge = nullptr;
};

} // namespace aoi::flow_host
