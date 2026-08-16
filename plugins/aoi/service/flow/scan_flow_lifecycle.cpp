/**
 * @file scan_flow_lifecycle.cpp
 * @brief AoiScanFlowLifecycleHandler 与 bindAoiScanFlowHost 实现
 */

#include "scan_flow_lifecycle.h"

#include "scan_flow_context.h"

// inspection_session 须先于 capture_motion_gateway：后者经 system_config_model.h
// #undef DEFINE_PARAM_TYPE_META，会破坏 recipe_config_model.h 末尾元数据宏。
#include "service/inspection/session/inspection_session.h"
#include "service/capture/capture_motion_gateway.h"
#include "step/inspection_step_utils.h"

#include <flow_host/flow_host_coordinator.h>

#include <atomic>
#include <memory>

namespace aoi::flow {

void AoiScanFlowLifecycleHandler::onFlowStart(scheduler::FlowContext &ctx)
{
    auto session = std::make_shared<inspection::InspectionSession>();
    session->progressBridge = m_progressBridge;
    if (!session->progressBridge)
        session->progressBridge = flow_host::flowHostCoordinator().activeBridge();
    session->abortFlag = std::make_shared<std::atomic<bool>>(false);
    ctx.set(kCtxSession, session);
}

QString AoiScanFlowLifecycleHandler::completeMessage(const scheduler::FlowContext &ctx) const
{
    if (auto session = inspection::sessionFrom(ctx))
        return session->progressMessage;
    return {};
}

QString AoiScanFlowLifecycleHandler::failedMessage(const scheduler::FlowContext &ctx) const
{
    if (auto session = inspection::sessionFrom(ctx)) {
        if (!session->progressMessage.isEmpty())
            return session->progressMessage;
    }
    return QStringLiteral("扫描流程失败");
}

QString AoiScanFlowLifecycleHandler::abortMessage() const
{
    return QStringLiteral("扫描已中止");
}

void AoiScanFlowLifecycleHandler::onFlowAbort(scheduler::FlowContext &ctx)
{
    if (auto session = inspection::sessionFrom(ctx)) {
        if (session->abortFlag)
            session->abortFlag->store(true);
        capture::CaptureMotionGateway::emergencyStop(session->sysConfig.common);
    }
}

QString AoiScanFlowLifecycleHandler::startedLogMessage() const
{
    return QStringLiteral("扫描工步流已启动");
}

void bindAoiScanFlowHost(flow_host::FlowHostCoordinator &coordinator)
{
    coordinator.setLifecycleHandler(std::make_shared<AoiScanFlowLifecycleHandler>());
}

} // namespace aoi::flow
