#include "flow_host_coordinator.h"

#include "flow_lifecycle_handler.h"

#include <scheduler/api/ischeduler_service.h>
#include <scheduler/engine/flow_definition.h>
#include <scheduler/engine/scheduler.h>
#include <scheduler/event/event_bus.h>
#include <scheduler/machine/machine_fsm.h>
#include <scheduler/machine/machine_status.h>

#include <Logger.h>

namespace aoi::flow_host {
namespace {

FlowHostCoordinator g_coordinator;

} // namespace

FlowHostCoordinator &flowHostCoordinator()
{
    return g_coordinator;
}

bool FlowHostCoordinator::attachToSchedulerService(ISchedulerService *schedulerService)
{
    if (!schedulerService || !schedulerService->isReady()) {
        LOG_WARN("[FlowHostCoordinator] scheduler service not ready");
        return false;
    }

    m_scheduler  = schedulerService->scheduler();
    m_machineFsm = schedulerService->machineFsm();
    m_eventBus   = schedulerService->eventBus();

    if (!m_scheduler) {
        LOG_WARN("[FlowHostCoordinator] scheduler unavailable");
        return false;
    }

    schedulerService->addFlowsLoadedListener(
        [this](ISchedulerService *svc) { attachFlowHooks(svc); });

    LOG_INFO("[FlowHostCoordinator] scheduler bound, awaiting flow definitions");
    return true;
}

bool FlowHostCoordinator::attachFlowHooks(ISchedulerService *schedulerService)
{
    if (m_flowHooksAttached)
        return true;

    if (!schedulerService) {
        LOG_WARN("[FlowHostCoordinator] scheduler service missing for flow hooks");
        return false;
    }

    m_flowDef = schedulerService->flowDefinition(schedulerService->flowsName());

    if (!m_flowDef) {
        LOG_WARN("[FlowHostCoordinator] flow '{}' unavailable",
                 schedulerService->flowsName().toStdString());
        return false;
    }

    m_flowDef->onStart = [this](scheduler::FlowContext &ctx) {
        if (m_lifecycleHandler)
            m_lifecycleHandler->onFlowStart(ctx);
        m_flowRunning = true;
        if (m_activeBridge && m_lifecycleHandler) {
            const QString msg = m_lifecycleHandler->startedLogMessage();
            if (!msg.isEmpty())
                m_activeBridge->reportLog(msg);
        }
    };

    m_flowDef->onComplete = [this](scheduler::FlowContext &ctx) {
        QString msg;
        if (m_lifecycleHandler)
            msg = m_lifecycleHandler->completeMessage(ctx);
        handleFlowFinished(true, msg);
    };

    m_flowDef->onFailed = [this](scheduler::FlowContext &ctx) {
        QString msg;
        if (m_lifecycleHandler)
            msg = m_lifecycleHandler->failedMessage(ctx);
        handleFlowFinished(false, msg);
    };

    m_flowDef->onAbort = [this](scheduler::FlowContext &ctx) {
        if (m_lifecycleHandler)
            m_lifecycleHandler->onFlowAbort(ctx);
        QString msg;
        if (m_lifecycleHandler)
            msg = m_lifecycleHandler->abortMessage();
        handleFlowFinished(false, msg);
    };

    if (m_eventBus) {
        if (m_flowStartedSubId_ >= 0)
            m_eventBus->unsubscribe(m_flowStartedSubId_);
        if (m_flowCompletedSubId_ >= 0)
            m_eventBus->unsubscribe(m_flowCompletedSubId_);
        if (m_flowFailedSubId_ >= 0)
            m_eventBus->unsubscribe(m_flowFailedSubId_);

        m_flowStartedSubId_ = m_eventBus->subscribe(
            scheduler::SchedulerEventType::FlowStarted,
            [this](const scheduler::SchedulerEvent &) { m_flowRunning = true; });

        m_flowCompletedSubId_ = m_eventBus->subscribe(
            scheduler::SchedulerEventType::FlowCompleted,
            [this](const scheduler::SchedulerEvent &evt) {
                Q_UNUSED(evt);
                m_flowRunning = false;
            });

        m_flowFailedSubId_ = m_eventBus->subscribe(
            scheduler::SchedulerEventType::FlowFailed,
            [this](const scheduler::SchedulerEvent &evt) {
                Q_UNUSED(evt);
                m_flowRunning = false;
            });
    }

    m_flowHooksAttached = true;
    LOG_INFO("[FlowHostCoordinator] attached to flow '{}'", m_flowDef->name);
    return true;
}

void FlowHostCoordinator::setLifecycleHandler(std::shared_ptr<FlowLifecycleHandler> handler)
{
    m_lifecycleHandler = std::move(handler);
    if (m_lifecycleHandler && m_activeBridge)
        m_lifecycleHandler->setProgressBridge(m_activeBridge);
}

void FlowHostCoordinator::setActiveBridge(aoi::flow::ScanFlowProgressBridge *bridge)
{
    m_activeBridge = bridge;
    if (m_lifecycleHandler)
        m_lifecycleHandler->setProgressBridge(bridge);
}

bool FlowHostCoordinator::submitSingleShot(QString *errorOut)
{
    if (!m_scheduler || !m_flowDef) {
        if (errorOut)
            *errorOut = QStringLiteral("调度服务未就绪");
        return false;
    }
    if (m_flowRunning) {
        if (errorOut)
            *errorOut = QStringLiteral("流程正在运行");
        return false;
    }

    const std::string flowName = m_flowDef->name;
    const std::string instanceId =
        m_scheduler->submit({flowName, m_flowDef->priority, {}, ""});
    if (instanceId.empty()) {
        if (errorOut)
            *errorOut = QStringLiteral("提交流程失败");
        return false;
    }
    return true;
}

bool FlowHostCoordinator::isProductionRunActive() const
{
    if (m_flowRunning)
        return true;
    if (m_machineFsm
        && m_machineFsm->currentStatus().main == scheduler::MachineState::Running)
        return true;
    return false;
}

void FlowHostCoordinator::handleFlowFinished(bool success, const QString &message)
{
    m_flowRunning = false;
    if (m_activeBridge)
        m_activeBridge->reportFlowFinished(success, message);
}

} // namespace aoi::flow_host
